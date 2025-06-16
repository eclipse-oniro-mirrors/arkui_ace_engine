/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { UIContext } from "@ohos/arkui/UIContext"
import { WrappedBuilder } from "./component/builder"
import { FrameNode, BuilderRootFrameNode, FrameNodeUtils } from "./FrameNode"

import { BuilderNode, BuildOptions, voidBuilderFunc, TBuilderFunc } from "./BuilderNode"

import { Content } from "./Content"
import { KPointer } from "@koalaui/interop"

export class ComponentContent<T = undefined> implements Content {
    private builderNode_: BuilderNode<T>;
    private parentWeak_?: WeakRef<FrameNode>;
    //@ts-ignore
    constructor(uiContext: UIContext, builder: WrappedBuilder<TBuilderFunc<T>>, params: T, options?: BuildOptions) {
        let builderNode = new BuilderNode<T>(uiContext, {});
        this.builderNode_ = builderNode;
        this.builderNode_.build(builder, params, options);
    }

    //@ts-ignore
    constructor(uiContext: UIContext, builder: WrappedBuilder<voidBuilderFunc>) {
        let builderNode = new BuilderNode<T>(uiContext, {});
        this.builderNode_ = builderNode;
        //@ts-ignore
        this.builderNode_.build(builder);
    }

    update(params: T): void {
        this.builderNode_.update(params);
    }
    getFrameNode(): FrameNode | null | undefined {
        return this.builderNode_.getFrameNodeWithoutCheck();
    }
    getNodePtr(): KPointer | undefined {
        return this.builderNode_.getNodePtr();
    }
    reuse(param?: Record<string, Object>): void {
        this.builderNode_.reuse(param);
    }
    recycle(): void {
        this.builderNode_.recycle();
    }
    dispose(): void {
        this.detachFromParent();
        this.builderNode_?.dispose();
    }
    updateConfiguration(): void {
        this.builderNode_.updateConfiguration();
    }
    setAttachedParent(frameNode?: WeakRef<FrameNode>): void {
        this.parentWeak_ = frameNode;
    }
    detachFromParent(): void {
        if (this.parentWeak_ === undefined) {
            return;
        }
        let parent = this.parentWeak_?.deref();
        if (parent !== undefined) {
            parent.removeComponentContent(this);
        }
    }
}