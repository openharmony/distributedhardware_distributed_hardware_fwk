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

#include "capability_utils.h"

#include "capability_info.h"
#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "CapabilityUtils"

int32_t CapabilityUtils::GetCapabilityByValue(const std::string &value, std::shared_ptr<CapabilityInfo> &capPtr)
{
    if (capPtr == nullptr) {
        capPtr = std::make_shared<CapabilityInfo>();
    }
    return capPtr->FromJsonString(value);
}

std::string CapabilityUtils::GetCapabilityKey(const std::string &deviceId, const std::string &dhId)
{
    return deviceId + RESOURCE_SEPARATOR + dhId;
}

bool CapabilityUtils::IsCapKeyMatchDeviceId(const std::string &key, const std::string &deviceId)
{
    std::size_t separatorPos = key.find(RESOURCE_SEPARATOR);
    if (separatorPos == std::string::npos) {
        return false;
    }
    std::string keyDevId = key.substr(0, separatorPos);
    return keyDevId.compare(deviceId) == 0;
}

bool CapabilityUtils::IsCapInfoJsonEqual(const std::string &firstData, const std::string &lastData)
{
    cJSON *firstJson = cJSON_Parse(firstData.c_str());
    if (firstJson == NULL) {
        DHLOGE("firstData parse failed");
        return false;
    }
    CapabilityInfo firstCapInfo;
    FromJson(firstJson, firstCapInfo);
    cJSON *lastJson = cJSON_Parse(lastData.c_str());
    if (lastJson == NULL) {
        DHLOGE("lastData parse failed");
        cJSON_Delete(firstJson);
        return false;
    }
    CapabilityInfo lastCapInfo;
    FromJson(lastJson, lastCapInfo);
    cJSON_Delete(firstJson);
    cJSON_Delete(lastJson);
    return firstCapInfo.Compare(lastCapInfo);
}
} // namespace DistributedHardware
} // namespace OHOS
