/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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


/**
 *
 * This file includes only framework internal classes and functions
 * non are part of SDK. Do not access from app.
 *
 *
 * ObserveV2 is the singleton object for observing state variable access and
 * change
 */

// in the case of ForEach, Repeat, AND If, two or more UINodes / elmtIds can render at the same time
// e.g. ForEach -> ForEach child Text, Repeat -> Nested Repeat, child Text
// Therefore, ObserveV2 needs to keep a strack of currently renderign ids / components
// in the same way as thsi is also done for PU stateMgmt with ViewPU.currentlyRenderedElmtIdStack_
class StackOfRenderedComponents {
  private stack_: Array<StackOfRenderedComponentsItem> = new Array<StackOfRenderedComponentsItem>();

  public push(id: number, cmp: IView | MonitorV2 | ComputedV2 | PersistenceV2Impl): void {
    this.stack_.push(new StackOfRenderedComponentsItem(id, cmp));
  }

  public pop(): [id: number, cmp: IView | MonitorV2 | ComputedV2 | PersistenceV2Impl] | undefined {
    const item = this.stack_.pop();
    return item ? [item.id_, item.cmp_] : undefined;
  }

  public top(): [id: number, cmp: IView | MonitorV2 | ComputedV2 | PersistenceV2Impl] | undefined {
    if (this.stack_.length) {
      const item = this.stack_[this.stack_.length - 1];
      return [item.id_, item.cmp_];
    } else {
      return undefined;
    }
  }
}

class StackOfRenderedComponentsItem {
  public id_ : number;
  public cmp_ : IView | MonitorV2 | ComputedV2 | PersistenceV2Impl;

  constructor(id : number, cmp : IView | MonitorV2 | ComputedV2 | PersistenceV2Impl) {
    this.id_ = id;
    this.cmp_ = cmp;
  }
}

class ObserveV2 {
  // meta data about decorated variable inside prototype
  public static readonly V2_DECO_META = Symbol('__v2_deco_meta__');

  public static readonly SYMBOL_REFS = Symbol('__use_refs__');
  public static readonly ID_REFS = Symbol('__id_refs__');
  public static readonly MONITOR_REFS = Symbol('___monitor_refs_');
  public static readonly COMPUTED_REFS = Symbol('___computed_refs_');

  public static readonly SYMBOL_PROXY_GET_TARGET = Symbol('__proxy_get_target');

  public static readonly SYMBOL_MAKE_OBSERVED = Symbol('___make_observed__');

  public static readonly OB_PREFIX = '__ob_'; // OB_PREFIX + attrName => backing store attribute name
  public static readonly OB_PREFIX_LEN = 5;

  // used by array Handler to create dependency on artificial 'length'
  // property of array, mark it as changed when array has changed.
  private static readonly OB_LENGTH = '___obj_length';
  private static readonly OB_MAP_SET_ANY_PROPERTY = '___ob_map_set';
  private static readonly OB_DATE = '__date__';

  // see MonitorV2.observeObjectAccess: bindCmp is the MonitorV2
  // see modified ViewV2 and ViewPU observeComponentCreation, bindCmp is the ViewV2 or ViewPU

  // bindId: UINode elmtId or watchId, depending on what is being observed
  private stackOfRenderedComponents_ : StackOfRenderedComponents = new StackOfRenderedComponents();

  // Map bindId to WeakRef<ViewPU> | MonitorV2
  private id2cmp_: { number: WeakRef<Object> } = {} as { number: WeakRef<Object> };

  // Map bindId -> Set of @observed class objects
  // reverse dependency map for quickly removing all dependencies of a bindId
  private id2targets_: { number: Set<WeakRef<Object>> } = {} as { number: Set<WeakRef<Object>> };

  // queued up Set of bindId
  // elmtIds of UINodes need re-render
  // @monitor functions that need to execute
  private elmtIdsChanged_: Set<number> = new Set();
  private computedPropIdsChanged_: Set<number> = new Set();
  private monitorIdsChanged_: Set<number> = new Set();
  private persistenceChanged_: Set<number> = new Set();
  // avoid recursive execution of updateDirty
  // by state changes => fireChange while
  // UINode rerender or @monitor function execution
  private startDirty_: boolean = false;

  // flag to indicate change observation is disabled
  private disabled_: boolean = false;

  // flag to indicate ComputedV2 calculation is ongoing
  private calculatingComputedProp_: boolean = false;

  private static obsInstance_: ObserveV2;

  public static getObserve(): ObserveV2 {
    if (!this.obsInstance_) {
      this.obsInstance_ = new ObserveV2();
    }
    return this.obsInstance_;
  }

  // return true given value is @observed object
  public static IsObservedObjectV2(value: any): boolean {
    return (value && typeof (value) === 'object' && value[ObserveV2.V2_DECO_META]);
  }

  // return true given value is the return value of makeObserved
  public static IsMakeObserved(value: any): boolean {
    return (value && typeof (value) === 'object' && value[ObserveV2.SYMBOL_MAKE_OBSERVED]);
  }

  public static IsTrackedProperty(parentObj: any, prop: string): boolean {
    const trackedKey = ObserveV2.OB_PREFIX + prop;
    return (parentObj && typeof (parentObj) === 'object' && trackedKey in parentObj);
  }

  public static getCurrentRecordedId(): number {
    const bound = ObserveV2.getObserve().stackOfRenderedComponents_.top();
    return bound ? bound[0] : -1;
  }

  // At the start of observeComponentCreation or
  // MonitorV2 observeObjectAccess
  public startRecordDependencies(cmp: IView | MonitorV2 | ComputedV2 | PersistenceV2Impl, id: number, doClearBinding: boolean = true): void {
    if (cmp != null) {
      doClearBinding && this.clearBinding(id);
      this.stackOfRenderedComponents_.push(id, cmp);
    }
  }

