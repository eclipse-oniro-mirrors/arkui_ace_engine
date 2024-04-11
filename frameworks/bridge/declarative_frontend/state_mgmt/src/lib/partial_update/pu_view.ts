/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *  * ViewPU - View for Partial Update
 *
* all definitions in this file are framework internal
*/

/**
 * WeakRef
 * ref to an Object that does not prevent the Object from getting GC'ed
 * current version of tsc does not know about WeakRef
 * but Ark runtime supports it
 *
 */
declare class WeakRef<T extends Object> {
  constructor(o: T);
  deref(): T;
}

declare class DumpLog {
  static print(depth: number, content: string): void;
}

type DFXCommand = { what: string, viewId: number, isRecursive: boolean };
type ProfileRecursionCounter = { total: number };

type ProvidedVarsMapPU = Map<string, ObservedPropertyAbstractPU<any>>;

// function type of partial update function
type UpdateFunc = (elmtId: number, isFirstRender: boolean) => void;
type UIClassObject = { prototype: Object, pop?: () => void };

// UpdateFuncRecord: misc framework-internal info related to updating of a UINode C++ object 
// that TS side needs to know. 
// updateFunc_  lambda function to update the UINode
// JS interface class reference (it only has static functions)
class UpdateFuncRecord {
  private updateFunc_: UpdateFunc;
  private classObject_: UIClassObject;
  private node_?: ArkComponent

  constructor(params: { updateFunc: UpdateFunc, classObject?: UIClassObject, node?: ArkComponent }) {
    this.updateFunc_ = params.updateFunc;
    this.classObject_ = params.classObject;
    this.node_ = params.node;
  }

  public getUpdateFunc(): UpdateFunc | undefined {
    return this.updateFunc_;
  }

  public getComponentClass(): UIClassObject | undefined {
    return this.classObject_;
  }

  public getComponentName(): string {
    return (this.classObject_ && ("name" in this.classObject_)) ? Reflect.get(this.classObject_, "name") as string : "unspecified UINode";
  }

  public getPopFunc(): () => void {
    return (this.classObject_ && "pop" in this.classObject_) ? this.classObject_.pop! : () => { };
  }

  public getNode(): ArkComponent | undefined {
    return this.node_;
  }

  public setNode(node: ArkComponent | undefined): void {
    this.node_ = node;
  }
}

// function type of recycle node update function
type RecycleUpdateFunc = (elmtId: number, isFirstRender: boolean, recycleNode: ViewPU) => void;

type ExtraInfo = { page: string, line: number };

