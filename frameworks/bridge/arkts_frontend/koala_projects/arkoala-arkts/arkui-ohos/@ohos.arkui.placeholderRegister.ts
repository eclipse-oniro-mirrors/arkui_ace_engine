import { SizeOptions } from '/arkui/component/units';
import { CustomBuilder, WrappedBuilder } from '/arkui/component/builder';

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

    /**
     * Registers a placeholder builder under the given ID for scrollable components.
     * The builder must be a global, parameterless **@Builder({ isPlaceholder: true })**
     * function wrapped through **wrapBuilder**.
     */
    public register(id: string, builder: WrappedBuilder<CustomBuilder>): void {
        this.builders_.set(id, builder);
    }

    /**
     * Unregisters the placeholder builder registered under the given ID for this
     * UI instance. Unregistering an ID that is not registered is a silent no-op.
     */
    public unregister(id: string): void {
        this.builders_.delete(id);
    }

    /**
     * Checks whether a placeholder builder has been registered under the given ID
     * for this UI instance.
     */
    public isRegistered(id: string): boolean {
        return this.builders_.has(id);
    }
}

/**
 * Creates the PlaceholderRegister for the UI instance with the given instance ID.
 */
export function createPlaceholderRegister(id: number): PlaceholderRegister {
    return new PlaceholderRegister();
}
