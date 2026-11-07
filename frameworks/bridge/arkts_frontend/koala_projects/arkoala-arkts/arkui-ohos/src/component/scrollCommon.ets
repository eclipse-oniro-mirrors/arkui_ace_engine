/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

// Merged with generated/component/scrollCommon.ets during abc preprocessing.
// Keep provider methods on the managed receiver; only its bound proxy callbacks are serialized.
export interface ScrollSnapOffsetProvider {
    calculateApproachOffset(velocity: double, decayOffset: double): double;
    calculateSnapOffset(velocity: double): double;
}