    // At the start of observeComponentCreation or
  // MonitorV2 observeObjectAccess
  public stopRecordDependencies(): void {
    const bound = this.stackOfRenderedComponents_.pop();
    if (bound === undefined) {
      stateMgmtConsole.error('stopRecordDependencies finds empty stack. Internal error!');
      return;
    }
    let targetsSet: Set<WeakRef<Object>>;
    if ((targetsSet = this.id2targets_[bound[0]]) !== undefined && targetsSet.size) {
      // only add IView | MonitorV2 | ComputedV2 if at least one dependency was
      // recorded when rendering this ViewPU/ViewV2/Monitor/ComputedV2
      // ViewPU is the likely case where no dependecy gets recorded
      // for others no dependencies are unlikely to happen
      this.id2cmp_[bound[0]] = new WeakRef<Object>(bound[1]);
    }
  }

  // clear any previously created dependency view model object to elmtId
  // find these view model objects with the reverse map id2targets_
  public clearBinding(id: number): void {
    // multiple weakRefs might point to the same target - here we get Set of unique targets
    const targetSet = new Set<Object>();
    this.id2targets_[id]?.forEach((weak : WeakRef<Object>) => {
      if (weak.deref() instanceof Object) {
        targetSet.add(weak.deref());
      }
    });

    targetSet.forEach((target) => {
      const idRefs: Object | undefined = target[ObserveV2.ID_REFS];
      const symRefs: Object = target[ObserveV2.SYMBOL_REFS];

      if (idRefs) {
        idRefs[id]?.forEach(key => symRefs?.[key]?.delete(id));
        delete idRefs[id];
      } else {
        for (let key in symRefs) {
          symRefs[key]?.delete(id);
        };
      }
    });

    delete this.id2targets_[id];
    delete this.id2cmp_[id];

    stateMgmtConsole.propertyAccess(`clearBinding (at the end): id2cmp_ length=${Object.keys(this.id2cmp_).length}, entries=${JSON.stringify(Object.keys(this.id2cmp_))} `);
    stateMgmtConsole.propertyAccess(`... id2targets_ length=${Object.keys(this.id2targets_).length}, entries=${JSON.stringify(Object.keys(this.id2targets_))} `);
  }

  /**
   *
   * this cleanUpId2CmpDeadReferences()
   * id2cmp is a 'map' object id => WeakRef<Object> where object is ViewV2, ViewPU, MonitorV2 or ComputedV2
   * This method iterates over the object entries and deleted all those entries whose value can no longer
   * be deref'ed.
   *
   * cleanUpId2TargetsDeadReferences()
   * is2targets is a 'map' object id => Set<WeakRef<Object>>
   * the method traverses over the object entries and for each value of type
   * Set<WeakRef<Object>> removes all those items from the set that can no longer be deref'ed.
   *
   * According to JS specifications, it is up to ArlTS runtime GC implementation when to collect unreferences objects.
   * Parameters such as available memory, ArkTS processing load, number and size of all JS objects for GC collection
   * can impact the time delay between an object loosing last reference and GC collecting this object.
   *
   * WeakRef deref() returns the object until GC has collected it.
   * The id2cmp and is2targets cleanup herein depends on WeakRef.deref() to return undefined, i.e. it depends on GC
   * collecting 'cmp' or 'target' objects. Only then the algorithm can remove the entry from id2cmp / from id2target.
   * It is therefore to be expected behavior that these map objects grow and they a contain a larger number of
   * MonitorV2, ComputedV2, and/or view model @Observed class objects that are no longer used / referenced by the application.
   * Only after ArkTS runtime GC has collected them, this function is able to clean up the id2cmp and is2targets.
   *
   * This cleanUpDeadReferences() function gets called from UINodeRegisterProxy.uiNodeCleanUpIdleTask()
   *
   */
  public cleanUpDeadReferences(): void {
    this.cleanUpId2CmpDeadReferences();
    this.cleanUpId2TargetsDeadReferences();
  }

  private cleanUpId2CmpDeadReferences(): void {
    stateMgmtConsole.debug(`cleanUpId2CmpDeadReferences ${JSON.stringify(this.id2cmp_)} `);
    for (const id in this.id2cmp_) {
      stateMgmtConsole.debug('cleanUpId2CmpDeadReferences loop');
      let weakRef: WeakRef<object> = this.id2cmp_[id];
      if (weakRef && typeof weakRef === 'object' && 'deref' in weakRef && weakRef.deref() === undefined) {
        stateMgmtConsole.debug('cleanUpId2CmpDeadReferences cleanup hit');
        delete this.id2cmp_[id];
      }
    }
  }

  private cleanUpId2TargetsDeadReferences(): void {
    for (const id in this.id2targets_) {
      const targetSet: Set<WeakRef<Object>> | undefined = this.id2targets_[id];
      if (targetSet && targetSet instanceof Set) {
        for (let weakTarget of targetSet) {
          if (weakTarget.deref() === undefined) {
            stateMgmtConsole.debug('cleanUpId2TargetsDeadReferences cleanup hit');
            targetSet.delete(weakTarget);
          }
        } // for targetSet
      }
    } // for id2targets_
  }

  /**
   * counts number of WeakRef<Object> entries in id2cmp_ 'map' object
   * @returns total count and count of WeakRefs that can be deref'ed
   * Methods only for testing
   */
  public get id2CompDeRefSize(): [ totalCount: number, aliveCount: number ] {
    let totalCount = 0;
    let aliveCount = 0;
    let comp: Object;
    for (const id in this.id2cmp_) {
      totalCount++;
      let weakRef: WeakRef<Object> = this.id2cmp_[id];
      if (weakRef && 'deref' in weakRef && (comp = weakRef.deref()) && comp instanceof Object) {
        aliveCount++;
      }
    }
    return [totalCount, aliveCount];
  }