// NativeView
// implemented in C++  for release
// and in utest/view_native_mock.ts for testing
abstract class ViewPU extends NativeViewPartialUpdate
  implements IViewPropertiesChangeSubscriber {

  // Array.sort() converts array items to string to compare them!
  static readonly compareNumber = (a: number, b: number): number => {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
  };

  // List of inactive components used for Dfx
  private static readonly inactiveComponents_: Set<string> = new Set<string>();

  private id_: number;

  private parent_: ViewPU = undefined;
  private childrenWeakrefMap_ = new Map<number, WeakRef<ViewPU>>();

  // flag for initial rendering or re-render on-going.
  private isRenderInProgress: boolean = false;

  // flag for initial rendering being done
  private isInitialRenderDone: boolean = false;

  // indicates the currently rendered or rendered UINode's elmtIds
  // or UINodeRegisterProxy.notRecordingDependencies if none is currently rendering
  // isRenderInProgress == true always when currentlyRenderedElmtIdStack_.length >= 0 
  private currentlyRenderedElmtIdStack_: Array<number> = new Array<number>();

  // static flag for paused rendering
  // when paused, getCurrentlyRenderedElmtId() will return UINodeRegisterProxy.notRecordingDependencies
  private static renderingPaused: boolean = false;

  // flag if active of inActive
  // inActive means updates are delayed
  private isActive_: boolean = true;

  /*
  private elmtIdsDelayedUpdate: Set<number> = new Set()
  private monitorIdsDelayedUpdate: Set<number> = new Set()
  private computedIdsDelayedUpdate: Set<number> = new Set()
  */

  private runReuse_: boolean = false;
  private hasBeenRecycled_: boolean = false;

  private paramsGenerator_: () => Object;

  // flag if {aboutToBeDeletedInternal} is called and the instance of ViewPU has not been GC.
  private isDeleting_: boolean = false;

  private watchedProps: Map<string, (propName: string) => void>
    = new Map<string, (propName: string) => void>();

  private recycleManager_: RecycleManager = undefined;

  private isCompFreezeAllowed: boolean = false;

  private extraInfo_: ExtraInfo = undefined;

  // @Provide'd variables by this class and its ancestors
  protected providedVars_: ProvidedVarsMapPU = new Map<string, ObservedPropertyAbstractPU<any>>();

  // Map elmtId -> Repeat instance in this ViewPU
  private elmtId2Repeat_: Map<number, RepeatAPI<any>> = new Map<number, RepeatAPI<any>>();

  // Set of dependent elmtIds that need partial update
  // during next re-render
  protected dirtDescendantElementIds_: Set<number>
    = new Set<number>();

  // registry of update functions
  // the key is the elementId of the Component/Element that's the result of this function
  private updateFuncByElmtId = new UpdateFuncsByElmtId();

  // my LocalStorage instance, shared with ancestor Views.
  // create a default instance on demand if none is initialized
  protected localStoragebackStore_: LocalStorage = undefined;

  private ownObservedPropertiesStore__?: Set<ObservedPropertyAbstractPU<any>>;

  private get ownObservedPropertiesStore_() {
    if (!this.ownObservedPropertiesStore__) {
      // lazy init
      this.ownObservedPropertiesStore__ = new Set<ObservedPropertyAbstractPU<any>>();
      this.obtainOwnObservedProperties();
    }
    return this.ownObservedPropertiesStore__;
  }

  protected obtainOwnObservedProperties(): void {
    let usesStateMgmtVersion = 0;
    Object.getOwnPropertyNames(this)
      .filter((propName) => {
        // do not include backing store, and ObserveV3/MonitorV3/ComputedV3 meta data objects
        return (propName.startsWith("__") 
        && !propName.startsWith(ObserveV3.OB_PREFIX) 
        && !propName.startsWith(MonitorV3.WATCH_PREFIX)
        && !propName.startsWith(ComputedV3.COMPUTED_PREFIX))
      })
      .forEach((propName) => {
        const stateVar = Reflect.get(this, propName) as Object;
        if (stateVar && typeof stateVar === 'object' && "notifyPropertyHasChangedPU" in stateVar) {
          stateMgmtConsole.debug(`... add state variable ${propName} to ${stateVar}`)
          this.ownObservedPropertiesStore_.add(stateVar as unknown as ObservedPropertyAbstractPU<any>);
          usesStateMgmtVersion = 2;
        } else {
          stateMgmtConsole.debug(`${this.debugInfo__()} ${propName} application may use an unregular naming style, or stateVar may be Non-Object.`);
        }
      });

    if (this.isViewV3==true) {
      if (usesStateMgmtVersion==2) {
        const error = `${this.debugInfo__()}: mixed use of stateMgmt V2 and V3 variable decorators. Application error!`;
        stateMgmtConsole.applicationError(error);
        throw new Error(error);
      }
    }
    stateMgmtConsole.debug(`${this.debugInfo__()}: uses stateMgmt version ${this.isViewV3==true ? 3 : 2}`);
  }

  protected get localStorage_() {
    if (!this.localStoragebackStore_ && this.parent_) {
      stateMgmtConsole.debug(`${this.debugInfo__()}: constructor: get localStorage_ : Using LocalStorage instance of the parent View.`);
      this.localStoragebackStore_ = this.parent_.localStorage_;
    }

    if (!this.localStoragebackStore_) {
      stateMgmtConsole.info(`${this.debugInfo__()}: constructor: is accessing LocalStorage without being provided an instance. Creating a default instance.`);
      this.localStoragebackStore_ = new LocalStorage({ /* empty */ });
    }
    return this.localStoragebackStore_;
  }

  protected set localStorage_(instance: LocalStorage) {
    if (!instance) {
      // setting to undefined not allowed
      return;
    }
    if (this.localStoragebackStore_) {
      stateMgmtConsole.applicationError(`${this.debugInfo__()}: constructor: is setting LocalStorage instance twice. Application error.`);
    }
    this.localStoragebackStore_ = instance;
  }

  // FIXME 
  // indicate if this is  V2 or a V3 component
  // V2 by default, changed to V3 by the first V3 decorated variable
  // when splitting ViewPU and ViewV3
  // use instanceOf. Until then, this is a workaround.
  // @state, @track, etc V3 decorator functions modify isViewV3 to return true
  // (decorator can modify functions in prototype)
  private get isViewV3() : boolean {
    return false;
  }

  /**
   * Create a View
   *
   * 1. option: top level View, specify
   *    - compilerAssignedUniqueChildId must specify
   *    - parent=undefined
   *    - localStorage  must provide if @LocalSTorageLink/Prop variables are used
   *      in this View or descendant Views.
   *
   * 2. option: not a top level View
   *    - compilerAssignedUniqueChildId must specify
   *    - parent must specify
   *    - localStorage do not specify, will inherit from parent View.
   *
  */
  constructor(parent: ViewPU, localStorage: LocalStorage, elmtId: number = UINodeRegisterProxy.notRecordingDependencies, extraInfo: ExtraInfo = undefined) {
    super();
    // if set use the elmtId also as the ViewPU object's subscribable id.
    // these matching is requirement for updateChildViewById(elmtId) being able to
    // find the child ViewPU object by given elmtId
    this.id_ = elmtId == UINodeRegisterProxy.notRecordingDependencies ? SubscriberManager.MakeId() : elmtId;

    this.localStoragebackStore_ = undefined;
    stateMgmtConsole.debug(`ViewPU constructor: Creating @Component '${this.constructor.name}' from parent '${parent?.constructor.name}'`);
    if (extraInfo) {
      this.extraInfo_ = extraInfo;
    }
    if (parent) {
      // this View is not a top-level View
      this.setCardId(parent.getCardId());
      // Call below will set this.parent_ to parent as well
      parent.addChild(this);
    }
    if (localStorage) {
      this.localStorage_ = localStorage;
      stateMgmtConsole.debug(`${this.debugInfo__()}: constructor: Using LocalStorage instance provided via @Entry or view instance creation.`);
    }
    this.isCompFreezeAllowed = this.isCompFreezeAllowed || (this.parent_ && this.parent_.isCompFreezeAllowed);

    SubscriberManager.Add(this);
    stateMgmtConsole.debug(`${this.debugInfo__()}: constructor: done`);
  }

  // globally unique id, this is different from compilerAssignedUniqueChildId!
  id__(): number {
    return this.id_;
  }

  updateId(elmtId: number): void {
    this.id_ = elmtId;
  }

  // inform the subscribed property
  // that the View and thereby all properties
  // are about to be deleted
  abstract aboutToBeDeleted(): void;

  aboutToReuse(params: Object): void { }

  aboutToRecycle(): void { }

  private setDeleteStatusRecursively(): void {
    if (!this.childrenWeakrefMap_.size) {
      return;
    }
    this.childrenWeakrefMap_.forEach((value: WeakRef<ViewPU>) => {
      let child: ViewPU = value.deref();
      if (child) {
        child.isDeleting_ = true;
        child.setDeleteStatusRecursively();
      }
    })
  }

  // super class will call this function from
  // its aboutToBeDeleted implementation
  protected aboutToBeDeletedInternal(): void {
    stateMgmtConsole.debug(`${this.debugInfo__()}: aboutToBeDeletedInternal`);
    // if this.isDeleting_ is true already, it may be set delete status recursively by its parent, so it is not necessary
    // to set and resursively set its children any more
    if (!this.isDeleting_) {
      this.isDeleting_ = true;
      this.setDeleteStatusRecursively();
    }
    // tell UINodeRegisterProxy that all elmtIds under
    // this ViewPU should be treated as already unregistered

    stateMgmtConsole.debug(`${this.constructor.name}: aboutToBeDeletedInternal `);

    // purge the elmtIds owned by this viewPU from the updateFuncByElmtId and also the state variable dependent elmtIds
    Array.from(this.updateFuncByElmtId.keys()).forEach((elmtId: number) => {
      this.purgeDeleteElmtId(elmtId);
    })

    if (this.hasRecycleManager()) {
      this.getRecycleManager().purgeAllCachedRecycleNode();
    }

    // unregistration of ElementIDs
    stateMgmtConsole.debug(`${this.debugInfo__()}: onUnRegElementID`);

    // it will unregister removed elementids from all the viewpu, equals purgeDeletedElmtIdsRecursively
    this.purgeDeletedElmtIds();

    // unregisters its own id once its children are unregistered above
    UINodeRegisterProxy.unregisterRemovedElmtsFromViewPUs([this.id__()]);
    
    stateMgmtConsole.debug(`${this.debugInfo__()}: onUnRegElementID  - DONE`);

    // in case ViewPU is currently frozen
    ViewPU.inactiveComponents_.delete(`${this.constructor.name}[${this.id__()}]`);

    MonitorV3.clearWatchesFromTarget(this);
    
    this.updateFuncByElmtId.clear();
    this.watchedProps.clear();
    this.providedVars_.clear();
    if (this.ownObservedPropertiesStore__) {
      this.ownObservedPropertiesStore__.clear();
    }
    if (this.parent_) {
      this.parent_.removeChild(this);
    }
    this.localStoragebackStore_ = undefined;
  }

  public purgeDeleteElmtId(rmElmtId: number): boolean {
    stateMgmtConsole.debug(`${this.debugInfo__} is purging the rmElmtId:${rmElmtId}`);
    const result = this.updateFuncByElmtId.delete(rmElmtId);
    if (result) {
      this.purgeVariableDependenciesOnElmtIdOwnFunc(rmElmtId);
      // it means rmElmtId has finished all the unregistration from the js side, ElementIdToOwningViewPU_  does not need to keep it
      UINodeRegisterProxy.ElementIdToOwningViewPU_.delete(rmElmtId);
    }
    
    // FIXME: only do this if app uses V3
    ObserveV3.getObserve().clearBinding(rmElmtId);
    return result;
  }

  public debugInfo__(): string {
    return `@Component '${this.constructor.name}'[${this.id__()}]`;
  }

  public debugInfoRegisteredElmtIds() {
    return this.updateFuncByElmtId.debugInfoRegisteredElmtIds();
  }

  // for given elmtIds look up their component name/type and format a string out of this info
  // use function only for debug output and DFX.
  public debugInfoElmtIds(elmtIds: Array<number>): string {
    let result: string = "";
    let sepa: string = "";
    elmtIds.forEach((elmtId: number) => {
      result += `${sepa}${this.debugInfoElmtId(elmtId)}`;
      sepa = ", ";
    });
    return result;
  }

  public debugInfoElmtId(elmtId: number): string {
    return this.updateFuncByElmtId.debugInfoElmtId(elmtId);
  }

  public dumpStateVars(): void {
    stateMgmtConsole.debug(`${this.debugInfo__()}:  State variables:\n ${this.debugInfoStateVars()}`);
  }

  private debugInfoStateVars(): string {
    let result: string = `|--${this.constructor.name}[${this.id__()}]`;
    Object.getOwnPropertyNames(this)
      .filter((varName: string) => varName.startsWith("__") && !varName.startsWith(ObserveV3.OB_PREFIX) )
      .forEach((varName) => {
        const prop: any = Reflect.get(this, varName);
        if ("debugInfoDecorator" in prop) {
          const observedProp = prop as ObservedPropertyAbstractPU<any>;
          result += `\n  ${observedProp.debugInfoDecorator()} '${observedProp.info()}'[${observedProp.id__()}]`;
          result += `\n  ${observedProp.debugInfoSubscribers()}`
          result += `\n  ${observedProp.debugInfoSyncPeers()}`;
          result += `\n  ${observedProp.debugInfoDependentElmtIds()}`
          result += `\n  ${observedProp.debugInfoDependentComponents()}`
        }
      });
    return result;
  }

  public isViewActive(): boolean {
    return this.isActive_;
  }

  /**
 * ArkUI engine will call this function when the corresponding CustomNode's active status change.
 * @param active true for active, false for inactive
 */
  public setActiveInternal(active: boolean): void {
    stateMgmtProfiler.begin("ViewPU.setActive");
    if (!this.isCompFreezeAllowed) {
      stateMgmtConsole.debug(`${this.debugInfo__()}: ViewPU.setActive. Component freeze state is ${this.isCompFreezeAllowed} - ignoring`);
      stateMgmtProfiler.end();
      return;
    }
    stateMgmtConsole.debug(`${this.debugInfo__()}: ViewPU.setActive ${active ? ' inActive -> active' : 'active -> inActive'}`);
    this.isActive_ = active;
    if (this.isActive_) {
      this.onActiveInternal()
    } else {
      this.onInactiveInternal();
    }
    stateMgmtProfiler.end();
  }

  private onActiveInternal(): void {
    if (!this.isActive_) {
      return;
    }

    stateMgmtConsole.debug(`${this.debugInfo__()}: onActiveInternal`);
    this.performDelayedUpdate();
    // Remove the active component from the Map for Dfx
    ViewPU.inactiveComponents_.delete(`${this.constructor.name}[${this.id__()}]`);
    for (const child of this.childrenWeakrefMap_.values()) {
      const childViewPU: ViewPU | undefined = child.deref();
      if (childViewPU) {
        childViewPU.setActiveInternal(this.isActive_);
      }
    }
    if (this.hasRecycleManager()) {
      this.getRecycleManager().setActive(this.isActive_);
    }
  }


  private onInactiveInternal(): void {
    if (this.isActive_) {
      return;
    }

    stateMgmtConsole.debug(`${this.debugInfo__()}: onInactiveInternal`);
    for (const stateLinkProp of this.ownObservedPropertiesStore_) {
      stateLinkProp.enableDelayedNotification();
    }
    // Add the inactive Components to Map for Dfx listing
    ViewPU.inactiveComponents_.add(`${this.constructor.name}[${this.id__()}]`);

    for (const child of this.childrenWeakrefMap_.values()) {
      const childViewPU: ViewPU | undefined = child.deref();
      if (childViewPU) {
        childViewPU.setActiveInternal(this.isActive_);
      }
    }
    if (this.hasRecycleManager()) {
      this.getRecycleManager().setActive(this.isActive_);
    }
  }

  private setParent(parent: ViewPU) {
    if (this.parent_ && parent) {
      stateMgmtConsole.warn(`${this.debugInfo__()}: setChild: changing parent to '${parent?.debugInfo__()} (unsafe operation)`);
    }
    this.parent_ = parent;
  }

  /**
   * Indicate if this @Component is allowed to freeze by calling with freezeState=true
   * Called with value of the @Component decorator 'freezeWhenInactive' parameter
   * or depending how UI compiler works also with 'undefined'
   * @param freezeState only value 'true' will be used, otherwise inherits from parent
   *      if not parent, set to false.
   */
  protected initAllowComponentFreeze(freezeState: boolean | undefined): void {
    // set to true if freeze parameter set for this @Component to true
    // otherwise inherit from parent @Component (if it exists).
    this.isCompFreezeAllowed = freezeState || this.isCompFreezeAllowed;
    stateMgmtConsole.debug(`${this.debugInfo__()}: @Component freezeWhenInactive state is set to ${this.isCompFreezeAllowed}`);
  }

  /**
   * add given child and set 'this' as its parent
   * @param child child to add
   * @returns returns false if child with given child's id already exists
   *
   * framework internal function
   * Note: Use of WeakRef ensures child and parent do not generate a cycle dependency.
   * The add. Set<ids> is required to reliably tell what children still exist.
   */
  public addChild(child: ViewPU): boolean {
    if (this.childrenWeakrefMap_.has(child.id__())) {
      stateMgmtConsole.warn(`${this.debugInfo__()}: addChild '${child?.debugInfo__()}' id already exists ${child.id__()}. Internal error!`);
      return false;
    }
    this.childrenWeakrefMap_.set(child.id__(), new WeakRef(child));
    child.setParent(this);
    return true;
  }

  /**
   * remove given child and remove 'this' as its parent
   * @param child child to add
   * @returns returns false if child with given child's id does not exist
   */
  public removeChild(child: ViewPU): boolean {
    const hasBeenDeleted = this.childrenWeakrefMap_.delete(child.id__());
    if (!hasBeenDeleted) {
      stateMgmtConsole.warn(`${this.debugInfo__()}: removeChild '${child?.debugInfo__()}', child id ${child.id__()} not known. Internal error!`);
    } else {
      child.setParent(undefined);
    }
    return hasBeenDeleted;
  }

  /**
   * Retrieve child by given id
   * @param id
   * @returns child if in map and weak ref can still be downreferenced
   */
  public getChildById(id: number) {
    const childWeakRef = this.childrenWeakrefMap_.get(id);
    return childWeakRef ? childWeakRef.deref() : undefined;
  }

  protected abstract purgeVariableDependenciesOnElmtId(removedElmtId: number);
  protected abstract initialRender(): void;
  protected abstract rerender(): void;
  protected abstract updateRecycleElmtId(oldElmtId: number, newElmtId: number): void;
  protected updateStateVars(params: {}): void {
    stateMgmtConsole.error(`${this.debugInfo__()}: updateStateVars unimplemented. Pls upgrade to latest eDSL transpiler version. Application error.`);
  }

  protected initialRenderView(): void {
    stateMgmtProfiler.begin("ViewPU.initialRenderView");
    this.obtainOwnObservedProperties();
    this.isRenderInProgress = true;
    this.initialRender();
    this.isRenderInProgress = false;
    this.isInitialRenderDone = true;
    stateMgmtProfiler.end();
  }

  public UpdateElement(elmtId: number): void {
    stateMgmtProfiler.begin("ViewPU.UpdateElement");
    if (elmtId == this.id__()) {
      // do not attempt to update itself.
      // a @Prop can add a dependency of the ViewPU onto itself. Ignore it.
      stateMgmtProfiler.end();
      return;
    }

    // do not process an Element that has been marked to be deleted
    const entry: UpdateFuncRecord | undefined = this.updateFuncByElmtId.get(elmtId);
    const updateFunc = entry ? entry.getUpdateFunc() : undefined;

    if (typeof updateFunc !== "function") {
      stateMgmtConsole.debug(`${this.debugInfo__()}: UpdateElement: update function of elmtId ${elmtId} not found, internal error!`);
    } else {
      stateMgmtConsole.debug(`${this.debugInfo__()}: UpdateElement: re-render of ${entry.getComponentName()} elmtId ${elmtId} start ...`);
      this.isRenderInProgress = true;
      stateMgmtProfiler.begin("ViewPU.updateFunc");
      updateFunc(elmtId, /* isFirstRender */ false);
      stateMgmtProfiler.end();
      stateMgmtProfiler.begin("ViewPU.finishUpdateFunc (native)");
      this.finishUpdateFunc(elmtId);
      stateMgmtProfiler.end();
      this.isRenderInProgress = false;
      stateMgmtConsole.debug(`${this.debugInfo__()}: UpdateElement: re-render of ${entry.getComponentName()} elmtId ${elmtId} - DONE`);
    }
    stateMgmtProfiler.end();
  }

  public dumpReport(): void {
    stateMgmtConsole.warn(`Printing profiler information`);
    stateMgmtProfiler.report();
  }

  /**
   * force a complete rerender / update by executing all update functions
   * exec a regular rerender first
   *
   * @param deep recurse all children as well
   *
   * framework internal functions, apps must not call
   */
  public forceCompleteRerender(deep: boolean = false): void {
    stateMgmtProfiler.begin("ViewPU.forceCompleteRerender");
    stateMgmtConsole.debug(`${this.debugInfo__()}: forceCompleteRerender - start.`);

    // see which elmtIds are managed by this View
    // and clean up all book keeping for them
    this.purgeDeletedElmtIds();

    Array.from(this.updateFuncByElmtId.keys()).sort(ViewPU.compareNumber).forEach(elmtId => this.UpdateElement(elmtId));

    if (deep) {
      this.childrenWeakrefMap_.forEach((weakRefChild: WeakRef<ViewPU>) => {
        const child = weakRefChild.deref();
        if (child) {
          (child as ViewPU).forceCompleteRerender(true);
        }
      });
    }
    stateMgmtConsole.debug(`${this.debugInfo__()}: forceCompleteRerender - end`);
    stateMgmtProfiler.end();
  }

  /**
   * force a complete rerender / update on specific node by executing update function.
   *
   * @param elmtId which node needs to update.
   *
   * framework internal functions, apps must not call
   */
  public forceRerenderNode(elmtId: number): void {
    stateMgmtProfiler.begin("ViewPU.forceRerenderNode");
    // see which elmtIds are managed by this View
    // and clean up all book keeping for them
    this.purgeDeletedElmtIds();
    this.UpdateElement(elmtId);

    // remove elemtId from dirtDescendantElementIds.
    this.dirtDescendantElementIds_.delete(elmtId);
    stateMgmtProfiler.end();
  }

  public updateStateVarsOfChildByElmtId(elmtId, params: Object): void {
    stateMgmtProfiler.begin("ViewPU.updateStateVarsOfChildByElmtId");
    stateMgmtConsole.debug(`${this.debugInfo__()}: updateChildViewById(${elmtId}) - start`);

    if (elmtId < 0) {
      stateMgmtConsole.warn(`${this.debugInfo__()}: updateChildViewById(${elmtId}) - invalid elmtId - internal error!`);
      stateMgmtProfiler.end();
      return;
    }
    let child: ViewPU = this.getChildById(elmtId);
    if (!child) {
      stateMgmtConsole.warn(`${this.debugInfo__()}: updateChildViewById(${elmtId}) - no child with this elmtId - internal error!`);
      stateMgmtProfiler.end();
      return;
    }
    child.updateStateVars(params);
    stateMgmtConsole.debug(`${this.debugInfo__()}: updateChildViewById(${elmtId}) - end`);
    stateMgmtProfiler.end();
  }

  // implements IMultiPropertiesChangeSubscriber
  viewPropertyHasChanged(varName: PropertyInfo, dependentElmtIds: Set<number>): void {
    stateMgmtProfiler.begin("ViewPU.viewPropertyHasChanged");
    aceTrace.begin("ViewPU.viewPropertyHasChanged", this.constructor.name, varName, dependentElmtIds.size);
    if (this.isRenderInProgress) {
      stateMgmtConsole.applicationError(`${this.debugInfo__()}: State variable '${varName}' has changed during render! It's illegal to change @Component state while build (initial render or re-render) is on-going. Application error!`);
    }

    this.syncInstanceId();

    if (dependentElmtIds.size && !this.isFirstRender()) {
      if (!this.dirtDescendantElementIds_.size && !this.runReuse_) {
        // mark ComposedElement dirty when first elmtIds are added
        // do not need to do this every time
        this.markNeedUpdate();
      }
      stateMgmtConsole.debug(`${this.debugInfo__()}: viewPropertyHasChanged property: elmtIds that need re-render due to state variable change: ${this.debugInfoElmtIds(Array.from(dependentElmtIds))} .`)
      for (const elmtId of dependentElmtIds) {
        if (this.hasRecycleManager()) {
          this.dirtDescendantElementIds_.add(this.recycleManager_.proxyNodeId(elmtId));
        } else {
          this.dirtDescendantElementIds_.add(elmtId);
        }
      }
      stateMgmtConsole.debug(`   ... updated full list of elmtIds that need re-render [${this.debugInfoElmtIds(Array.from(this.dirtDescendantElementIds_))}].`)
    } else {
      stateMgmtConsole.debug(`${this.debugInfo__()}: viewPropertyHasChanged: state variable change adds no elmtIds for re-render`);
      stateMgmtConsole.debug(`   ... unchanged full list of elmtIds that need re-render [${this.debugInfoElmtIds(Array.from(this.dirtDescendantElementIds_))}].`)
    }

    let cb = this.watchedProps.get(varName)
    if (cb && typeof cb === "function") {
      stateMgmtConsole.debug(`   ... calling @Watch function`);
      cb.call(this, varName);
    }

    this.restoreInstanceId();
    aceTrace.end();
    stateMgmtProfiler.end();
  }

  /**
   *  inform that UINode with given elmtId needs rerender
   *  does NOT exec @Watch function.
   *  only used on V3 code path from ObserveV3.fireChange.
   * 
   * FIXME will still use in the future?
   */
  public uiNodeNeedUpdateV3(elmtId: number): void {
    if (this.isFirstRender()) {
      return;
    }

    stateMgmtProfiler.begin(`ViewPU.uiNodeNeedUpdate ${this.debugInfoElmtId(elmtId)}`);

    if (!this.dirtDescendantElementIds_.size && !this.runReuse_) {
      // mark ComposedElement dirty when first elmtIds are added
      // do not need to do this every time
      this.syncInstanceId();
      this.markNeedUpdate();
      this.restoreInstanceId();
    }
    if (this.hasRecycleManager()) {
      this.dirtDescendantElementIds_.add(this.recycleManager_.proxyNodeId(elmtId));
    } else {
      this.dirtDescendantElementIds_.add(elmtId);
    }
    stateMgmtConsole.debug(`ViewPU.uiNodeNeedUpdate: updated full list of elmtIds that need re-render [${this.debugInfoElmtIds(Array.from(this.dirtDescendantElementIds_))}].`)

    stateMgmtProfiler.end();
  }


  /*
  private performDelayedUpdateV3(): void {
    FIXME Component freeze 
    if(this.computedIdsDelayedUpdate.size) {
      // exec computed functions
      ObserveV3.getObserve().updateDirtyComputedProps(this.computedIdsDelayedUpdate);
    }
    if(this.monitorIdsDelayedUpdate.size) {
      // exec  monitor functions
      ObserveV3.getObserve().updateDirtyMonitors(this.monitorIdsDelayedUpdate);
    }
    if(this.elmtIdsDelayedUpdate.size) {
      // update re-render of updated element ids once the view gets active
      if(this.dirtDescendantElementIds_.size === 0) {
        this.dirtDescendantElementIds_ = new Set(this.elmtIdsDelayedUpdate);
      }
      else {
        this.elmtIdsDelayedUpdate.forEach((element) => {
          this.dirtDescendantElementIds_.add(element);
        });
      }
    }
    this.markNeedUpdate();
    this.elmtIdsDelayedUpdate.clear();
    this.monitorIdsDelayedUpdate.clear();
    this.computedIdsDelayedUpdate.clear();
  }
}
   */

  private performDelayedUpdate(): void {
    if (!this.ownObservedPropertiesStore_.size) {
      return;
    }
    stateMgmtProfiler.begin("ViewPU.performDelayedUpdate");
    aceTrace.begin("ViewPU.performDelayedUpdate", this.constructor.name);
    stateMgmtConsole.debug(`${this.debugInfo__()}: performDelayedUpdate start ...`);
    this.syncInstanceId();

    for (const stateLinkPropVar of this.ownObservedPropertiesStore_) {
      const changedElmtIds = stateLinkPropVar.moveElmtIdsForDelayedUpdate();
      if (changedElmtIds) {
        const varName = stateLinkPropVar.info();
        if (changedElmtIds.size && !this.isFirstRender()) {
          for (const elmtId of changedElmtIds) {
            this.dirtDescendantElementIds_.add(elmtId);
          }
        }

        stateMgmtConsole.debug(`${this.debugInfo__()}: performDelayedUpdate: all elmtIds that need re-render [${Array.from(this.dirtDescendantElementIds_).toString()}].`);

        const cb = this.watchedProps.get(varName)
        if (cb) {
          stateMgmtConsole.debug(`   ... calling @Watch function`);
          cb.call(this, varName);
        }
      }
    } // for all ownStateLinkProps_
    this.restoreInstanceId();

    if (this.dirtDescendantElementIds_.size) {
      this.markNeedUpdate();
    }
    aceTrace.end()
    stateMgmtProfiler.end();
  }

  /**
   * Function to be called from the constructor of the sub component
   * to register a @Watch variable
   * @param propStr name of the variable. Note from @Provide and @Consume this is
   *      the variable name and not the alias!
   * @param callback application defined member function of sub-class
   */
  protected declareWatch(propStr: string, callback: (propName: string) => void): void {
    this.watchedProps.set(propStr, callback);
  }

  /**
   * This View @Provide's a variable under given name
   * Call this function from the constructor of the sub class
   * @param providedPropName either the variable name or the alias defined as
   *        decorator param
   * @param store the backing store object for this variable (not the get/set variable!)
   */
  protected addProvidedVar<T>(providedPropName: string, store: ObservedPropertyAbstractPU<T>, allowOverride: boolean = false) {
    if (!allowOverride && this.findProvide(providedPropName)) {
      throw new ReferenceError(`${this.constructor.name}: duplicate @Provide property with name ${providedPropName}. Property with this name is provided by one of the ancestor Views already. @Provide override not allowed.`);
    }
    store.setDecoratorInfo("@Provide");
    this.providedVars_.set(providedPropName, store);
  }

  /*
    findProvide finds @Provided property recursively by traversing ViewPU's towards that of the UI tree root @Component:
    if 'this' ViewPU has a @Provide("providedPropName") return it, otherwise ask from its parent ViewPU.
  */
  public findProvide(providedPropName: string): ObservedPropertyAbstractPU<any> | undefined {
    return this.providedVars_.get(providedPropName) || (this.parent_ && this.parent_.findProvide(providedPropName));
  }

  /**
   * Method for the sub-class to call from its constructor for resolving
   *       a @Consume variable and initializing its backing store
   *       with the SyncedPropertyTwoWay<T> object created from the
   *       @Provide variable's backing store.
   * @param providedPropName the name of the @Provide'd variable.
   *     This is either the @Consume decorator parameter, or variable name.
   * @param consumeVarName the @Consume variable name (not the
   *            @Consume decorator parameter)
   * @returns initializing value of the @Consume backing store
   */
  protected initializeConsume<T>(providedPropName: string,
    consumeVarName: string): ObservedPropertyAbstractPU<T> {
    let providedVarStore: ObservedPropertyAbstractPU<any> = this.findProvide(providedPropName);
    if (providedVarStore === undefined) {
      throw new ReferenceError(`${this.debugInfo__()} missing @Provide property with name ${providedPropName}.
          Fail to resolve @Consume(${providedPropName}).`);
    }

    const factory = <T>(source: ObservedPropertyAbstract<T>) => {
      const result: ObservedPropertyAbstractPU<T> = new SynchedPropertyTwoWayPU<T>(source, this, consumeVarName);
      result.setDecoratorInfo("@Consume");
      stateMgmtConsole.debug(`The @Consume is instance of ${result.constructor.name}`);
      return result;
    };
    return providedVarStore.createSync(factory) as ObservedPropertyAbstractPU<T>;
  }


  /**
   * given the elmtId of a child or child of child within this custom component
   * remember this component needs a partial update
   * @param elmtId
   */
  public markElemenDirtyById(elmtId: number): void {
    // TODO ace-ets2bundle, framework, compilated apps need to update together
    // this function will be removed after a short transiition periode
    stateMgmtConsole.applicationError(`${this.debugInfo__()}: markElemenDirtyById no longer supported.
        Please update your ace-ets2bundle and recompile your application. Application error!`);
  }

  /**
   * For each recorded dirty Element in this custom component
   * run its update function
   *
   */
  public updateDirtyElements() {
    stateMgmtProfiler.begin("ViewPU.updateDirtyElements");
    do {
      stateMgmtConsole.debug(`${this.debugInfo__()}: updateDirtyElements (re-render): sorted dirty elmtIds: ${Array.from(this.dirtDescendantElementIds_).sort(ViewPU.compareNumber)}, starting ....`);

      // see which elmtIds are managed by this View
      // and clean up all book keeping for them
      this.purgeDeletedElmtIds();

      // process all elmtIds marked as needing update in ascending order.
      // ascending order ensures parent nodes will be updated before their children
      // prior cleanup ensure no already deleted Elements have their update func executed
      const dirtElmtIdsFromRootNode= Array.from(this.dirtDescendantElementIds_).sort(ViewPU.compareNumber);
      // if state changed during exec update lambda inside UpdateElement, then the dirty elmtIds will be added 
      // to newly created this.dirtDescendantElementIds_ Set
      dirtElmtIdsFromRootNode.forEach(elmtId => {
        if (this.hasRecycleManager()) {
          this.UpdateElement(this.recycleManager_.proxyNodeId(elmtId));
        } else {
          this.UpdateElement(elmtId);
        }
        this.dirtDescendantElementIds_.delete(elmtId);
      });

      if (this.dirtDescendantElementIds_.size) {
        stateMgmtConsole.applicationError(`${this.debugInfo__()}: New UINode objects added to update queue while re-render! - Likely caused by @Component state change during build phase, not allowed. Application error!`);
      }
    } while (this.dirtDescendantElementIds_.size);
    stateMgmtConsole.debug(`${this.debugInfo__()}: updateDirtyElements (re-render) - DONE, dump of ViewPU in next lines`);
    //this.dumpStateVars();
    stateMgmtProfiler.end();
  }

  // request list of all (global) elmtIds of deleted UINodes and unregister from the all ViewPUs
  // this function equals purgeDeletedElmtIdsRecursively because it does unregistration for all ViewPUs
  protected purgeDeletedElmtIds(): void {
    stateMgmtConsole.debug(`purgeDeletedElmtIds @Component '${this.constructor.name}' (id: ${this.id__()}) start ...`)
    // request list of all (global) elmtIds of deleted UINodes that need to be unregistered
    UINodeRegisterProxy.obtainDeletedElmtIds();
    // unregister the removed elementids requested from the cpp side for all viewpus, it will make the first viewpu slower
    // than before, but the rest viewpu will be faster
    UINodeRegisterProxy.unregisterElmtIdsFromViewPUs();
    stateMgmtConsole.debug(`purgeDeletedElmtIds @Component '${this.constructor.name}' (id: ${this.id__()}) end... `)
  }


  protected purgeVariableDependenciesOnElmtIdOwnFunc(elmtId: number): void {
    this.ownObservedPropertiesStore_.forEach((stateVar: ObservedPropertyAbstractPU<any>) => {
      stateVar.purgeDependencyOnElmtId(elmtId);
    })
  }

  /**
   * return its elmtId if currently rendering or re-rendering an UINode
   * otherwise return UINodeRegisterProxy.notRecordingDependencies
   * set in observeComponentCreation(2)
   */
  public getCurrentlyRenderedElmtId() {
    return ViewPU.renderingPaused || this.currentlyRenderedElmtIdStack_.length == 0 
      ? UINodeRegisterProxy.notRecordingDependencies 
      : this.currentlyRenderedElmtIdStack_[this.currentlyRenderedElmtIdStack_.length-1];
  }

  public static pauseRendering() {
    ViewPU.renderingPaused = true;
  }

  public static restoreRendering() {
    ViewPU.renderingPaused = false;
  }

  // executed on first render only
  // kept for backward compatibility with old ace-ets2bundle
  public observeComponentCreation(compilerAssignedUpdateFunc: UpdateFunc): void {
    if (this.isDeleting_) {
      stateMgmtConsole.error(`View ${this.constructor.name} elmtId ${this.id__()} is already in process of destruction, will not execute observeComponentCreation `);
      return;
    }
    const updateFunc = (elmtId: number, isFirstRender: boolean) => {
      stateMgmtConsole.debug(`${this.debugInfo__()}: ${isFirstRender ? `First render` : `Re-render/update`} start ....`);
      this.currentlyRenderedElmtIdStack_.push(elmtId);
      compilerAssignedUpdateFunc(elmtId, isFirstRender);
      this.currentlyRenderedElmtIdStack_.pop();
      stateMgmtConsole.debug(`${this.debugInfo__()}: ${isFirstRender ? `First render` : `Re-render/update`} - DONE ....`);
    }

    const elmtId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
    // in observeComponentCreation function we do not get info about the component name, in 
    // observeComponentCreation2 we do.
    this.updateFuncByElmtId.set(elmtId, { updateFunc: updateFunc });
    // add element id -> owning ViewPU
    UINodeRegisterProxy.ElementIdToOwningViewPU_.set(elmtId, new WeakRef(this));
    try {
      updateFunc(elmtId, /* is first render */ true);
    } catch (error) {
      // avoid the incompatible change that move set function before updateFunc.
      this.updateFuncByElmtId.delete(elmtId);
      UINodeRegisterProxy.ElementIdToOwningViewPU_.delete(elmtId);
      stateMgmtConsole.applicationError(`${this.debugInfo__()} has error in update func: ${(error as Error).message}`);
      throw error;
    }
  }

  public observeComponentCreation2(compilerAssignedUpdateFunc: UpdateFunc, classObject: UIClassObject): void {
    if (this.isDeleting_) {
      stateMgmtConsole.error(`View ${this.constructor.name} elmtId ${this.id__()} is already in process of destruction, will not execute observeComponentCreation2 `);
      return;
    }
    const _componentName: string = (classObject && ("name" in classObject)) ? Reflect.get(classObject, "name") as string : "unspecified UINode";
    const _popFunc: () => void = (classObject && "pop" in classObject) ? classObject.pop! : () => { };
    const updateFunc = (elmtId: number, isFirstRender: boolean) => {
      this.syncInstanceId();
      stateMgmtConsole.debug(`${this.debugInfo__()}: ${isFirstRender ? `First render` : `Re-render/update`} ${_componentName}[${elmtId}] ${!this.isViewV3 ? '(enable PU state observe) ' : ''} ${ConfigureStateMgmt.instance.needsV2Observe() ? '(enabled V2 state observe) ' : ''} - start ....`);

      ViewStackProcessor.StartGetAccessRecordingFor(elmtId);

      if (!this.isViewV3) {
        // Enable PU state tracking only in PU @Components
        this.currentlyRenderedElmtIdStack_.push(elmtId);
      }

      // if V2 @Observed/@Track used anywhere in the app (there is no more fine grained criteria), 
      // enable V2 object deep observation
      // FIXME: A @Component should only use PU or V2 state, but ReactNative dynamic viewer uses both.
      if (ConfigureStateMgmt.instance.needsV2Observe()) {
        // FIXME: like in V2 setting bindId_ in ObserveV3 does not work with 'stacked' 
        // update + initial render calls, like in if and ForEach case, convert to stack as well
        ObserveV3.getObserve().startBind(this, elmtId);
      }

      compilerAssignedUpdateFunc(elmtId, isFirstRender);
      if (!isFirstRender) {
        _popFunc();
      }

      let node = this.getNodeById(elmtId);
      if (node !== undefined) {
        (node as ArkComponent).cleanStageValue();
      }

      if (ConfigureStateMgmt.instance.needsV2Observe()) {
        ObserveV3.getObserve().startBind(null, UINodeRegisterProxy.notRecordingDependencies);
      }
      if (!this.isViewV3) {
        this.currentlyRenderedElmtIdStack_.pop();
      }
      ViewStackProcessor.StopGetAccessRecording();

      stateMgmtConsole.debug(`${this.debugInfo__()}: ${isFirstRender ? `First render` : `Re-render/update`}  ${_componentName}[${elmtId}] - DONE ....`);
      this.restoreInstanceId();
    };

    const elmtId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
    // needs to move set before updateFunc.
    // make sure the key and object value exist since it will add node in attributeModifier during updateFunc.
    this.updateFuncByElmtId.set(elmtId, { updateFunc: updateFunc, classObject: classObject });
    // add element id -> owning ViewPU
    UINodeRegisterProxy.ElementIdToOwningViewPU_.set(elmtId, new WeakRef(this));
    try {
      updateFunc(elmtId, /* is first render */ true);
    } catch (error) {
      // avoid the incompatible change that move set function before updateFunc.
      this.updateFuncByElmtId.delete(elmtId);
      UINodeRegisterProxy.ElementIdToOwningViewPU_.delete(elmtId);
      stateMgmtConsole.applicationError(`${this.debugInfo__()} has error in update func: ${(error as Error).message}`)
      throw error;
    }
    stateMgmtConsole.debug(`${this.debugInfo__()} is initial rendering elmtId ${elmtId}, tag: ${_componentName}, and updateFuncByElmtId size :${this.updateFuncByElmtId.size}`);
  }


  getOrCreateRecycleManager(): RecycleManager {
    if (!this.recycleManager_) {
      this.recycleManager_ = new RecycleManager
    }
    return this.recycleManager_;
  }

  getRecycleManager(): RecycleManager {
    return this.recycleManager_;
  }

  hasRecycleManager(): boolean {
    return !(this.recycleManager_ === undefined);
  }

  initRecycleManager(): void {
    if (this.recycleManager_) {
      stateMgmtConsole.error(`${this.debugInfo__()}: init recycleManager multiple times. Internal error.`);
      return;
    }
    this.recycleManager_ = new RecycleManager;
  }
  rebuildUpdateFunc(elmtId, compilerAssignedUpdateFunc) {
    const updateFunc = (elmtId, isFirstRender) => {
        this.currentlyRenderedElmtIdStack_.push(elmtId);
        compilerAssignedUpdateFunc(elmtId, isFirstRender);
        this.currentlyRenderedElmtIdStack_.pop();
    };
    if (this.updateFuncByElmtId.has(elmtId)) {
        this.updateFuncByElmtId.set(elmtId, { updateFunc: updateFunc });
    }
  }

  /**
   * @function observeRecycleComponentCreation
   * @description custom node recycle creation
   * @param name custom node name
   * @param recycleUpdateFunc custom node recycle update which can be converted to a normal update function
   * @return void
   */
  public observeRecycleComponentCreation(name: string, recycleUpdateFunc: RecycleUpdateFunc): void {
    // convert recycle update func to update func
    const compilerAssignedUpdateFunc: UpdateFunc = (element, isFirstRender) => {
      recycleUpdateFunc(element, isFirstRender, undefined)
    };
    let node: ViewPU;
    // if there is no suitable recycle node, run a normal creation function.
    if (!this.hasRecycleManager() || !(node = this.getRecycleManager().popRecycleNode(name))) {
      stateMgmtConsole.debug(`${this.constructor.name}[${this.id__()}]: cannot init node by recycle, crate new node`);
      this.observeComponentCreation(compilerAssignedUpdateFunc);
      return;
    }

    // if there is a suitable recycle node, run a recycle update function.
    const newElmtId: number = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
    const oldElmtId: number = node.id__();
    this.recycleManager_.updateNodeId(oldElmtId, newElmtId);
    this.hasBeenRecycled_ = true;
    this.rebuildUpdateFunc(oldElmtId, compilerAssignedUpdateFunc);
    recycleUpdateFunc(oldElmtId, /* is first render */ true, node);
  }

  aboutToReuseInternal() {
    this.runReuse_ = true;
    stateMgmtTrace.scopedTrace(() => {
      if (this.paramsGenerator_ && typeof this.paramsGenerator_ == "function") {
        const params = this.paramsGenerator_();
        this.updateStateVars(params);
        this.aboutToReuse(params);
      }
    }, "aboutToReuse", this.constructor.name);
    this.updateDirtyElements();
    this.childrenWeakrefMap_.forEach((weakRefChild) => {
      const child = weakRefChild.deref();
      if (child && !child.hasBeenRecycled_) {
        child.aboutToReuseInternal();
      }
    });
    this.runReuse_ = false;
  }

  aboutToRecycleInternal() {
    this.runReuse_ = true;
    stateMgmtTrace.scopedTrace(() => {
      this.aboutToRecycle();
    }, "aboutToRecycle", this.constructor.name);
    this.childrenWeakrefMap_.forEach((weakRefChild) => {
      const child = weakRefChild.deref();
      if (child && !child.hasBeenRecycled_) {
        child.aboutToRecycleInternal();
      }
    });
    this.runReuse_ = false;
  }

  // add current JS object to it's parent recycle manager
  public recycleSelf(name: string): void {
    if (this.parent_ && !this.parent_.isDeleting_) {
      this.parent_.getOrCreateRecycleManager().pushRecycleNode(name, this);
      this.hasBeenRecycled_ = true;
    } else {
      this.resetRecycleCustomNode();
      stateMgmtConsole.error(`${this.constructor.name}[${this.id__()}]: recycleNode must have a parent`);
    }
  }

  // performs the update on a branch within if() { branch } else if (..) { branch } else { branch }
  public ifElseBranchUpdateFunction(branchId: number, branchfunc: () => void): void {
    const oldBranchid: number = If.getBranchId();

    if (branchId == oldBranchid) {
      stateMgmtConsole.debug(`${this.debugInfo__()}: ifElseBranchUpdateFunction: IfElse branch unchanged, no work to do.`);
      return;
    }

    // branchid identifies uniquely the if .. <1> .. else if .<2>. else .<3>.branch
    // ifElseNode stores the most recent branch, so we can compare
    // removedChildElmtIds will be filled with the elmtIds of all children and their children will be deleted in response to if .. else change
    let removedChildElmtIds = new Array<number>();
    If.branchId(branchId, removedChildElmtIds);

    //unregisters the removed child elementIDs using proxy
    UINodeRegisterProxy.unregisterRemovedElmtsFromViewPUs(removedChildElmtIds);

    // purging these elmtIds from state mgmt will make sure no more update function on any deleted child wi;ll be executed
    stateMgmtConsole.debug(`ViewPU ifElseBranchUpdateFunction: elmtIds need unregister after if/else branch switch: ${JSON.stringify(removedChildElmtIds)}`);
    this.purgeDeletedElmtIds();

    branchfunc();
  }

  /**
   Partial updates for ForEach.
   * @param elmtId ID of element.
   * @param itemArray Array of items for use of itemGenFunc.
   * @param itemGenFunc Item generation function to generate new elements. If index parameter is
   *                    given set itemGenFuncUsesIndex to true.
   * @param idGenFunc   ID generation function to generate unique ID for each element. If index parameter is
   *                    given set idGenFuncUsesIndex to true.
   * @param itemGenFuncUsesIndex itemGenFunc optional index parameter is given or not.
   * @param idGenFuncUsesIndex idGenFunc optional index parameter is given or not.
   */
  public forEachUpdateFunction(elmtId: number,
    itemArray: Array<any>,
    itemGenFunc: (item: any, index?: number) => void,
    idGenFunc?: (item: any, index?: number) => string,
    itemGenFuncUsesIndex: boolean = false,
    idGenFuncUsesIndex: boolean = false): void {

    stateMgmtProfiler.begin("ViewPU.forEachUpdateFunction");
    stateMgmtConsole.debug(`${this.debugInfo__()}: forEachUpdateFunction (ForEach re-render) start ...`);

    if (itemArray === null || itemArray === undefined) {
      stateMgmtConsole.applicationError(`${this.debugInfo__()}: forEachUpdateFunction (ForEach re-render): input array is null or undefined error. Application error!`);
      stateMgmtProfiler.end();
      return;
    }

    if (typeof itemGenFunc !== "function") {
      stateMgmtConsole.applicationError(`${this.debugInfo__()}: forEachUpdateFunction (ForEach re-render): Item generation function missing. Application error!`);
      stateMgmtProfiler.end();
      return;
    }

    if (idGenFunc !== undefined && typeof idGenFunc !== "function") {
      stateMgmtConsole.applicationError(`${this.debugInfo__()}: forEachUpdateFunction (ForEach re-render): id generator is not a function. Application error!`);
      stateMgmtProfiler.end();
      return;
    }

    if (idGenFunc === undefined) {
      stateMgmtConsole.debug(`${this.debugInfo__()}: forEachUpdateFunction: providing default id gen function `);
      idGenFuncUsesIndex = true;
      // catch possible error caused by Stringify and re-throw an Error with a meaningful (!) error message
      idGenFunc = (item: any, index: number) => {
        try {
          return `${index}__${JSON.stringify(item)}`;
        } catch (e) {
          throw new Error(`${this.debugInfo__()}: ForEach id ${elmtId}: use of default id generator function not possible on provided data structure. Need to specify id generator function (ForEach 3rd parameter). Application Error!`)
        }
      }
    }

    let diffIndexArray = []; // New indexes compared to old one.
    let newIdArray = [];
    let idDuplicates = [];
    const arr = itemArray; // just to trigger a 'get' onto the array

    // ID gen is with index.
    if (idGenFuncUsesIndex || idGenFunc.length > 1) {
      // Create array of new ids.
      arr.forEach((item, indx) => {
        newIdArray.push(idGenFunc(item, indx));
      });
    }
    else {
      // Create array of new ids.
      arr.forEach((item, index) => {
        newIdArray.push(`${itemGenFuncUsesIndex ? index + '_' : ''}` + idGenFunc(item));
      });
    }

    // Set new array on C++ side.
    // C++ returns array of indexes of newly added array items.
    // these are indexes in new child list.
    ForEach.setIdArray(elmtId, newIdArray, diffIndexArray, idDuplicates);

    // Its error if there are duplicate IDs.
    if (idDuplicates.length > 0) {
      idDuplicates.forEach((indx) => {
        stateMgmtConsole.error(`Error: ${newIdArray[indx]} generated for ${indx}${indx < 4 ? indx == 2 ? "nd" : "rd" : "th"} array item ${arr[indx]}.`);
      });
      stateMgmtConsole.applicationError(`${this.debugInfo__()}: Ids generated by the ForEach id gen function must be unique. Application error!`);
    }

    aceTrace.begin(`forEach update id: ${elmtId}, idDuplicates.length: ${idDuplicates.length}, diffIndexArray.length: ${diffIndexArray.length}`);

    stateMgmtConsole.debug(`${this.debugInfo__()}: forEachUpdateFunction: diff indexes ${JSON.stringify(diffIndexArray)} . `);

    // Item gen is with index.
    stateMgmtConsole.debug(`   ... item Gen ${itemGenFuncUsesIndex ? 'with' : "without"} index`);
    // Create new elements if any.
    stateMgmtProfiler.begin("ViewPU.forEachUpdateFunction (native)");
    diffIndexArray.forEach((indx) => {
      ForEach.createNewChildStart(newIdArray[indx], this);
      if (itemGenFuncUsesIndex) {
        itemGenFunc(arr[indx], indx);
      } else {
        itemGenFunc(arr[indx]);
      }
      ForEach.createNewChildFinish(newIdArray[indx], this);
    });
    stateMgmtConsole.debug(`${this.debugInfo__()}: forEachUpdateFunction (ForEach re-render) - DONE.`);
    stateMgmtProfiler.end();
    stateMgmtProfiler.end();
    aceTrace.end();
  }

  public UpdateLazyForEachElements(elmtIds: Array<number>): void {
    if (!Array.isArray(elmtIds)) {
      return;
    }
    Array.from(elmtIds).sort(ViewPU.compareNumber).forEach((elmtId: number) => {
      const entry: UpdateFuncRecord | undefined = this.updateFuncByElmtId.get(elmtId);
      const updateFunc: UpdateFunc = entry ? entry.getUpdateFunc() : undefined;
      if (typeof updateFunc !== "function") {
        stateMgmtConsole.debug(`${this.debugInfo__()}: update function of elmtId ${elmtId} not found, internal error!`);
      } else {
        this.isRenderInProgress = true;
        updateFunc(elmtId, false);
        this.finishUpdateFunc(elmtId);
        this.isRenderInProgress = false;
      }
    })
  }

  /**
     * CreateStorageLink and CreateStorageLinkPU are used by the implementation of @StorageLink and
     * @LocalStotrageLink in full update and partial update solution respectively.
     * These are not part of the public AppStorage API , apps should not use.
     * @param storagePropName - key in LocalStorage
     * @param defaultValue - value to use when creating a new prop in the LocalStotage
     * @param owningView - the View/ViewPU owning the @StorageLink/@LocalStorageLink variable
     * @param viewVariableName -  @StorageLink/@LocalStorageLink variable name
     * @returns SynchedPropertySimple/ObjectTwoWay/PU
     */
  public createStorageLink<T>(storagePropName: string, defaultValue: T, viewVariableName: string): ObservedPropertyAbstractPU<T> {
    const appStorageLink = AppStorage.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyTwoWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    appStorageLink.setDecoratorInfo("@StorageLink");
    return appStorageLink;
  }

  public createStorageProp<T>(storagePropName: string, defaultValue: T, viewVariableName: string): ObservedPropertyAbstractPU<T> {
    const appStorageProp = AppStorage.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyOneWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    appStorageProp.setDecoratorInfo("@StorageProp");
    return appStorageProp;
  }

  public createLocalStorageLink<T>(storagePropName: string, defaultValue: T,
    viewVariableName: string): ObservedPropertyAbstractPU<T> {
    const localStorageLink = this.localStorage_.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyTwoWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    localStorageLink.setDecoratorInfo("@LocalStorageLink");
    return localStorageLink;
  }

  public createLocalStorageProp<T>(storagePropName: string, defaultValue: T,
    viewVariableName: string): ObservedPropertyAbstractPU<T> {
    const localStorageProp = this.localStorage_.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : new SynchedPropertyObjectOneWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
    localStorageProp.setDecoratorInfo("@LocalStorageProp");
    return localStorageProp;
  }

  public createOrGetNode(elmtId: number, builder: () => ArkComponent): object {
    const entry = this.updateFuncByElmtId.get(elmtId);
    if (entry === undefined) {
      throw new Error(`${this.debugInfo__()} fail to create node, elmtId is illegal`);
    }
    let nodeInfo = entry.getNode();
    if (nodeInfo === undefined) {
      nodeInfo = builder();
      entry.setNode(nodeInfo);
    }
    return nodeInfo;
  }

  /**
   * getNodeById is used to get ArkComponent stored updateFuncByElmtId
   * @param elmtId -  the id of the component
   * @returns ArkComponent | undefined
   */
  public getNodeById(elmtId: number): ArkComponent | undefined {
    const entry = this.updateFuncByElmtId.get(elmtId);
    return entry ? entry.getNode() : undefined;
  }

  /**
   * onDumpInfo is used to process commands delivered by the hidumper process
   * @param commands -  list of commands provided in the shell
   * @returns void
   */
  protected onDumpInfo(commands: string[]): void {

    let dfxCommands: DFXCommand[] = this.processOnDumpCommands(commands);

    dfxCommands.forEach((command) => {
      let view: ViewPU = undefined;
      if (command.viewId) {
        view = this.findViewInHierarchy(command.viewId);
        if (!view) {
          DumpLog.print(0, `\nTarget view: ${command.viewId} not found for command: ${command.what}\n`);
          return;
        }
      } else {
        view = this;
        command.viewId = view.id__();
      }
      switch (command.what) {
        case "-dumpAll":
          view.printDFXHeader("ViewPU Info", command);
          DumpLog.print(0, view.debugInfoView(command.isRecursive));
          break;
        case "-viewHierarchy":
          view.printDFXHeader("ViewPU Hierarchy", command);
          DumpLog.print(0, view.debugInfoViewHierarchy(command.isRecursive));
          break;
        case "-stateVariables":
          view.printDFXHeader("ViewPU State Variables", command);
          DumpLog.print(0, view.debugInfoStateVars());
          break;
        case "-registeredElementIds":
          view.printDFXHeader("ViewPU Registered Element IDs", command);
          DumpLog.print(0, view.debugInfoUpdateFuncByElmtId(command.isRecursive));
          break;
        case "-dirtyElementIds":
          view.printDFXHeader("ViewPU Dirty Registered Element IDs", command);
          DumpLog.print(0, view.debugInfoDirtDescendantElementIds(command.isRecursive));
          break;
        case "-inactiveComponents":
          view.printDFXHeader("List of Inactive Components", command);
          DumpLog.print(0, view.debugInfoInactiveComponents());
          break;
        case "-profiler":
          view.printDFXHeader("Profiler Info", command);
          view.dumpReport();
          break;
        default:
          DumpLog.print(0, `\nUnsupported JS DFX dump command: [${command.what}, viewId=${command.viewId}, isRecursive=${command.isRecursive}]\n`);
      }
    })
  }

  private printDFXHeader(header: string, command: DFXCommand): void {
    let length: number = 50;
    let remainder: number = length - header.length < 0 ? 0 : length - header.length;
    DumpLog.print(0, `\n${'-'.repeat(remainder / 2)}${header}${'-'.repeat(remainder / 2)}`);
    DumpLog.print(0, `[${command.what}, viewId=${command.viewId}, isRecursive=${command.isRecursive}]\n`);
  }

  private processOnDumpCommands(commands: string[]): DFXCommand[] {
    let isFlag: Function = (param: string): boolean => {
      return "-r".match(param) != null || param.startsWith("-viewId=");
    }

    let dfxCommands: DFXCommand[] = [];

    for (var i: number = 0; i < commands.length; i++) {
      let command = commands[i];
      if (isFlag(command)) {
        if (command.startsWith("-viewId=")) {
          let dfxCommand: DFXCommand = dfxCommands[dfxCommands.length - 1];
          if (dfxCommand) {
            let input: string[] = command.split('=');
            if (input[1]) {
              let viewId: number = Number.parseInt(input[1]);
              dfxCommand.viewId = Number.isNaN(viewId) ? UINodeRegisterProxy.notRecordingDependencies : viewId;
            }
          }
        } else if (command.match("-r")) {
          let dfxCommand: DFXCommand = dfxCommands[dfxCommands.length - 1];
          if (dfxCommand) {
            dfxCommand.isRecursive = true;
          }
        }
      } else {
        dfxCommands.push({
          what: command,
          viewId: undefined,
          isRecursive: false,
        })
      }
    }
    return dfxCommands;
  }

  private findViewInHierarchy(id: number): ViewPU {
    let weak = this.childrenWeakrefMap_.get(id);
    if (weak) {
      return weak.deref();
    }

    let retVal: ViewPU = undefined;
    for (const [key, value] of this.childrenWeakrefMap_.entries()) {
      retVal = value.deref().findViewInHierarchy(id);
      if (retVal)
        break;
    }
    return retVal;
  }

  private debugInfoView(recursive: boolean = false): string {
    return this.debugInfoViewInternal(recursive);
  }

  private debugInfoViewInternal(recursive: boolean = false): string {
    let retVal: string = `@Component\n${this.constructor.name}[${this.id__()}]`;
    retVal += `\n\nView Hierarchy:\n${this.debugInfoViewHierarchy(recursive)}`;
    retVal += `\n\nState variables:\n${this.debugInfoStateVars()}`;
    retVal += `\n\nRegistered Element IDs:\n${this.debugInfoUpdateFuncByElmtId(recursive)}`;
    retVal += `\n\nDirty Registered Element IDs:\n${this.debugInfoDirtDescendantElementIds(recursive)}`;
    return retVal;
  }

  private debugInfoViewHierarchy(recursive: boolean = false): string {
    return this.debugInfoViewHierarchyInternal(0, recursive);
  }

  private debugInfoViewHierarchyInternal(depth: number = 0, recursive: boolean = false): string {
    let retVaL: string = `\n${"  ".repeat(depth)}|--${this.constructor.name}[${this.id__()}]`;
    if (this.isCompFreezeAllowed) {
      retVaL += ` {freezewhenInactive : ${this.isCompFreezeAllowed}}`;
    }

    if (depth < 1 || recursive) {
      this.childrenWeakrefMap_.forEach((value, key, map) => {
        retVaL += value.deref()?.debugInfoViewHierarchyInternal(depth + 1, recursive);
      })
    }
    return retVaL;
  }

  private debugInfoUpdateFuncByElmtId(recursive: boolean = false): string {
    return this.debugInfoUpdateFuncByElmtIdInternal({ total: 0 }, 0, recursive);
  }

  private debugInfoUpdateFuncByElmtIdInternal(counter: ProfileRecursionCounter, depth: number = 0, recursive: boolean = false): string {
    let retVaL: string = `\n${"  ".repeat(depth)}|--${this.constructor.name}[${this.id__()}]: {`;
    this.updateFuncByElmtId.forEach((value, key, map) => {
      let componentName: string = value.getComponentName();
      if (componentName === "unspecified UINode") {
        componentName = this.getChildById(key)?.constructor.name;
      }
      retVaL += `\n${"  ".repeat(depth + 2)}${componentName}[${key}]`
    })
    counter.total += this.updateFuncByElmtId.size;
    retVaL += `\n${"  ".repeat(depth + 1)}}[${this.updateFuncByElmtId.size}]`
    if (recursive) {
      this.childrenWeakrefMap_.forEach((value, key, map) => {
        retVaL += value.deref()?.debugInfoUpdateFuncByElmtIdInternal(counter, depth + 1, recursive);
      })
    }
    if (recursive && depth == 0) {
      retVaL += `\nTotal: ${counter.total}`
    }
    return retVaL;
  }

  private debugInfoDirtDescendantElementIds(recursive: boolean = false): string {
    return this.debugInfoDirtDescendantElementIdsInternal(0, recursive, { total: 0 });
  }

  private debugInfoDirtDescendantElementIdsInternal(depth: number = 0, recursive: boolean = false, counter: ProfileRecursionCounter): string {
    let retVaL: string = `\n${"  ".repeat(depth)}|--${this.constructor.name}[${this.id__()}]: {`;
    this.dirtDescendantElementIds_.forEach((value) => {
      retVaL += `${value}, `
    })
    counter.total += this.dirtDescendantElementIds_.size;
    retVaL += `\n${"  ".repeat(depth + 1)}}[${this.dirtDescendantElementIds_.size}]`
    if (recursive) {
      this.childrenWeakrefMap_.forEach((value, key, map) => {
        retVaL += value.deref()?.debugInfoDirtDescendantElementIdsInternal(depth + 1, recursive, counter);
      })
    }

    if (recursive && depth == 0) {
      retVaL += `\nTotal: ${counter.total}`
    }
    return retVaL;
  }

  private debugInfoInactiveComponents(): string {
    return Array.from(ViewPU.inactiveComponents_)
      .map((component) => `- ${component}`).join('\n');
  }

  /**
   * 
   * @param paramVariableName @param is read only, therefore, update form parent needs to be done without
   *        causing property setter() to be called
   * @param newValue 
   */
  protected updateParam<Z>(paramVariableName : string, newValue : Z) {
    ObserveV3.getObserve().setReadOnlyAttr<Z>(this, paramVariableName, newValue);
  }

  /**
   * sub-class must call this function at the end of its constructor
   * especially after init variables from parent ViewPU has been done
   */
  protected finalizeConstruction(): void {
    // FIXME, can we skip for apps that do not use V3 at all?
    ObserveV3.getObserve().constructMonitor(this, this.constructor.name);
    ObserveV3.getObserve().constructComputed(this, this.constructor.name);

    // FIME ProvideConsumeUtilV3.setupConsumeVarsV3(this);

    // Always use ID_REFS in ViewPU
    this[ObserveV3.ID_REFS] = {};
  }

  /**
   * v3: find a @provide'ed variable in the nearest ancestor ViewPU.
   * @param provideName 
   * @returns 
   */
  public findProvideV3(provideName: string): [ViewPU | undefined, string, boolean] {
    // FIXME unimplemented
    return [ undefined, provideName, true]
  }

  // WatchIds that needs to be fired later gets added to monitorIdsDelayedUpdate
  // monitor firechange will be triggered for all these watchIds once this view gets active
  public addDelayedMonitorIds(watchId: number) {
    /* FIXME @Component freeze 
    stateMgmtConsole.debug(`${this.debugInfo__()} addDelayedMonitorIds called for watchId: ${watchId}`);
    this.monitorIdsDelayedUpdate.add(watchId);
    */
  }

  public addDelayedComputedIds(watchId: number) {
    /* FIXME @Component freeze 
    stateMgmtConsole.debug(`${this.debugInfo__()} addDelayedComputedIds called for watchId: ${watchId}`);
    this.computedIdsDelayedUpdate.add(watchId);
  */
  }

  /**
   * on first render create a new Instance of Repeat
   * on re-render connect to existing instance 
   * @param arr 
   * @returns 
   */
  public __mkRepeatAPI: <I>(arr: Array<I>) => RepeatAPI<I>
  = <I>(arr: Array<I>): RepeatAPI<I> => {
      // factory is for future extensions, currently always return the same
      const elmtId = this.getCurrentlyRenderedElmtId();
      let repeat = this.elmtId2Repeat_.get(elmtId) as __Repeat<I>
      if (!repeat) {
          repeat = new __Repeat<I>(this, arr);
          this.elmtId2Repeat_.set(elmtId, repeat);
      } else {
          repeat.updateArr(arr)
      }

      return repeat;
  }
}  // class ViewPU

