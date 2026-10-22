import { SizeOptions } from '/arkui/component/units';
import { CustomBuilder, WrappedBuilder } from '/arkui/component/builder';
import { BusinessError } from "@ohos.base";
import { ArkUIAniModule } from "arkui.ani";
import { createUiDetachedRoot, destroyUiDetachedRoot } from '/arkui/ArkUIEntry';
import { UIContextUtil } from '/arkui/base/UIContextUtil';
import { PeerNode } from '/arkui/PeerNode';
import { ArkComponentRootPeer } from '/arkui/component/staticComponents';
import { InteropNativeModule, KPointer } from '@koalaui/interop';
import { int32 } from '@koalaui/common';
import { ObserveSingleton } from '/arkui/stateManagement/base/observeSingleton';
import { ExtendableComponent } from '/arkui/component/extendableComponent';
import { setNeedCreate } from '/arkui/ArkComponentRoot';
import { GlobalStateManager } from '@koalaui/runtime';

// Result codes of the native registration (ani_placeholder_register_module.cpp):
// the admission protocol (isolated execution, restricted-scope probe, clone
// check) runs natively exactly like on the declarative side.
const REGISTER_RESULT_ADMITTED: number = 0;
const REGISTER_RESULT_REJECTED: number = -1;
const REGISTER_RESULT_PARAM_INVALID: number = -2;
const REGISTER_RESULT_NOT_INSTRUMENTED: number = -3;
const REGISTER_RESULT_UNSUPPORTED_COMPONENT: number = -4;

// Diagnostics channel of the static registration chain: every step of the
// module entry points and of the isolated execution logs here, so a broken
// chain (attribute set but nothing shown, register rejected, builder context
// failure) can be located from one filtered log run. Grep "PlaceholderRegister".
function prLog(message: string): void {
    InteropNativeModule._NativeLog(`PlaceholderRegister: ${message}`);
}

/**
 * Static-side mirror of the SDK module @ohos.arkui.placeholderRegister.
 *
 * Describes the placeholder to display for an item of a scrollable component
 * while the real item is being created across frames.
 */
export interface PlaceholderOptions {
    /**
     * ID of the registered placeholder builder. If **id** is omitted, empty,
     * or not registered, the system default placeholder is used.
     */
    id?: string;

    /**
     * Suggested layout size of the placeholder.
     */
    size?: SizeOptions;
}

/**
 * Provides the placeholder description for an item of a scrollable component.
 * The provider is invoked synchronously during scrolling and must be a pure
 * calculation. Returning **undefined** means the item does not need a placeholder.
 */
export type PlaceholderProvider = (index: int) => PlaceholderOptions | undefined;

/**
 * Provides the methods for registering, unregistering, and querying scroll
 * placeholder builders. Each UI instance holds its own registry: builders
 * registered in one instance are not visible to another. Do not create
 * instances of this class; use getPlaceholderRegister() of the **UIContext**
 * to obtain the registry of the current UI instance.
 */
export class PlaceholderRegister {
    private builders_: Map<string, WrappedBuilder<CustomBuilder>> = new Map<string, WrappedBuilder<CustomBuilder>>();
    private instanceId_: int32;
    // Latest detached-root pointer per template id: only the pointer is kept
    // here for the unregister-time teardown - the tree itself stays alive in
    // the UIContext's DetachedRootEntryManager (detachedRoots_).
    private roots_: Map<string, KPointer> = new Map<string, KPointer>();

    constructor(instanceId: int32) {
        this.instanceId_ = instanceId;
    }

    /**
     * Registers a placeholder builder under the given ID for scrollable components.
     * The builder must be a global, parameterless **@Builder({ isPlaceholder: true })**
     * function wrapped through **wrapBuilder**.
     */
    public register(id: string, builder: WrappedBuilder<CustomBuilder>): void {
        prLog(`register: enter, id='${id}', hasBuilder=${builder !== undefined && builder !== null}, ` +
            `instanceId=${this.instanceId_}`);
        if (id === undefined || id === null || builder === undefined || builder === null) {
            prLog(`register: rejected before native call, invalid argument (id or builder is null/undefined)`);
            throw new BusinessError(401, 'Parameter error.');
        }
        const result: number =
            ArkUIAniModule._PlaceholderRegisterRegister(id, builder, this.makeExecution(id, builder));
        prLog(`register: native result=${result}, id='${id}'`);
        if (result === REGISTER_RESULT_ADMITTED) {
            this.builders_.set(id, builder);
            prLog(`register: admitted, id='${id}'`);
            return;
        }
        if (result === REGISTER_RESULT_PARAM_INVALID) {
            prLog(`register: rejected, native parameter validation failed, id='${id}'`);
            throw new BusinessError(401, 'Parameter error.');
        }
        let cause: string;
        if (result === REGISTER_RESULT_NOT_INSTRUMENTED) {
            cause = 'Registration rejected: the builder is not marked with @Builder({ isPlaceholder: true }).';
            prLog(`register: rejected, builder without isPlaceholder instrumentation, id='${id}' ` +
                `(the __aceBegin/EndScrollPlaceholder__ signal never fired during the isolated execution)`);
        } else if (result === REGISTER_RESULT_UNSUPPORTED_COMPONENT) {
            cause = 'Registration rejected: the placeholder template contains an unsupported component.';
            prLog(`register: rejected, template contains an unsupported (uncopyable) component, id='${id}'`);
        } else {
            cause = 'Registration rejected: the placeholder template could not be registered.';
            prLog(`register: rejected, native registration failed (no pipeline context / manager / template ` +
                `registry), result=${result}, id='${id}'`);
        }
        throw new BusinessError(100031, cause);
    }