  /** counts number of target WeakRef<object> entries in all the Sets inside id2targets 'map' object
 * @returns total count and those can be dereferenced
 * Methods only for testing
 */
  public get id2TargetsDerefSize(): [ totalCount: number, aliveCount: number ] {
    let totalCount = 0;
    let aliveCount = 0;
    for (const id in this.id2targets_) {
      const targetSet: Set<WeakRef<Object>> | undefined = this.id2targets_[id];
      if (targetSet && targetSet instanceof Set) {
        for (let weakTarget of targetSet) {
          totalCount++;
          if (weakTarget.deref()) {
            aliveCount++;
          }
        } // for targetSet
      }
    } // for id2targets_
    return [totalCount, aliveCount];
  }

  // add dependency view model object 'target' property 'attrName'
  // to current this.bindId
  public addRef(target: object, attrName: string): void {
    const bound = this.stackOfRenderedComponents_.top();
    if (!bound) {
      return;
    }
    if (bound[0] === UINodeRegisterProxy.monitorIllegalV2V3StateAccess) {
      const error = `${attrName}: ObserveV2.addRef: trying to use V3 state '${attrName}' to init/update child V2 @Component. Application error`;
      stateMgmtConsole.applicationError(error);
      throw new TypeError(error);
    }

    stateMgmtConsole.propertyAccess(`ObserveV2.addRef '${attrName}' for id ${bound[0]}...`);
    this.addRef4IdInternal(bound[0], target, attrName);
  }

  public addRef4Id(id: number, target: object, attrName: string): void {
    stateMgmtConsole.propertyAccess(`ObserveV2.addRef4Id '${attrName}' for id ${id} ...`);
    this.addRef4IdInternal(id, target, attrName);
  }

  private addRef4IdInternal(id: number, target: object, attrName: string): void {
    // Map: attribute/symbol -> dependent id
    const symRefs = target[ObserveV2.SYMBOL_REFS] ??= {};
    symRefs[attrName] ??= new Set();
    symRefs[attrName].add(id);

    // Map id -> attribute/symbol
    // optimization for faster clearBinding
    const idRefs = target[ObserveV2.ID_REFS];
    if (idRefs) {
      idRefs[id] ??= new Set();
      idRefs[id].add(attrName);
    }

    const targetSet = this.id2targets_[id] ??= new Set<WeakRef<Object>>();
    targetSet.add(new WeakRef<Object>(target));
  }

  /**
   *
   * @param target set tracked attribute to new value without notifying the change
   *               !! use with caution !!
   * @param attrName
   * @param newValue
   */
  public setUnmonitored<Z>(target: object, attrName: string, newValue: Z): void {
    const storeProp = ObserveV2.OB_PREFIX + attrName;
    if (storeProp in target) {
      // @track attrName
      stateMgmtConsole.propertyAccess(`setUnmonitored '${attrName}' - tracked but unchanged. Doing nothing.`);
      target[storeProp] = newValue;
    } else {
      stateMgmtConsole.propertyAccess(`setUnmonitored '${attrName}' - untracked, assigning straight.`);
      // untracked attrName
      target[attrName] = newValue;
    }
  }

  /**
   * Execute given task while state change observation is disabled
   * A state mutation caused by the task will NOT trigger UI rerender
   * and @monitor function execution.
   *
   * !!! Use with Caution !!!
   *
   * @param task a function to execute without monitoring state changes
   * @returns task function return value
   */
  public executeUnobserved<Z>(task: () => Z): Z {
    stateMgmtConsole.propertyAccess(`executeUnobserved - start`);
    this.disabled_ = true;
    let ret: Z;
    try {
      ret = task();
    } catch (e) {
      stateMgmtConsole.applicationError(`executeUnobserved - task execution caused error ${e} !`);
    }
    this.disabled_ = false;
    stateMgmtConsole.propertyAccess(`executeUnobserved - done`);
    return ret;
  }


  // mark view model object 'target' property 'attrName' as changed
  // notify affected watchIds and elmtIds
  public fireChange(target: object, attrName: string): void {
    // enable to get more fine grained traces
    // including 2 (!) .end calls.

    if (!target[ObserveV2.SYMBOL_REFS] || this.disabled_) {
      return;
    }

    const bound = this.stackOfRenderedComponents_.top();
    if (this.calculatingComputedProp_) {
      const prop = bound ? (bound[1] as ComputedV2).getProp() : 'unknown computed property';
      const error = `Usage of ILLEGAL @Computed function detected for ${prop}! The @Computed function MUST NOT change the state of any observed state variable!`;
      stateMgmtConsole.applicationError(error);
      throw new Error(error);
    }

    // enable this trace marker for more fine grained tracing of the update pipeline
    // note: two (!) end markers need to be enabled
    let changedIdSet = target[ObserveV2.SYMBOL_REFS][attrName];
    if (!changedIdSet || !(changedIdSet instanceof Set)) {
      return;
    }

    stateMgmtConsole.propertyAccess(`ObserveV2.fireChange '${attrName}' dependent ids: ${JSON.stringify(Array.from(changedIdSet))}  ...`);

    for (const id of changedIdSet) {
      // Cannot fireChange the object that is being created.
      if (bound && id === bound[0]) {
        continue;
      }

      // if this is the first id to be added to any Set of changed ids,
      // schedule an 'updateDirty' task
      // that will run after the current call stack has unwound.
      // purpose of check for startDirty_ is to avoid going into recursion. This could happen if
      // exec a re-render or exec a monitor function changes some state -> calls fireChange -> ...
      if ((this.elmtIdsChanged_.size + this.monitorIdsChanged_.size + this.computedPropIdsChanged_.size === 0) &&
        /* update not already in progress */ !this.startDirty_) {
        Promise.resolve()
        .then(this.updateDirty.bind(this))
        .catch(error => {
          stateMgmtConsole.applicationError(`Exception occurred during the update process involving @Computed properties, @Monitor functions or UINode re-rendering`, error);
          throw error;
        });
      }

      // add bindId to the correct Set of pending changes.
      if (id < ComputedV2.MIN_COMPUTED_ID) {
        this.elmtIdsChanged_.add(id);
      } else if (id < MonitorV2.MIN_WATCH_ID) {
        this.computedPropIdsChanged_.add(id);
      } else if (id < PersistenceV2Impl.MIN_PERSISTENCE_ID) {
        this.monitorIdsChanged_.add(id);
      } else {
        this.persistenceChanged_.add(id);
      }
    } // for
  }

