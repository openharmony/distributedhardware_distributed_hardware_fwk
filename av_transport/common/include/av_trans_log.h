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

#ifndef OHOS_AV_TRANSPORT_LOG_H
#define OHOS_AV_TRANSPORT_LOG_H

#include <cinttypes>
#include "hilog/log.h"
#include <inttypes.h>
#include <string>

namespace OHOS {
namespace DistributedHardware {
#undef LOG_TAG
#define LOG_TAG "DAVTRANS"

#define AVTRANS_LOGD(fmt, ...) HILOG_DEBUG(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

#define AVTRANS_LOGI(fmt, ...) HILOG_INFO(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

#define AVTRANS_LOGW(fmt, ...) HILOG_WARN(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

#define AVTRANS_LOGE(fmt, ...) HILOG_ERROR(LOG_CORE, \
    "[%{public}s][%{public}s]:" fmt, DH_LOG_TAG, __FUNCTION__, ##__VA_ARGS__)

std::string GetAnonyString(const std::string &value);
std::string GetAnonyInt32(const int32_t value);

#ifndef TRUE_RETURN
#define TRUE_RETURN(exec, fmt, args...)                                                                       \
    do {                                                                                                      \
        bool retCode = (exec);                                                                                \
        if (retCode) {                                                                                        \
            AVTRANS_LOGE(fmt, ##args);                                                                        \
            return;                                                                                           \
        }                                                                                                     \
    } while (0)
#endif

#ifndef TRUE_RETURN_V
#define TRUE_RETURN_V(exec, ret)                                                                              \
    do {                                                                                                      \
        bool retCode = (exec);                                                                                \
        if (retCode) {                                                                                        \
            return ret;                                                                                       \
        }                                                                                                     \
    } while (0)
#endif

#ifndef TRUE_RETURN_V_MSG_E
#define TRUE_RETURN_V_MSG_E(exec, ret, fmt, args...)                                                          \
    do {                                                                                                      \
        bool retCode = (exec);                                                                                \
        if (retCode) {                                                                                        \
            AVTRANS_LOGE(fmt, ##args);                                                                        \
            return ret;                                                                                       \
        }                                                                                                     \
    } while (0)
#endif

#ifndef TRUE_RETURN_V_MSG_D
#define TRUE_RETURN_V_MSG_D(exec, ret, fmt, args...)                                                          \
    do {                                                                                                      \
        bool retCode = (exec);                                                                                \
        if (retCode) {                                                                                        \
            AVTRANS_LOGD(fmt, ##args);                                                                        \
            return ret;                                                                                       \
        }                                                                                                     \
    } while (0)
#endif

#ifndef TRUE_LOG_MSG
#define TRUE_LOG_MSG(exec, fmt, args...)                                                                      \
    do {                                                                                                      \
        bool retCode = (exec);                                                                                \
        if (retCode) {                                                                                        \
            AVTRANS_LOGE(fmt, ##args);                                                                        \
        }                                                                                                     \
    } while (0)
#endif
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANSPORT_LOG_H