    /**
     * Static counterpart of the declarative execution-closure contract: the
     * native side never calls the compiled builder bare. Every execution runs
     * through the UIContext's DetachedRootEntryManager (the same route as
     * ComponentSnapshot.createFromBuilder and NavDestination): the manager
     * builds the template subtree on a detached root under a proper memo
     * context with instance scoping, registers the root in detachedRoots_
     * (lifetime hosted by the UIContext), and the product pointer crosses to
     * the native registration through an explicit channel - it arrives as an
     * int64 argument instead of an ani return value, so it never depends on
     * primitive boxing.
     */
    private makeExecution(id: string, builder: WrappedBuilder<CustomBuilder>): () => void {
        const instanceId = this.instanceId_;
        return (): void => {
            prLog(`execution: begin, id='${id}', instanceId=${instanceId}`);
            const uiContext = UIContextUtil.getUIContextById(instanceId);
            if (uiContext === undefined) {
                prLog(`execution: ABORT, no UIContext for instanceId=${instanceId}`);
                return;
            }
            const manager = GlobalStateManager.instance;
            const savedRendering = ObserveSingleton.instance.renderingComponent;
            ObserveSingleton.instance.renderingComponent = ObserveSingleton.RenderingComponentV1;
            const savedExtendable = ExtendableComponent.current;
            ExtendableComponent.current = undefined;
            const needCreate = setNeedCreate(true);
            const savedFrozen = manager.frozen;
            manager.frozen = true;
            try {
                prLog(`execution: building template on detached root (createUiDetachedRoot route)`);
                const peerNode = createUiDetachedRoot(
                    (): PeerNode => ArkComponentRootPeer.create(undefined),
                    builder.builder,
                    instanceId);
                const productPtr = peerNode.peer.ptr;
                this.roots_.set(id, productPtr);
                prLog(`execution: detached root built, productPtr=${productPtr}`);
                ArkUIAniModule._PlaceholderRegisterPublishProduct(productPtr);
                prLog(`execution: product published natively, end`);
            } catch (e) {
                // Surface the failure reason of the isolated execution; the
                // native side records the violation from the ani call status.
                prLog(`execution: builder THREW: ${e}`);
                throw e;
            } finally {
                manager.frozen = savedFrozen;
                setNeedCreate(needCreate);
                ExtendableComponent.current = savedExtendable;
                ObserveSingleton.instance.renderingComponent = savedRendering;
            }
        };
    }

    /**
     * Unregisters the placeholder builder registered under the given ID for this
     * UI instance. Unregistering an ID that is not registered is a silent no-op.
     */
    public unregister(id: string): void {
        prLog(`unregister: enter, id='${id}'`);
        this.builders_.delete(id);
        ArkUIAniModule._PlaceholderRegisterUnregister(id);
        // Tear down the latest template source tree; roots built by earlier
        // executions were already replaced in the registry and stay hosted by
        // the DetachedRootEntryManager until the UIContext goes away.
        const rootPtr = this.roots_.get(id);
        if (rootPtr !== undefined) {
            this.roots_.delete(id);
            const destroyed = destroyUiDetachedRoot(rootPtr, this.instanceId_);
            prLog(`unregister: detached root destroyed=${destroyed}, ptr=${rootPtr}, id='${id}'`);
        }
        prLog(`unregister: done, id='${id}'`);
    }

    /**
     * Checks whether a placeholder builder has been registered under the given ID
     * for this UI instance.
     */
    public isRegistered(id: string): boolean {
        const registered = ArkUIAniModule._PlaceholderRegisterIsRegistered(id);
        prLog(`isRegistered: id='${id}', result=${registered}`);
        return registered;
    }
}

/**
 * Creates the PlaceholderRegister for the UI instance with the given instance ID.
 */
export function createPlaceholderRegister(id: int32): PlaceholderRegister {
    prLog(`createPlaceholderRegister: instanceId=${id}`);
    return new PlaceholderRegister(id);
}