  public updateDirty(): void {
    this.startDirty_ = true;
    this.updateDirty2(false);
    this.startDirty_ = false;
  }

  /**
   * execute /update in this order
   * - @Computed variables
   * - @Monitor functions
   * - UINode re-render
   * three nested loops, means:
   * process @Computed until no more @Computed need update
   * process @Monitor until no more @Computed and @Monitor
   * process UINode update until no more @Computed and @Monitor and UINode rerender
   *
   * @param updateUISynchronously should be set to true if called during VSYNC only
   *
   */

  public updateDirty2(updateUISynchronously: boolean = false): void {
    aceTrace.begin('updateDirty2');
    stateMgmtConsole.debug(`ObservedV3.updateDirty2 updateUISynchronously=${updateUISynchronously} ... `);
    // obtain and unregister the removed elmtIds
    UINodeRegisterProxy.obtainDeletedElmtIds();
    UINodeRegisterProxy.unregisterElmtIdsFromIViews();

    // priority order of processing:
    // 1- update computed properties until no more need computed props update
    // 2- update monitors until no more monitors and no more computed props
    // 3- update UINodes until no more monitors, no more computed props, and no more UINodes
    // FIXME prevent infinite loops
    do {
      do {
        while (this.computedPropIdsChanged_.size) {
          //  sort the ids and update in ascending order
          // If a @Computed property depends on other @Computed properties, their
          // ids will be smaller as they are defined first.
          const computedProps = Array.from(this.computedPropIdsChanged_).sort((id1, id2) => id1 - id2);
          this.computedPropIdsChanged_ = new Set<number>();
          this.updateDirtyComputedProps(computedProps);
        }

        if (this.persistenceChanged_.size) {
          const persistKeys: Array<number> = Array.from(this.persistenceChanged_);
          this.persistenceChanged_ = new Set<number>();
          PersistenceV2Impl.instance().onChangeObserved(persistKeys);
        }

        if (this.monitorIdsChanged_.size) {
          const monitors = this.monitorIdsChanged_;
          this.monitorIdsChanged_ = new Set<number>();
          this.updateDirtyMonitors(monitors);
        }
      } while (this.monitorIdsChanged_.size + this.persistenceChanged_.size + this.computedPropIdsChanged_.size > 0);

      if (this.elmtIdsChanged_.size) {
        const elmtIds = Array.from(this.elmtIdsChanged_).sort((elmtId1, elmtId2) => elmtId1 - elmtId2);
        this.elmtIdsChanged_ = new Set<number>();
        updateUISynchronously ? this.updateUINodesSynchronously(elmtIds) : this.updateUINodes(elmtIds);
      }
    } while (this.elmtIdsChanged_.size + this.monitorIdsChanged_.size + this.computedPropIdsChanged_.size > 0);

    stateMgmtConsole.debug(`ObservedV3.updateDirty2 updateUISynchronously=${updateUISynchronously} - DONE `);
    aceTrace.end();
  }

  public updateDirtyComputedProps(computed: Array<number>): void {
    stateMgmtConsole.debug(`ObservedV2.updateDirtyComputedProps ${computed.length} props: ${JSON.stringify(computed)} ...`);
    aceTrace.begin(`ObservedV2.updateDirtyComputedProps ${computed.length} @Computed`);
    computed.forEach((id) => {
      let comp: ComputedV2 | undefined;
      let weakComp: WeakRef<ComputedV2 | undefined> = this.id2cmp_[id];
      if (weakComp && 'deref' in weakComp && (comp = weakComp.deref()) && comp instanceof ComputedV2) {
        const target = comp.getTarget();
        if (target instanceof ViewV2 && !target.isViewActive()) {
          // add delayed ComputedIds id
          target.addDelayedComputedIds(id);
        } else {
          comp.fireChange();
        }
      }
    });
    aceTrace.end();
  }


  public updateDirtyMonitors(monitors: Set<number>): void {
    stateMgmtConsole.debug(`ObservedV3.updateDirtyMonitors: ${Array.from(monitors).length} @monitor funcs: ${JSON.stringify(Array.from(monitors))} ...`);
    aceTrace.begin(`ObservedV3.updateDirtyMonitors: ${Array.from(monitors).length} @monitor`);
    let weakMonitor: WeakRef<MonitorV2 | undefined>;
    let monitor: MonitorV2 | undefined;
    let monitorTarget: Object;
    monitors.forEach((watchId) => {
      weakMonitor = this.id2cmp_[watchId];
      if (weakMonitor && 'deref' in weakMonitor && (monitor = weakMonitor.deref()) && monitor instanceof MonitorV2) {
        if (((monitorTarget = monitor.getTarget()) instanceof ViewV2) && !monitorTarget.isViewActive()) {
          // monitor notifyChange delayed if target is a View that is not active
          monitorTarget.addDelayedMonitorIds(watchId);
        } else {
          monitor.notifyChange();
        }
      }
    });
    aceTrace.end();
  }

