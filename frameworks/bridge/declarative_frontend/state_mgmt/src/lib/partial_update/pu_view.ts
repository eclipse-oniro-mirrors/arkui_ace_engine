/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
 */

type ProvidedVarsMapPU = Map<string, ObservedPropertyAbstractPU<any>>;

// denotes a missing elemntId, this is the case during initial render
const UndefinedElmtId = -1;

// function type of partial update function
type UpdateFunc = (elmtId: number, isFirstRender: boolean) => void;

// Nativeview
// implemented in C++  for release
// and in utest/view_native_mock.ts for testing
abstract class ViewPU extends NativeViewPartialUpdate
  implements IViewPropertiesChangeSubscriber {

  private id_: number;

  private watchedProps: Map<string, (propName: string) => void>
    = new Map<string, (propName: string) => void>();

  // @Provide'd variables by this class and its ancestors
  protected providedVars_: ProvidedVarsMapPU;

  // Set of dependent elmtIds that need partial update
  // during next re-render
  protected dirtDescendantElementIds_: Set<number>
    = new Set<number>();

  // registry of update functions
  // the key is the elementId of the Component/Element that's the result of this function
  protected updateFuncByElmtId: Map<number, UpdateFunc>
    = new Map<number, UpdateFunc>();

  // my LocalStorge instance, shared with ancestor Views.
  // create a default instance on demand if none is initialized
  protected localStoragebackStore_: LocalStorage = undefined;

  protected get localStorage_() {
    if (!this.localStoragebackStore_) {
      this.localStoragebackStore_ = new LocalStorage({ /* emty */ });
    }
    return this.localStoragebackStore_;
  }

  protected set localStorage_(instance: LocalStorage) {
    if (!instance) {
      // setting to undefined not allowed
      return;
    }
    if (this.localStoragebackStore_) {
      console.error(`${this.constructor.name} is setting LocalStorage instance twice`);
    }
    this.localStoragebackStore_ = instance;
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
  constructor(parent: ViewPU, localStorage?: LocalStorage) {
    super();
    this.id_ = SubscriberManager.Get().MakeId();
    this.providedVars_ = parent ? new Map(parent.providedVars_)
      : new Map<string, ObservedPropertyAbstractPU<any>>();

    this.localStoragebackStore_ = undefined;
    if (parent) {
      // this View is not a top-level View
      console.debug(`${this.constructor.name} constructor: Using LocalStorage instance of the parent View.`);
      this.localStorage_ = parent.localStorage_;
    } else if (localStorage) {
      this.localStorage_ = localStorage;
      console.debug(`${this.constructor.name} constructor: Using LocalStorage instance provided via @Entry.`);
    }

    SubscriberManager.Get().add(this);
    console.debug(`${this.constructor.name}(${this.id__()}): constructor done`);
  }

  // globally unique id, this is different from compilerAssignedUniqueChildId!
  id__(): number {
    return this.id_;
  }

  // inform the subscribed property
  // that the View and thereby all properties
  // are about to be deleted
  abstract aboutToBeDeleted(): void;

  // super class will call this function from 
  // its aboutToBeDeleted implementation
  protected aboutToBeDeletedInternal(): void {
    this.updateFuncByElmtId.clear();
    this.watchedProps.clear();
    this.providedVars_.clear();
  }

  protected abstract purgeVariableDependenciesOnElmtId(removedElmtId: number);
  protected abstract initialRender(): void;
  protected abstract rerender(): void;

  protected initialRenderView(): void {
    this.initialRender();
  }

  // implements IMultiPropertiesChangeSubscriber
  viewPropertyHasChanged(varName: PropertyInfo, dependentElmtIds: Set<number>): void {
    console.debug(`${this.constructor.name}: viewPropertyHasChanged property '${varName}'. View needs ${dependentElmtIds.size ? 'update' : 'no update'}.`);
    this.syncInstanceId();

    let cb = this.watchedProps.get(varName)
    if (cb) {
      console.debug(`   .. calling @Watch function`);
      cb.call(this, varName);
    }

    if (dependentElmtIds.size) {
      if (!this.dirtDescendantElementIds_.size) {
        // mark Composedelement dirty when first elmtIds are added
        // do not need to do this every time
        this.markNeedUpdate();
      }
      console.debug(`${this.constructor.name}: viewPropertyHasChanged property '${varName}': elmtIds affected by value change [${Array.from(dependentElmtIds).toString()}].`)
      const union: Set<number> = new Set<number>([...this.dirtDescendantElementIds_, ...dependentElmtIds]);
      this.dirtDescendantElementIds_ = union;
      console.debug(`${this.constructor.name}: viewPropertyHasChanged property '${varName}': all elmtIds need update [${Array.from(this.dirtDescendantElementIds_).toString()}].`)
    }
    this.restoreInstanceId();
  }

  /**
   * Function to be called from the constructor of the sub component
   * to register a @Watch varibale
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
  protected addProvidedVar<T>(providedPropName: string, store: ObservedPropertyAbstractPU<T>) {
    if (this.providedVars_.has(providedPropName)) {
      throw new ReferenceError(`${this.constructor.name}: duplicate @Provide property with name ${providedPropName}.
      Property with this name is provided by one of the ancestor Views already.`);
    }
    this.providedVars_.set(providedPropName, store);
  }

  /**
   * Method for the sub-class to call from its constructor for resolving
   *       a @Consume variable and initializing its backing store
   *       with the yncedPropertyTwoWay<T> object created from the
   *       @Provide variable's backing store.
   * @param providedPropName the name of the @Provide'd variable.
   *     This is either the @Consume decortor parameter, or variable name.
   * @param consumeVarName the @Consume variable name (not the
   *            @Consume decortor parameter)
   * @returns initiaizing value of the @Consume backing store
   */
  protected initializeConsume<T>(providedPropName: string,
    consumeVarName: string): ObservedPropertyAbstractPU<T> {
    let providedVarStore = this.providedVars_.get(providedPropName);
    if (providedVarStore === undefined) {
      throw new ReferenceError(`${this.constructor.name}: missing @Provide property with name ${providedPropName}.
     Fail to resolve @Consume(${providedPropName}).`);
    }

    return providedVarStore.createSync(
      <T>(source: ObservedPropertyAbstract<T>) => (source instanceof ObservedPropertySimple)
        ? new SynchedPropertySimpleTwoWayPU<T>(source, this, consumeVarName)
        : new SynchedPropertyObjectTwoWayPU<T>(source, this, consumeVarName)) as ObservedPropertyAbstractPU<T>;
  }


  /**
   * given the elmtid of a child or child of child within this custom component
   * remember this component needs a partial update
   * @param elmtId
   */
  public markElemenDirtyById(elmtId: number): void {
    // TODO ace-ets2bundle, framework, compilated apps need to update together
    // this function will be removed after a short transiition periode
    console.error(`markElemenDirtyById no longer supported. 
        Please update your ace-ets2bundle and recompile your application!`);
  }

  /**
   * For each recorded dirty Element in this custom component
   * run its update function
   *
   */
  public updateDirtyElements() {
    if (this.dirtDescendantElementIds_.size == 0) {
      console.debug(`No dirty elements for ${this.constructor.name}`);
      return;
    }

    // Array.sort() converts array items to string to compare them, sigh!
    var compareNumber = (a: number, b: number): number => {
      return (a < b) ? -1 : (a > b) ? 1 : 0;
    };

    console.debug(`View ${this.constructor.name} elmtId ${this.id__()}:  updateDirtyElements: sorted dirty elmtIds: ${JSON.stringify(Array.from(this.dirtDescendantElementIds_).sort(compareNumber))}, starting ....`);

    // request list of all (gloabbly) deleteelmtIds;
    let deletedElmtIds: number[] = [];
    this.getDeletedElemtIds(deletedElmtIds);

    // see which elmtIds are managed by this View
    // and clean up all book keeping for them
    this.purgeDeletedElmtIds(deletedElmtIds);

    // process all elmtIds marked as needing update in ascending order.
    // ascending order ensures parent nodes will be updated before their children
    // prior cleanup ensure no already deleted Elements have their update func executed
    Array.from(this.dirtDescendantElementIds_).sort(compareNumber).forEach(elmtId => {
      // do not process an Element that has been marked to be deleted
      const updateFunc: UpdateFunc = this.updateFuncByElmtId.get(elmtId);
      if (updateFunc == undefined) {
        console.error(`${this.constructor.name}[${this.id__()}]: update function of ElementId ${elmtId} not found, internal error!`);
      } else {
        console.debug(`${this.constructor.name}[${this.id__()}]: updateDirtyElements: update function on elmtId ${elmtId} start ...`);
        updateFunc(elmtId, /* isFirstRender */ false);
        // continue in native JSView
        // Finish the Update in JSView::JsFinishUpdateFunc
        // this function appends no longer used elmtIds (as recrded by VSP) to the given allRmElmtIds array
        this.finishUpdateFunc(elmtId);
        console.debug(`View ${this.constructor.name} elmtId ${this.id__()}: ViewPU.updateDirtyElements: update function on ElementId ${elmtId} done`);
      }
    });
    this.dirtDescendantElementIds_.clear();
  }

  //  given a list elementIds removes these from state variables dependency list and from elmtId -> updateFunc map
  purgeDeletedElmtIds(rmElmtIds: number[]) {
    if (rmElmtIds.length == 0) {
      return;
    }

    console.debug(`View ${this.constructor.name} elmtId ${this.id__()}.purgeDeletedElmtIds -  start.`);

    // rmElmtIds is the array of ElemntIds that 
    let removedElmtIds: number[] = [];
    rmElmtIds.forEach((elmtId: number) => {
      // remove entry from Map elmtId -> update function
      if (this.updateFuncByElmtId.delete(elmtId)) {

        // for each state var, remove dependent elmtId (if present)
        // purgeVariableDependenciesOnElmtId needs to be generated by the compiler
        this.purgeVariableDependenciesOnElmtId(elmtId);

        // keep track of elmtId that has been de-registered
        removedElmtIds.push(elmtId);
      }
    });

    this.deletedElmtIdsHaveBeenPurged(removedElmtIds);
    console.debug(`View ${this.constructor.name} elmtId ${this.id__()}.purgeDeletedElmtIds: removed elemntIds  ${JSON.stringify(removedElmtIds)}.`);
    console.debug(`   ... remaining update funcs for elmtIds ${JSON.stringify([... this.updateFuncByElmtId.keys()])} .`);
  }

  // the current executed update function
  public observeComponentCreation(compilerAssignedUpdateFunc: UpdateFunc): void {
    const elmtId = ViewStackProcessor.AllocateNewElmetIdForNextComponent();
    console.debug(`${this.constructor.name}[${this.id__()}]: First render for elmtId ${elmtId} start ....`);
    compilerAssignedUpdateFunc(elmtId, /* is first rneder */ true);

    this.updateFuncByElmtId.set(elmtId, compilerAssignedUpdateFunc);
    console.debug(`${this.constructor.name}[${this.id__()}]: First render for elmtId ${elmtId} - DONE.`);
  }

  /*
    partual updates for ForEach
    The processing steps in re-render case are
    1. create ForEachComponent,
    2. get the previousIdArray from the ForEachElement. This array of
       trings stores the result of the id generation function for all
       array items at previous render,
    3. compute newIdArray in the same way, using the current array. By comparing
       the two array it gets clear what array items a- still exist but may have
       moved position, b- which ones are new, and c- which ones no longer exist.
    4. the ForEachComponent to sync the new idArray to the ForEachElement when it updates.
    5. create ForEach children for new item array (case 2 above).

    In the initial render case oldIdArray is empty. Therefore a new child will be
    created for each item in the array (see step 3).
  */
  forEachUpdateFunction(elmtId: number,
    _arr: Array<any>,
    itemGenFunc: (item: any) => void,
    idGenFunc?: (item: any) => string) {
    console.debug(`${this.constructor.name}[${this.id__()}]: forEachUpdateFunction elmtId ${elmtId} - start`);

    if (idGenFunc === undefined) {
      console.debug(`${this.constructor.name}[${this.id__()}]: forEachUpdateFunction: forEach does not specify id generation function. This will degrade performance`);
      idGenFunc = function makeIdGenFunction() {
        let index = 0;
        // developers should always add an id gen function, because framework can not invent a good one
        // for most cases:
        // generate id that consist of the running index and the strinified item
        return (item) => `${index++}_${JSON.stringify(item)}`;
      }();
    }
    const arr = _arr;  // just to trigger a 'get' onto the array

    // id array of previous render or empty
    const prevIdArray: string[] = [];
    const success: boolean = ForEach.getIdArray(elmtId, prevIdArray) || false;  // step 2

    console.debug(`${JSON.stringify(prevIdArray)}`);

    // create array of new ids
    let newIdArray: string[] = [];
    arr.forEach((item) => {
      let value = idGenFunc(item);
      console.debug(`ID generator for ${item} returned: ${value}`)
      newIdArray.push(value);
    });

    // step 3
    // replace the idArray
    // ForEachComponent needs to sync this back to the ForEachElement
    ForEach.setIdArray(elmtId, newIdArray);

    // step 4
    // now, we compare the old and the new array
    // for each entry three cases
    // 1. id is in newIdArray, but missing from oldIdArray -> new entry, execute itemGenFunction
    // 2. id is not in newIdArray, but in oldIdArray -> do nothing,
    //       ForEachComponent to ForEachElement update will take care of deletion
    // 3. id is in newdIdEntry and in oldIdEntry -> do nothing
    //         ForEachComponent to ForEachElement update will change the slot
    //         based on info found in the new idArray
    newIdArray.forEach((id, newIndex) => {     // step 5
      console.debug(`Handle item with item-id ${id}:`);
      if (prevIdArray.indexOf(id) < 0) {
        // id not in oldIdArray: create new child

        console.debug(`   ... render children for item with item-id ${id} - start !`);
        // on native side:
        //  viewStack->PushKey(id);
        //  viewStack->Push(AceType::MakeRefPtr<MultiComposedComponent>(viewStack->GetKey(), "ForEachItem"));
        ForEach.createNewChildStart(id, /* View/JSView */ this);

        itemGenFunc(arr[newIndex]);

        // on native side:
        //   viewStack->PopContainer();  FIXME: put to the right position in children_
        //   viewStack->PopKey();
        //   sync Component to Element
        ForEach.createNewChildFinish(id, /* View/JSView */ this);

        console.debug(`   ... render children for item with item-id ${id} - done!`);
      } else {
        console.debug(`   ... item-id ${id} is no new item, will not render child - done!`);
      }
    }); // newIdArray.forEach

    console.debug(`${this.constructor.name}[${this.id__()}]: forEachUpdateFunction elmtId ${elmtId} - done.`);
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
    return AppStorage.__CreateSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : (source instanceof ObservedPropertySimple)
          ? new SynchedPropertySimpleTwoWayPU<T>(source, this, viewVariableName)
          : new SynchedPropertyObjectTwoWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
  }


  public createStorageProp<T>(storagePropName: string, defaultValue: T,
    viewVariableName: string): SynchedPropertySimpleOneWayPU<T> {
    return AppStorage.__CreateSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source instanceof ObservedPropertySimple)
        ? new SynchedPropertySimpleOneWayPU<T>(source, this, viewVariableName)
        : undefined
    ) as SynchedPropertySimpleOneWayPU<T>;
  }

  public createLocalStorageLink<T>(storagePropName: string, defaultValue: T,
    viewVariableName: string): ObservedPropertyAbstractPU<T> {
    return this.localStorage_.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source === undefined)
        ? undefined
        : (source instanceof ObservedPropertySimple)
          ? new SynchedPropertySimpleTwoWayPU<T>(source, this, viewVariableName)
          : new SynchedPropertyObjectTwoWayPU<T>(source, this, viewVariableName)
    ) as ObservedPropertyAbstractPU<T>;
  }


  public createLocalStorageProp<T>(storagePropName: string, defaultValue: T,
    viewVariableName: string): SynchedPropertySimpleOneWayPU<T> {
    return this.localStorage_.__createSync<T>(storagePropName, defaultValue,
      <T>(source: ObservedPropertyAbstract<T>) => (source instanceof ObservedPropertySimple)
        ? new SynchedPropertySimpleOneWayPU<T>(source, this, viewVariableName)
        : undefined
    ) as SynchedPropertySimpleOneWayPU<T>;
  }
}