class UpdateFuncsByElmtId {

  private map_ = new Map<number, UpdateFuncRecord>();

  public delete(elmtId: number): boolean {
    return this.map_.delete(elmtId);
  }

  public set(elmtId: number, params: UpdateFunc | { updateFunc: UpdateFunc, classObject?: UIClassObject, node?: ArkComponent }): void {
    (typeof params === 'object') ?
      this.map_.set(elmtId, new UpdateFuncRecord(params)) :
      this.map_.set(elmtId, new UpdateFuncRecord({ updateFunc: params as UpdateFunc }));
  }

  public get(elmtId: number): UpdateFuncRecord | undefined {
    return this.map_.get(elmtId);
  }

  public has(elmtId: number): boolean {
    return this.map_.has(elmtId);
  }

  public keys(): IterableIterator<number> {
    return this.map_.keys();
  }

  public clear(): void {
    return this.map_.clear();
  }

  public get size(): number {
    return this.map_.size;
  }

  public forEach(callbackfn: (value: UpdateFuncRecord, key: number, map: Map<number, UpdateFuncRecord>) => void): void {
    this.map_.forEach(callbackfn);
  }

  // dump info about known elmtIds to a string
  // use function only for debug output and DFX.
  public debugInfoRegisteredElmtIds(): string {
    let result: string = "";
    let sepa: string = "";
    this.map_.forEach((value: UpdateFuncRecord, elmtId: number) => {
      result += `${sepa}${value.getComponentName()}[${elmtId}]`;
      sepa = ", ";
    });
    return result;
  }

  public debugInfoElmtId(elmtId: number): string {
    const updateFuncEntry = this.map_.get(elmtId);
    return updateFuncEntry ? `'${updateFuncEntry!.getComponentName()}[${elmtId}]'` : `'unknown component type'[${elmtId}]`;
  }
}  // class UpdateFuncByElmtId
