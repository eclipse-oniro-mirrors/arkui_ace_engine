/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import * as arkts from "@koalaui/libarkts"
import checkedTransformer from "./MemoTransformer"
import parsedTransformer from "./ParserTransformer"

export function init(parsedJson?: Object, checkedJson?: Object) {
    let pluginContext = new arkts.PluginContextImpl()
    const parsedHooks = new arkts.DumpingHooks(arkts.Es2pandaContextState.ES2PANDA_STATE_PARSED, "memo")
    const checkedHooks = new arkts.DumpingHooks(arkts.Es2pandaContextState.ES2PANDA_STATE_CHECKED, "memo")
    return {
        name: "memo",
        parsed(hooks: arkts.RunTransformerHooks = parsedHooks) {
            arkts.Tracer.pushContext('memo-plugin')
            arkts.trace(() => "Run parsed state plugin", true)
            const transform = parsedTransformer(parsedJson)
            const prog = arkts.arktsGlobal.compilerContext.program
            const state = arkts.Es2pandaContextState.ES2PANDA_STATE_PARSED
            try {
                arkts.runTransformer(prog, state, transform, pluginContext, hooks)
                arkts.Tracer.popContext()
            } catch(e) {
                console.trace(e)
                throw e
            }
        },
        checked(hooks: arkts.RunTransformerHooks = checkedHooks) {
            arkts.Tracer.pushContext('memo-plugin')
            arkts.trace(() => "Run checked state plugin", true)
            const transform = checkedTransformer(checkedJson)
            const prog = arkts.arktsGlobal.compilerContext.program
            const state = arkts.Es2pandaContextState.ES2PANDA_STATE_CHECKED
            try {
                arkts.runTransformer(prog, state, transform, pluginContext, hooks)
                arkts.recheckSubtree(prog.ast)
                arkts.Tracer.popContext()
            } catch(e) {
                console.trace(e)
                throw e
            }
        },
        clean() {
            arkts.Tracer.pushContext('memo-plugin')
            arkts.trace(() => "Clean", true)
            arkts.Tracer.popContext()
            pluginContext = new arkts.PluginContextImpl()
        }
    }
}
