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
 * SynchedPropertyNesedObjectPU
 * implementation of @ObjectLink decorated variables
 * 
 * all definitions in this file are framework internal
 * 
 */
class SynchedPropertyNesedObjectPU<C extends Object>
  extends ObservedPropertyObjectAbstractPU<C>
  implements ObservedObjectEventsPUReceiver<C> {

  private obsObject_: C;

  /**
   * Construct a Property of a su component that links to a variable of parent view that holds an ObservedObject
   * example
   *   this.b.$a with b of type PC and a of type C, or
   *   this.$b[5] with this.b of type PC and array item b[5] of type C;
   *
   * @param subscribeMe
   * @param propName
   */
  constructor(obsObject: C,
    owningChildView: IPropertySubscriber, propertyName: PropertyInfo) {
    super(owningChildView, propertyName);

    if (obsObject==undefined || obsObject == null || typeof obsObject != "object") {  
      stateMgmtConsole.error(`SynchedPropertyNesedObjectPU[${this.id__()}, '${this.info() || "unknown"}']: constructor @ObjectLink value must be an object, and it must not be undefined or null!.`);
      // TODO enable support for undefined and null
      return;
    }
    this.obsObject_ = obsObject;

    // register to the ObservedObject
    ObservedObject.addOwningProperty(this.obsObject_, this);
  }

  /*
  like a destructor, need to call this before deleting
  the property.
  */
  aboutToBeDeleted() {
    // unregister from the ObservedObject
    ObservedObject.removeOwningProperty(this.obsObject_, this);
    super.aboutToBeDeleted();
  }

  public objectPropertyHasChangedPU(eventSource: ObservedObject<C>, changedPropertyName: string) {
    stateMgmtConsole.debug(`SynchedPropertyNesedObjectPU[${this.id__()}, '${this.info() || "unknown"}']: \
        objectPropertyHasChangedPU: contained ObservedObject property '${changedPropertyName}' has changed.`)
    this.notifyPropertyHasChangedPU();
  }


  public objectPropertyHasBeenReadPU(souceObject: ObservedObject<C>, changedPropertyName : string) {
    stateMgmtConsole.debug(`SynchedPropertyNesedObjectPU[${this.id__()}, '${this.info() || "unknown"}']: \
    objectPropertyHasBeenReadPU: contained ObservedObject property '${changedPropertyName}' has been read.`);
    this.notifyPropertyHasBeenReadPU();
  }
  
  public getUnmonitored(): C {
    stateMgmtConsole.debug(`SynchedPropertyNesedObject[${this.id__()}, '${this.info() || "unknown"}']: getUnmonitored.`);
    // unmonitored get access , no call to otifyPropertyRead !
    return this.obsObject_;
  }

  // get 'read through` from the ObservedProperty
  public get(): C {
    stateMgmtConsole.debug(`SynchedPropertyNesedObjectPU[${this.id__()}, '${this.info() || "unknown"}']: get`)
    this.notifyPropertyHasBeenReadPU()
    return this.obsObject_;
  }

  // set 'writes through` to the ObservedProperty
  public set(newValue: C): void {
    if (newValue==undefined || newValue == null || typeof newValue != "object") {  
      stateMgmtConsole.error(`SynchedPropertyNesedObjectPU[${this.id__()}, '${this.info() || "unknown"}']: set: @ObjectLink value must be an object, and it must not be undefined or null!.`);
      // TODO enable support for undefined and null
      return;
    }

    if (this.obsObject_ == newValue) {
      stateMgmtConsole.debug(`SynchedPropertyNesedObjectPu[${this.id__()}IP, '${this.info() || "unknown"}']: set @ObjectLink with unchanged value - nothing to do.`);
      return;
    }

    stateMgmtConsole.debug(`SynchedPropertyNesedObjectPU[${this.id__()}, '${this.info() || "unknown"}']: set: @ObjectLink set to new value: .`);

    // unsubscribe from the old value ObservedObject
    ObservedObject.removeOwningProperty(this.obsObject_, this);

    this.obsObject_ = newValue;

    // subscribe to the new value ObservedObject
    ObservedObject.addOwningProperty(this.obsObject_, this);

    // notify value change to subscribing View
    this.notifyPropertyHasChangedPU();
  }
}
