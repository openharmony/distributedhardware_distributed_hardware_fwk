/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_UTILS_H
#define OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_UTILS_H

#include <cstdint>
#include <memory>
#include <string>

#include "capability_info.h"
#include "distributed_hardware_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "CapabilityUtils"

namespace OHOS {
namespace DistributedHardware {
class CapabilityInfo;
enum class CapabilityInfoFilter : uint32_t {
    FILTER_DH_ID = 0,
    FILTER_DEVICE_ID = 1,
    FILTER_DEVICE_NAME = 2,
    FILTER_DEVICE_TYPE = 3,
    FILTER_DH_TYPE = 4,
    FILTER_DH_ATTRS = 5
};

template<typename T>
int32_t GetCapabilityByValue(const std::string &value, std::shared_ptr<T> &capPtr)
{
    if (capPtr == nullptr) {
        capPtr = std::make_shared<T>();
    }
    return capPtr->FromJsonString(value);
}

std::string GetCapabilityKey(const std::string &deviceId, const std::string &dhId);
bool IsCapKeyMatchDeviceId(const std::string &key, const std::string &deviceId);

template<typename T>
bool IsCapInfoJsonEqual(const std::string &firstData, const std::string &lastData)
{
    cJSON *firstJson = cJSON_Parse(firstData.c_str());
    if (firstJson == NULL) {
        DHLOGE("firstData parse failed");
        return false;
    }
    T firstCapInfo;
    FromJson(firstJson, firstCapInfo);
    cJSON *lastJson = cJSON_Parse(lastData.c_str());
    if (lastJson == NULL) {
        DHLOGE("lastData parse failed");
        cJSON_Delete(firstJson);
        return false;
    }
    T lastCapInfo;
    FromJson(lastJson, lastCapInfo);
    cJSON_Delete(firstJson);
    cJSON_Delete(lastJson);
    return firstCapInfo.Compare(lastCapInfo);
}
} // namespace DistributedHardware
} // namespace OHOS
#endif
