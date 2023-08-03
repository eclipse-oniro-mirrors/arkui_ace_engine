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

/**
 * ObservedPropertyAbstractPU aka ObservedPropertyAbstract for partial update
 * 
 * all definitions in this file are framework internal
 */


abstract class ObservedPropertyAbstractPU<T> extends ObservedPropertyAbstract<T> 
implements ISinglePropertyChangeSubscriber<T>, IMultiPropertiesChangeSubscriber, IMultiPropertiesReadSubscriber
// these interfaces implementations are all empty functions, overwrite FU base class implementations.
{
  static readonly DelayedNotifyChangesEnum=class  {
    static readonly do_not_delay = 0;
    static readonly delay_none_pending = 1;
    static readonly delay_notification_pending = 2;
  };
  
  private owningView_ : ViewPU = undefined;
  
  private dependentElementIds_: Set<number> = new Set<number>();

  // PU code stores object references to dependencies directly as class variable
  // SubscriberManager is not used for lookup in PU code path to speedup updates
  protected subscriberRefs_: Set<IPropertySubscriber>;
  
  // when owning ViewPU is inActive, delay notifying changes
  private delayedNotification_: number = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.do_not_delay;

  constructor(subscriber: IPropertySubscriber, viewName: PropertyInfo) {
    super(subscriber, viewName);
    Object.defineProperty(this, 'owningView_', {writable: true, enumerable: false});
    Object.defineProperty(this, 'subscriberRefs_',
      {writable: true, enumerable: false, value: new Set<IPropertySubscriber>()});
    if(subscriber) {
      if (subscriber instanceof ViewPU) {
        this.owningView_ = subscriber;
      } else {
        this.subscriberRefs_.add(subscriber);
      }
    }
  }

  aboutToBeDeleted() {
    super.aboutToBeDeleted();
    this.subscriberRefs_.clear();
    this.owningView_ = undefined;
  }

  /*
    Virtualized version of the subscription mechanism - add subscriber
    Overrides implementation in ObservedPropertyAbstract<T>
  */
  public addSubscriber(subscriber: ISinglePropertyChangeSubscriber<T>):void {
    if (subscriber) {
      // ObservedPropertyAbstract will also add subscriber to
      // SubscriberManager map and to its own Set of subscribers as well
      // Something to improve in the future for PU path.
      // subscribeMe should accept IPropertySubscriber interface
      super.subscribeMe(subscriber as ISinglePropertyChangeSubscriber<T>);
      this.subscriberRefs_.add(subscriber);
    }
  }

  /*
    Virtualized version of the subscription mechanism - remove subscriber
    Overrides implementation in ObservedPropertyAbstract<T>
  */
  public removeSubscriber(subscriber: IPropertySubscriber, id?: number):void {
    if (subscriber) {
      this.subscriberRefs_.delete(subscriber);
      if (!id) {
        id = subscriber.id__();
      }
    }
    super.unlinkSuscriber(id);
  }


  /**
   * put the property to delayed notification mode
   * feature is only used for @StorageLink/Prop, @LocalStorageLink/Prop
   */
  public enableDelayedNotification() : void {
  if (this.delayedNotification_ != ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_notification_pending) {
      stateMgmtConsole.debug(`${this.constructor.name}: enableDelayedNotification.`);
      this.delayedNotification_ = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_none_pending;
    }
  }

  /*
     when moving from inActive to active state the owning ViewPU calls this function
     This solution is faster than ViewPU polling each variable to send back a viewPropertyHasChanged event
     with the elmtIds

    returns undefined if variable has _not_ changed
    returns dependentElementIds_ Set if changed. This Set is empty if variable is not used to construct the UI
  */
  public moveElmtIdsForDelayedUpdate(): Set<number> | undefined {
    const result = (this.delayedNotification_ == ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_notification_pending)
      ? this.dependentElementIds_
      : undefined;
    stateMgmtConsole.debug(`${this.constructor.name}: moveElmtIdsForDelayedUpdate: elmtIds that need delayed update ${result ? Array.from(result).toString() : 'no delayed notifications'} .`);
    this.delayedNotification_ = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.do_not_delay;
    return result;
  }

  protected notifyPropertyRead() {
    stateMgmtConsole.error(`ObservedPropertyAbstractPU[${this.id__()}, '${this.info() || "unknown"}']: \
        notifyPropertyRead, DO NOT USE with PU. Use notifyPropertyHasBeenReadPU`);
  }

  protected notifyPropertyHasBeenReadPU() {
    stateMgmtConsole.debug(`ObservedPropertyAbstractPU[${this.id__()}, '${this.info() || "unknown"}']: notifyPropertyHasBeenReadPU.`)
    this.subscriberRefs_.forEach((subscriber) => {
      if (subscriber) {
        // TODO
        // propertyHasBeenReadPU is not use in the code
        // defined by interface that is not used either: PropertyReadEventListener
        // Maybe compiler generated code has it?
        if ('propertyHasBeenReadPU' in subscriber) {
          (subscriber as unknown as PropertyReadEventListener<T>).propertyHasBeenReadPU(this);
        }
      }
    });
    this.recordDependentUpdate();
  } 

  protected notifyPropertyHasChangedPU() {
    stateMgmtConsole.debug(`ObservedPropertyAbstractPU[${this.id__()}, '${this.info() || "unknown"}']: notifyPropertyHasChangedPU.`)
    if (this.owningView_) {
      if (this.delayedNotification_ == ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.do_not_delay) {
        // send viewPropertyHasChanged right away
        this.owningView_.viewPropertyHasChanged(this.info_, this.dependentElementIds_);
      } else {
        // mark this @StorageLink/Prop or @LocalStorageLink/Prop variable has having changed and notification of viewPropertyHasChanged delivery pending
        this.delayedNotification_ = ObservedPropertyAbstractPU.DelayedNotifyChangesEnum.delay_notification_pending;
      }
    }
    this.subscriberRefs_.forEach((subscriber) => {
      if (subscriber) {
        if ('syncPeerHasChanged' in subscriber) {
          (subscriber as unknown as PeerChangeEventReceiverPU<T>).syncPeerHasChanged(this);
        } else  {
          stateMgmtConsole.warn(`ObservedPropertyAbstractPU[${this.id__()}, '${this.info() || "unknown"}']: notifyPropertryHasChangedPU: unknown subscriber ID 'subscribedId' error!`);
        }
      }
    });
  }  

  
  public markDependentElementsDirty(view: ViewPU) {
    // TODO ace-ets2bundle, framework, complicated apps need to update together
    // this function will be removed after a short transition period.
    stateMgmtConsole.warn(`ObservedPropertyAbstractPU[${this.id__()}, '${this.info() || "unknown"}']: markDependentElementsDirty no longer supported. App will work ok, but
        please update your ace-ets2bundle and recompile your application!`);
  }

  public numberOfSubscrbers(): number {
    return this.subscriberRefs_.size + (this.owningView_ ? 1 : 0);
  }

  /*
   type checking for any supported type, as required for union type support
    see 1st parameter for explanation what is allowed

    FIXME this expects the Map, Set patch to go in
  */

  protected checkIsSupportedValue(value: T): boolean {
    return this.checkNewValue(
      `undefined, null, number, boolean, string, or Object but not function`,
      value,
      () => ((typeof value == "object" && typeof value != "function")
        || typeof value == "number" || typeof value == "string" || typeof value == "boolean")
        || (value == undefined || value == null)
    );
  }

  /*
    type checking for allowed Object type value
    see 1st parameter for explanation what is allowed

      FIXME this expects the Map, Set patch to go in
    */
  protected checkIsObject(value: T): boolean {
    return this.checkNewValue(
      `undefined, null, Object including Array and instance of SubscribableAbstract and excluding function, Set, and Map`,
      value,
      () => (value == undefined || value == null || (typeof value == "object"))
    );
  }

  /*
    type checking for allowed simple types value
    see 1st parameter for explanation what is allowed
 */
  protected checkIsSimple(value: T): boolean {
    return this.checkNewValue(
      `undefined, number, boolean, string`,
      value,
      () => (value == undefined || typeof value == "number" || typeof value == "string" || typeof value == "boolean")
    );
  }
    
  private static readonly mapDeco = new Map<string, string>([
    ["ObservedPropertyObjectPU", "@State/@Provide"],
    ["ObservedPropertySimplePU", "@State/@Provide (error, should not be used)"],
    ["SynchedPropertyObjectOneWayPU", "@Prop"],
    ["SynchedPropertySimpleOneWayPU", "@Prop  (error, should not be used)"],
    ["SynchedPropertyObjectTwoWayPU", "@Link/@Consume"],
    ["SynchedPropertySimpleTwoWayPU", "@Link/@Consume (error, should not be used)"],
    ["SynchedPropertyNestedObjectPU", "@ObjectLink (only class-objects supported"],
    ["SynchedPropertyNesedObjectPU", "@ObjectLink (only class-objects supported"]
  ]);

  protected checkNewValue(isAllowedComment : string, newValue: T, validator: (value: T) => boolean) : boolean {
    if (validator(newValue)) {
      return true;
    } 

    // report error
    // current implementation throws an Exception
    errorReport.varValueCheckFailed({
      customComponent: this.owningView_? this.owningView_.constructor.name : "unknown owningView / internal error",
      variableDeco: ObservedPropertyAbstractPU.mapDeco.get(this.constructor.name),
      variableName: this.info(),
      expectedType: isAllowedComment,
      value: newValue
    });

    // never gets here if errorReport.varValueCheckFailed throws an exception
    // but should nto depend on its implementation
    return false;
  }

  
  /**
   * factory function for concrete 'object' or 'simple' ObservedProperty object
   * depending if value is Class object
   * or simple type (boolean | number | string)
   * @param value
   * @param owningView
   * @param thisPropertyName
   * @returns either
   */
  static CreateObservedObject<C>(value: C, owningView: IPropertySubscriber, thisPropertyName: PropertyInfo)
    : ObservedPropertyAbstract<C> {
    return (typeof value === "object") ?
      new ObservedPropertyObject(value, owningView, thisPropertyName)
      : new ObservedPropertySimple(value, owningView, thisPropertyName);
  }


  /**
   * during 'get' access recording take note of the created component and its elmtId
   * and add this component to the list of components who are dependent on this property
   */
  protected recordDependentUpdate() {
    const elmtId = ViewStackProcessor.GetElmtIdToAccountFor();
    if (elmtId < 0) {
      // not access recording 
      return;
    }
    stateMgmtConsole.debug(`ObservedPropertyAbstract[${this.id__()}, '${this.info() || "unknown"}']: recordDependentUpdate on elmtId ${elmtId}.`)
    this.dependentElementIds_.add(elmtId);
  }

  
  public purgeDependencyOnElmtId(rmElmtId: number): void {
    stateMgmtConsole.debug(`ObservedPropertyAbstractPU[${this.id__()}, '${this.info() || "unknown"}']:purgeDependencyOnElmtId ${rmElmtId}`);
    this.dependentElementIds_.delete(rmElmtId);
  }

  public SetPropertyUnchanged(): void {
    // function to be removed
    // keep it here until transpiler is updated.
  }

  // FIXME check, is this used from AppStorage.
  // unified Appstorage, what classes to use, and the API
  public createLink(subscribeOwner?: IPropertySubscriber,
    linkPropName?: PropertyInfo): ObservedPropertyAbstractPU<T> {
    throw new Error("Can not create a AppStorage 'Link' from a @State property. ");
  }

  public createProp(subscribeOwner?: IPropertySubscriber,
    linkPropName?: PropertyInfo): ObservedPropertyAbstractPU<T> {
    throw new Error("Can not create a AppStorage 'Prop' from a @State property. ");
  }

  /*
    Below empty functions required to keep as long as this class derives from FU version
    ObservedPropertyAbstract. Need to overwrite these functions to do nothing for PU
    */
    protected notifyHasChanged(_: T) {
      stateMgmtConsole.error(`ObservedPropertyAbstractPU[${this.id__()}, '${this.info() || "unknown"}']: \
          notifyHasChanged, DO NOT USE with PU. Use syncPeerHasChanged() or objectPropertyHasChangedPU()`);
    }
  
    hasChanged(_: T): void {
      // unused for PU
      // need to overwrite impl of base class with empty function.
    }
  
    propertyHasChanged(_?: PropertyInfo): void {
      // unused for PU
      // need to overwrite impl of base class with empty function.
    }
  
    propertyRead(_?: PropertyInfo): void {
      // unused for PU
      // need to overwrite impl of base class with empty function.
    }
}