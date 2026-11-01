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

#ifndef FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_LOG_WRAPPER_WITHOUT_INSTANCE_H
#define FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_LOG_WRAPPER_WITHOUT_INSTANCE_H

#include "base/log/log_wrapper.h"

// Opt-in logging without the instance ID, generated ID, or generation reason.
#ifdef ACE_INSTANCE_LOG
#define ACE_FMT_PREFIX_WITHOUT_INSTANCE "[%{public}s(%{public}d)] "
#else
#define ACE_FMT_PREFIX_WITHOUT_INSTANCE "[%{private}s(%{private}d)] "
#endif

#if defined(USE_HILOG) && defined(IS_RELEASE_VERSION)
#define PRINT_LOG_WITHOUT_INSTANCE(level, tag, fmt, ...) \
    HILOG_IMPL(LOG_CORE, LOG_##level, (tag + ACE_DOMAIN), (OHOS::Ace::g_DOMAIN_CONTENTS_MAP.at(tag)), \
        fmt, ##__VA_ARGS__)
#elif defined(USE_HILOG)
#define PRINT_LOG_WITHOUT_INSTANCE(level, tag, fmt, ...) \
    HILOG_IMPL(LOG_CORE, LOG_##level, (tag + ACE_DOMAIN), (OHOS::Ace::g_DOMAIN_CONTENTS_MAP.at(tag)), \
        ACE_FMT_PREFIX_WITHOUT_INSTANCE fmt, OHOS::Ace::LogWrapper::GetBriefFileName(__FILE__), \
        __LINE__, ##__VA_ARGS__)
#else
#define PRINT_LOG_WITHOUT_INSTANCE(level, tag, fmt, ...) \
    do { \
        if (OHOS::Ace::LogWrapper::JudgeLevel(OHOS::Ace::LogLevel::level)) { \
            OHOS::Ace::LogWrapper::PrintLog(OHOS::Ace::LogDomain::FRAMEWORK, OHOS::Ace::LogLevel::level, tag, \
                ACE_FMT_PREFIX_WITHOUT_INSTANCE fmt, OHOS::Ace::LogWrapper::GetBriefFileName(__FILE__), \
                __LINE__, ##__VA_ARGS__); \
        } \
    } while (0)
#endif

#if defined(STRIP_RELEASE_LOG)
#define TAG_LOGD_WITHOUT_INSTANCE(tag, fmt, ...) ((void)0)
#else
#define TAG_LOGD_WITHOUT_INSTANCE(tag, fmt, ...) \
    PRINT_LOG_WITHOUT_INSTANCE(DEBUG, tag, fmt, ##__VA_ARGS__)
#endif
#define TAG_LOGI_WITHOUT_INSTANCE(tag, fmt, ...) \
    PRINT_LOG_WITHOUT_INSTANCE(INFO, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGW_WITHOUT_INSTANCE(tag, fmt, ...) \
    PRINT_LOG_WITHOUT_INSTANCE(WARN, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGE_WITHOUT_INSTANCE(tag, fmt, ...) \
    PRINT_LOG_WITHOUT_INSTANCE(ERROR, tag, fmt, ##__VA_ARGS__)
#define TAG_LOGF_WITHOUT_INSTANCE(tag, fmt, ...) \
    PRINT_LOG_WITHOUT_INSTANCE(FATAL, tag, fmt, ##__VA_ARGS__)

#endif // FOUNDATION_ACE_FRAMEWORKS_BASE_LOG_LOG_WRAPPER_WITHOUT_INSTANCE_H