  /**
   * This version of UpdateUINodes does not wait for VSYNC, violates rules
   * calls UpdateElement, thereby avoids the long and frequent code path from
   * FlushDirtyNodesUpdate to CustomNode to ViewV2.updateDirtyElements to UpdateElement
   * Code left here to reproduce benchmark measurements, compare with future optimisation
   * @param elmtIds
   *
   */
  private updateUINodesSynchronously(elmtIds: Array<number>): void {
    stateMgmtConsole.debug(`ObserveV2.updateUINodesSynchronously: ${elmtIds.length} elmtIds: ${JSON.stringify(elmtIds)} ...`);
    aceTrace.begin(`ObserveV2.updateUINodesSynchronously: ${elmtIds.length} elmtId`);
    let view: Object;
    let weak: any;
    elmtIds.forEach((elmtId) => {
      if ((weak = this.id2cmp_[elmtId]) && (typeof weak === 'object') && ('deref' in weak) &&
        (view = weak.deref()) && ((view instanceof ViewV2) || (view instanceof ViewPU))) {
        if (view.isViewActive()) {
          // FIXME need to call syncInstanceId before update?
          view.UpdateElement(elmtId);
        } else if (view instanceof ViewV2) {
          // schedule delayed update once the view gets active
          view.scheduleDelayedUpdate(elmtId);
        }
      } // if ViewV2 or ViewPU
    });
    aceTrace.end();
  }

  // This is the code path similar to V2, follows the rule that UI updates on VSYNC.
  // ViewPU/ViewV2 queues the elmtId that need update, marks the CustomNode dirty in RenderContext
  // On next VSYNC runs FlushDirtyNodesUpdate to call rerender to call UpdateElement. Much longer code path
  // much slower
  private updateUINodes(elmtIds: Array<number>): void {
    stateMgmtConsole.debug(`ObserveV2.updateUINodes: ${elmtIds.length} elmtIds need rerender: ${JSON.stringify(elmtIds)} ...`);
    aceTrace.begin(`ObserveV2.updateUINodes: ${elmtIds.length} elmtId`);
    let viewWeak: WeakRef<Object>;
    let view: Object | undefined;
    elmtIds.forEach((elmtId) => {
      viewWeak = this.id2cmp_[elmtId];
      if (viewWeak && 'deref' in viewWeak && (view = viewWeak.deref()) &&
        ((view instanceof ViewV2) || (view instanceof ViewPU))) {
        if (view.isViewActive()) {
          view.uiNodeNeedUpdateV3(elmtId);
        } else if (view instanceof ViewV2) {
          // schedule delayed update once the view gets active
          view.scheduleDelayedUpdate(elmtId);
        }
      }
    });
    aceTrace.end();
  }

  public constructMonitor(owningObject: Object, owningObjectName: string): void {
    let watchProp = Symbol.for(MonitorV2.WATCH_PREFIX + owningObjectName);
    if (owningObject && (typeof owningObject === 'object') && owningObject[watchProp]) {
      Object.entries(owningObject[watchProp]).forEach(([pathString, monitorFunc]) => {
        if (monitorFunc && pathString && typeof monitorFunc === 'function') {
          const monitor = new MonitorV2(owningObject, pathString, monitorFunc as (m: IMonitor) => void);
          monitor.InitRun();
          const refs = owningObject[ObserveV2.MONITOR_REFS] ??= {};
          // store a reference inside owningObject
          // thereby MonitorV2 will share lifespan as owning @ComponentV2 or @ObservedV2
          // remember: id2cmp only has a WeakRef to MonitorV2 obj
          refs[monitorFunc.name] = monitor;
        }
        // FIXME Else handle error
      });
    } // if target[watchProp]
  }

  public constructComputed(owningObject: Object, owningObjectName: string): void {
    const computedProp = Symbol.for(ComputedV2.COMPUTED_PREFIX + owningObjectName);
    if (owningObject && (typeof owningObject === 'object') && owningObject[computedProp]) {
      Object.entries(owningObject[computedProp]).forEach(([computedPropertyName, computeFunc]) => {
        stateMgmtConsole.debug(`constructComputed: in ${owningObject?.constructor?.name} found @Computed ${computedPropertyName}`);
        const computed = new ComputedV2(owningObject, computedPropertyName, computeFunc as unknown as () => any);
        computed.InitRun();
        const refs = owningObject[ObserveV2.COMPUTED_REFS] ??= {};
        // store a reference inside owningObject
        // thereby ComputedV2 will share lifespan as owning @ComponentV2 or @ObservedV2
        // remember: id2cmp only has a WeakRef to ComputedV2 obj
        refs[computedPropertyName] = computed;
      });
    }
  }

  public clearWatch(id: number): void {
    this.clearBinding(id);
  }



  public static autoProxyObject(target: Object, key: string | symbol): any {
    let val = target[key];
    // Not an object, not a collection, no proxy required
    if (!val || typeof (val) !== 'object' ||
      !(Array.isArray(val) || val instanceof Set || val instanceof Map || val instanceof Date)) {
      return val;
    }

    // Only collections require proxy observation, and if it has been observed, it does not need to be observed again.
    if (!val[ObserveV2.SYMBOL_PROXY_GET_TARGET]) {
      target[key] = new Proxy(val, ObserveV2.arraySetMapProxy);
      val = target[key];
    }

    // If the return value is an Array, Set, Map
    if (!(val instanceof Date)) {
      ObserveV2.getObserve().addRef(ObserveV2.IsMakeObserved(val) ? RefInfo.get(UIUtilsImpl.instance().getTarget(val)) :
        val, ObserveV2.OB_LENGTH);
    }

    return val;
  }

