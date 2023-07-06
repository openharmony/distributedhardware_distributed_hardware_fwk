/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <string>

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    DH_LOG_DEBUG,
    DH_LOG_INFO,
    DH_LOG_WARN,
    DH_LOG_ERROR,
} DHLogLevel;

void AVTransLog(DHLogLevel logLevel, const char *fmt, ...);

#define AVTRANS_LOGD(fmt, ...) AVTransLog(DH_LOG_DEBUG, \
    (std::string("[") + DH_LOG_TAG + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)

#define AVTRANS_LOGI(fmt, ...) AVTransLog(DH_LOG_INFO, \
    (std::string("[") + DH_LOG_TAG + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)

#define AVTRANS_LOGW(fmt, ...) AVTransLog(DH_LOG_WARN, \
    (std::string("[") + DH_LOG_TAG + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)

#define AVTRANS_LOGE(fmt, ...) AVTransLog(DH_LOG_ERROR, \
    (std::string("[") + DH_LOG_TAG + "][" + __FUNCTION__ + "]:" + fmt).c_str(), ##__VA_ARGS__)

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