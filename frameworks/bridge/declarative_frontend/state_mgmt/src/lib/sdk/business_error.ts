/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 * BusinessError & errorCode
 * 
 */
class BusinessError extends Error {
    code: string;
    constructor(code: string, message: string) {
        super(message);
        this.code = code;
        this.name = 'BusinessError';
    }
}

const ADD_MONITOR_FAIL_TARGET_ILLEGAL: string = '130000';
const ADD_MONITOR_FAIL_PATH_ILLEGAL: string = '130001';
const ADD_MONITOR_FAIL_FUNC_ILLEGAL: string = '130002';