  // shrinkTo and extendTo is collection.Array api.
  private static readonly arrayLengthChangingFunctions = new Set(['push', 'pop', 'shift', 'splice', 'unshift', 'shrinkTo', 'extendTo']);
  private static readonly arrayMutatingFunctions = new Set(['copyWithin', 'fill', 'reverse', 'sort']);
  private static readonly dateSetFunctions = new Set(['setFullYear', 'setMonth', 'setDate', 'setHours', 'setMinutes',
    'setSeconds', 'setMilliseconds', 'setTime', 'setUTCFullYear', 'setUTCMonth', 'setUTCDate', 'setUTCHours',
    'setUTCMinutes', 'setUTCSeconds', 'setUTCMilliseconds']);

    public static readonly normalObjectHandlerDeepObserved = {
      get(target: object, property: string | Symbol, receiver: any) {
        if (typeof property === 'symbol') {
          if (property === ObserveV2.SYMBOL_PROXY_GET_TARGET) {
            return target;
          }
          if (property === ObserveV2.SYMBOL_MAKE_OBSERVED) {
            return true;
          }
          return target[property];
        }

        let prop = property as string;
        ObserveV2.getObserve().addRef(RefInfo.get(target), prop);
        let ret = target[prop];
        let type = typeof (ret);

        return type === 'function'
          ? ret.bind(receiver)
          : (type === 'object'
            ? RefInfo.get(ret).proxy
            : ret);
      },
      set(target: object, prop: string, value: any, receiver: any): boolean {
        if (target[prop] === value) {
          return true;
        }
        target[prop] = value;
        ObserveV2.getObserve().fireChange(RefInfo.get(target), prop);
        return true;
      }
    };


  public static commonHandlerSet(target: any, key: string | symbol, value: any): boolean {
    if (typeof key === 'symbol') {
      return true;
    }

    if (target[key] === value) {
      return true;
    }

    target[key] = value;

    ObserveV2.getObserve().fireChange(RefInfo.get(target), key.toString());
    return true;
  }


  public static readonly arrayHandlerDeepObserved = {
    get(target: any, key: string | symbol, receiver: any): any {
      if (typeof key === 'symbol') {
        if (key === Symbol.iterator) {
          let refInfo = RefInfo.get(target);
          ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
          ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH);
          return (...args): any => target[key](...args);
        }
        if (key === ObserveV2.SYMBOL_PROXY_GET_TARGET) {
          return target;
        }
        if (key === ObserveV2.SYMBOL_MAKE_OBSERVED) {
          return true;
        }
        return target[key];
      }

      let refInfo = RefInfo.get(target);
      let ret = target[key];

      if (typeof (ret) !== 'function') {
        ObserveV2.getObserve().addRef(refInfo, key === 'length' ? ObserveV2.OB_LENGTH : key);
        return (typeof (ret) === 'object') ? RefInfo.get(ret).proxy : ret;
      }

      if (ObserveV2.arrayMutatingFunctions.has(key as string)) {
        return function (...args): any {
          ret.call(target, ...args);
          ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_LENGTH);
          // returning the 'receiver(proxied object)' ensures that when chain calls also 2nd function call
          // operates on the proxied object.
          return receiver;
        };
      } else if (ObserveV2.arrayLengthChangingFunctions.has(key as string)) {
        return function (...args): any {
          const result = ret.call(target, ...args);
          ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_LENGTH);
          return result;
        };
      } else if (key === 'forEach') {
        // to make ForEach Component and its Item can addref
        ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH)
        return function (callbackFn: (value: any, index: number, array: Array<any>) => void): any {
          const result = ret.call(target, (value: any, index: number, array: Array<any>) => {
            // Collections.Array will report BusinessError: The foreach cannot be bound if call "receiver".
            // because the passed parameter is not the instance of the container class.
            // so we must call "target" here to deal with the collections situations.
            // But we also need to addref for each index.
            receiver[index];
            callbackFn(typeof value == 'object' ? RefInfo.get(value).proxy : value, index, receiver);
          });
          return result;
        }
      } else {
        return ret.bind(target);
      }

    },
    set(target: any, key: string | symbol, value: any): boolean {

      if (typeof key === 'symbol' || target[key] === value) {
        return true;
      }

      const originalLength = target.length;
      target[key] = value;
      const arrayLenChanged = target.length !== originalLength;
      ObserveV2.getObserve().fireChange(RefInfo.get(target), arrayLenChanged ? ObserveV2.OB_LENGTH : key.toString());
      return true;
    }
  }

  public static readonly setMapHandlerDeepObserved = {
    get(target: any, key: string | symbol, receiver: any): any {
      if (typeof key === 'symbol') {
        if (key === Symbol.iterator) {
          let refInfo = RefInfo.get(target);
          ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
          ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH);
          return (...args): any => target[key](...args);
        }
        if (key === ObserveV2.SYMBOL_PROXY_GET_TARGET) {
          return target;
        }
        if (key === ObserveV2.SYMBOL_MAKE_OBSERVED) {
          return true;
        }
        return target[key];
      }

      let refInfo = RefInfo.get(target);
      if (key === 'size') {
        ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH);
        return target.size;
      }

      let ret = target[key];
      if (typeof (ret) !== 'function') {
        if (typeof (ret) === 'object') {
          let wrapper = RefInfo.get(ret);
          ObserveV2.getObserve().addRef(refInfo, key);
          return wrapper.proxy;
        }
        if (key === 'length') {
          ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH);
        }
        return ret;
      }

      if (key === 'has') {
        return (prop): boolean => {
          const ret = target.has(prop);
          if (ret) {
            ObserveV2.getObserve().addRef(refInfo, prop);
          } else {
            ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH);
          }
          return ret;
        };
      }
      if (key === 'delete') {
        return (prop): boolean => {
          if (target.has(prop)) {
            ObserveV2.getObserve().fireChange(refInfo, prop);
            ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_LENGTH);
            return target.delete(prop);
          } else {
            return false;
          }
        };
      }
      if (key === 'clear') {
        return (): void => {
          if (target.size > 0) {
            target.forEach((_, prop) => {
              ObserveV2.getObserve().fireChange(refInfo, prop.toString());
            });
            ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_LENGTH);
            ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
            target.clear();
          }
        };
      }
      if (key === 'keys' || key === 'values' || key === 'entries') {
        return (): any => {
          ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
          ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH);
          return target[key]();
        };
      }


      if (target instanceof Set || SendableType.isSet(target)) {
        return key === 'add' ?
          (val): any => {
            ObserveV2.getObserve().fireChange(refInfo, val.toString());
            ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
            if (!target.has(val)) {
              ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_LENGTH);
              target.add(val);
            }
            // return proxied This
            return receiver;
          } : (typeof ret === 'function')
            ? ret.bind(target) : ret;
      }

      if (target instanceof Map || SendableType.isMap(target)) {
        if (key === 'get') { // for Map
          return (prop): any => {
            if (target.has(prop)) {
              ObserveV2.getObserve().addRef(refInfo, prop);
            } else {
              ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_LENGTH);
            }
            let ret = target.get(prop);

            return typeof ret === 'object' ? RefInfo.get(ret).proxy : ret;
          };
        }
        if (key === 'set') { // for Map
          return (prop, val): any => {
            if (!target.has(prop)) {
              ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_LENGTH);
            } else if (target.get(prop) !== val) {
              ObserveV2.getObserve().fireChange(refInfo, prop);
            }
            ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
            target.set(prop, val);
            return true;
          };
        }
      }
      return (typeof ret === 'function') ? ret.bind(target) : ret;
    },
    set(target: any, key: string | symbol, value: any): boolean {
      return ObserveV2.commonHandlerSet(target, key, value);
    }
  }

  public static readonly dateHandlerDeepObserved = {
    get(target: any, key: string | symbol, receiver: any): any {
      if (typeof key === 'symbol') {
        if (key === ObserveV2.SYMBOL_PROXY_GET_TARGET) {
          return target;
        }
        if (key === ObserveV2.SYMBOL_MAKE_OBSERVED) {
          return true;
        }
        return target[key];
      }

      let ret = target[key];
      let refInfo = RefInfo.get(target);
      if (ObserveV2.dateSetFunctions.has(key)) {
        return function (...args): any {
          // execute original function with given arguments
          let result = ret.call(this, ...args);
          ObserveV2.getObserve().fireChange(refInfo, ObserveV2.OB_DATE);
          return result;
          // bind 'this' to target inside the function
        }.bind(target);
      } else {
        ObserveV2.getObserve().addRef(refInfo, ObserveV2.OB_DATE);
      }
      return ret.bind(target);
    },

    set(target: any, key: string | symbol, value: any): boolean {
      return ObserveV2.commonHandlerSet(target, key, value);
    }
  };

  public static readonly arraySetMapProxy = {
    get(
      target: any,
      key: string | symbol,
      receiver: any
    ): any {
      if (typeof key === 'symbol') {
        if (key === Symbol.iterator) {
          ObserveV2.getObserve().fireChange(target, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
          ObserveV2.getObserve().addRef(target, ObserveV2.OB_LENGTH);
          return (...args): any => target[key](...args);
        } else {
          return key === ObserveV2.SYMBOL_PROXY_GET_TARGET ? target : target[key];
        }
      }

      if (key === 'size') {
        ObserveV2.getObserve().addRef(target, ObserveV2.OB_LENGTH);
        return target.size;
      }

      let ret = ObserveV2.autoProxyObject(target, key);
      if (typeof (ret) !== 'function') {
        ObserveV2.getObserve().addRef(target, key);
        return ret;
      }

      if (Array.isArray(target)) {
        if (ObserveV2.arrayMutatingFunctions.has(key)) {
          return function (...args): any {
            ret.call(target, ...args);
            ObserveV2.getObserve().fireChange(target, ObserveV2.OB_LENGTH);
            // returning the 'receiver(proxied object)' ensures that when chain calls also 2nd function call
            // operates on the proxied object.
            return receiver;
          };
        } else if (ObserveV2.arrayLengthChangingFunctions.has(key)) {
          return function (...args): any {
            const result = ret.call(target, ...args);
            ObserveV2.getObserve().fireChange(target, ObserveV2.OB_LENGTH);
            return result;
          };
        } else {
          return ret.bind(receiver);
        }
      }

      if (target instanceof Date) {
        if (ObserveV2.dateSetFunctions.has(key)) {
          return function (...args): any {
            // execute original function with given arguments
            let result = ret.call(this, ...args);
            ObserveV2.getObserve().fireChange(target, ObserveV2.OB_DATE);
            return result;
            // bind 'this' to target inside the function
          }.bind(target);
        } else {
          ObserveV2.getObserve().addRef(target, ObserveV2.OB_DATE);
        }
        return ret.bind(target);
      }

      if (target instanceof Set || target instanceof Map) {
        if (key === 'has') {
          return (prop): boolean => {
            const ret = target.has(prop);
            if (ret) {
              ObserveV2.getObserve().addRef(target, prop);
            } else {
              ObserveV2.getObserve().addRef(target, ObserveV2.OB_LENGTH);
            }
            return ret;
          };
        }
        if (key === 'delete') {
          return (prop): boolean => {
            if (target.has(prop)) {
              ObserveV2.getObserve().fireChange(target, prop);
              ObserveV2.getObserve().fireChange(target, ObserveV2.OB_LENGTH);
              return target.delete(prop);
            } else {
              return false;
            }
          };
        }
        if (key === 'clear') {
          return (): void => {
            if (target.size > 0) {
              target.forEach((_, prop) => {
                ObserveV2.getObserve().fireChange(target, prop.toString());
              });
              ObserveV2.getObserve().fireChange(target, ObserveV2.OB_LENGTH);
              ObserveV2.getObserve().addRef(target, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
              target.clear();
            }
          };
        }
        if (key === 'keys' || key === 'values' || key === 'entries') {
          return (): any => {
            ObserveV2.getObserve().addRef(target, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
            ObserveV2.getObserve().addRef(target, ObserveV2.OB_LENGTH);
            return target[key]();
          };
        }
      }

      if (target instanceof Set) {
        return key === 'add' ?
        (val): any => {
          ObserveV2.getObserve().fireChange(target, val.toString());
          ObserveV2.getObserve().fireChange(target, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
          if (!target.has(val)) {
            ObserveV2.getObserve().fireChange(target, ObserveV2.OB_LENGTH);
            target.add(val);
          }
          // return proxied This
          return receiver;
        } : (typeof ret === 'function')
          ? ret.bind(target) : ret;
      }

      if (target instanceof Map) {
        if (key === 'get') { // for Map
          return (prop): any => {
            if (target.has(prop)) {
              ObserveV2.getObserve().addRef(target, prop);
            } else {
              ObserveV2.getObserve().addRef(target, ObserveV2.OB_LENGTH);
            }
            return target.get(prop);
          };
        }
        if (key === 'set') { // for Map
          return (prop, val): any => {
            if (!target.has(prop)) {
              ObserveV2.getObserve().fireChange(target, ObserveV2.OB_LENGTH);
            } else if (target.get(prop) !== val) {
              ObserveV2.getObserve().fireChange(target, prop);
            }
            ObserveV2.getObserve().fireChange(target, ObserveV2.OB_MAP_SET_ANY_PROPERTY);
            target.set(prop, val);
            return receiver;
          };
        }
      }

      return (typeof ret === 'function') ? ret.bind(target) : ret;
    },

    set(
      target: any,
      key: string | symbol,
      value: any
    ): boolean {
      if (typeof key === 'symbol') {
        if (key !== ObserveV2.SYMBOL_PROXY_GET_TARGET) {
          target[key] = value;
        }
        return true;
      }

      if (target[key] === value) {
        return true;
      }
      if (Array.isArray(target)) {
        const originalLength = target.length;
        target[key] = value;
        if (target.length !== originalLength) {
            ObserveV2.getObserve().fireChange(target, ObserveV2.OB_LENGTH);
            // autoProxyObject function adds ref to OB_LENGTH for all arrays that
            // are not MakeObserved. No need to fire key.toString() separately. Just return.
            return true;
        }
      } else {
          target[key] = value;
      }
      ObserveV2.getObserve().fireChange(target, key.toString());
      return true;
    }
  };

  /**
   * Helper function to add meta data about decorator to ViewPU or ViewV2
   * @param proto prototype object of application class derived from  ViewPU or ViewV2
   * @param varName decorated variable
   * @param deco '@state', '@event', etc (note '@model' gets transpiled in '@param' and '@event')
   */
  public static addVariableDecoMeta(proto: Object, varName: string, deco: string): void {
    // add decorator meta data
    const meta = proto[ObserveV2.V2_DECO_META] ??= {};
    meta[varName] = {};
    meta[varName].deco = deco;

    // FIXME
    // when splitting ViewPU and ViewV3
    // use instanceOf. Until then, this is a workaround.
    // any @state, @track, etc V3 event handles this function to return false
    Reflect.defineProperty(proto, 'isViewV3', {
      get() { return true; },
      enumerable: false
    }
    );
  }


  public static addParamVariableDecoMeta(proto: Object, varName: string, deco?: string, deco2?: string): void {
    // add decorator meta data
    const meta = proto[ObserveV2.V2_DECO_META] ??= {};
    meta[varName] ??= {};
    if (deco) {
      meta[varName].deco = deco;
    }
    if (deco2) {
      meta[varName].deco2 = deco2;
    }

    // FIXME
    // when splitting ViewPU and ViewV3
    // use instanceOf. Until then, this is a workaround.
    // any @state, @track, etc V3 event handles this function to return false
    Reflect.defineProperty(proto, 'isViewV3', {
      get() { return true; },
      enumerable: false
    }
    );
  }


  public static usesV3Variables(proto: Object): boolean {
    return (proto && typeof proto === 'object' && proto[ObserveV2.V2_DECO_META]);
  }
} // class ObserveV2


const trackInternal = (
  target: any,
  propertyKey: string
): void => {
  if (typeof target === 'function' && !Reflect.has(target, propertyKey)) {
    // dynamic track，and it not a static attribute
    target = target.prototype;
  }
  const storeProp = ObserveV2.OB_PREFIX + propertyKey;
  target[storeProp] = target[propertyKey];
  Reflect.defineProperty(target, propertyKey, {
    get() {
      ObserveV2.getObserve().addRef(this, propertyKey);
      return ObserveV2.autoProxyObject(this, ObserveV2.OB_PREFIX + propertyKey);
    },
    set(val) {
      // If the object has not been observed, you can directly assign a value to it. This improves performance.
      if (val !== this[storeProp]) {
        this[storeProp] = val;
        if (this[ObserveV2.SYMBOL_REFS]) { // This condition can improve performance.
          ObserveV2.getObserve().fireChange(this, propertyKey);
        }
      }
    },
    enumerable: true
  });
  // this marks the proto as having at least one @track property inside
  // used by IsObservedObjectV2
  target[ObserveV2.V2_DECO_META] ??= {};
}; // trackInternal
