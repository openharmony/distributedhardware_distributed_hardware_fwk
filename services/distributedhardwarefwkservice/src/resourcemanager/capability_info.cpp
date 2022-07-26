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

#include "capability_info.h"

#include <string>

#include "nlohmann/json.hpp"

#include "anonymous_string.h"
#include "constants.h"
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "CapabilityInfo"

std::string CapabilityInfo::GetDHId() const
{
    return dhId_;
}

void CapabilityInfo::SetDHId(const std::string &dhId)
{
    this->dhId_ = dhId;
}

std::string CapabilityInfo::GetDeviceId() const
{
    return deviceId_;
}

void CapabilityInfo::SetDeviceId(const std::string &deviceId)
{
    this->deviceId_ = deviceId;
}

std::string CapabilityInfo::GetDeviceName() const
{
    return deviceName_;
}

void CapabilityInfo::SetDeviceName(const std::string &deviceName)
{
    this->deviceName_ = deviceName;
}

uint16_t CapabilityInfo::GetDeviceType() const
{
    return deviceType_;
}

void CapabilityInfo::SetDeviceType(uint16_t deviceType)
{
    this->deviceType_ = deviceType;
}

DHType CapabilityInfo::GetDHType() const
{
    return dhType_;
}

void CapabilityInfo::SetDHType(const DHType dhType)
{
    this->dhType_ = dhType;
}

std::string CapabilityInfo::GetDHAttrs() const
{
    return dhAttrs_;
}

void CapabilityInfo::SetDHAttrs(const std::string &dhAttrs)
{
    this->dhAttrs_ = dhAttrs;
}

std::string CapabilityInfo::GetKey() const
{
    std::string kvStoreKey;
    kvStoreKey.append(deviceId_);
    kvStoreKey.append(RESOURCE_SEPARATOR);
    kvStoreKey.append(dhId_);
    return kvStoreKey;
}

std::string CapabilityInfo::GetAnonymousKey() const
{
    std::string kvStoreKey;
    kvStoreKey.append(GetAnonyString(deviceId_));
    kvStoreKey.append(RESOURCE_SEPARATOR);
    kvStoreKey.append(GetAnonyString(dhId_));
    return kvStoreKey;
}

int32_t CapabilityInfo::FromJsonString(const std::string &jsonStr)
{
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr);
    FromJson(jsonObj, *this);
    return DH_FWK_SUCCESS;
}

std::string CapabilityInfo::ToJsonString()
{
    nlohmann::json jsonObj;
    ToJson(jsonObj, *this);
    return jsonObj.dump();
}

void ToJson(nlohmann::json &jsonObject, const CapabilityInfo &capability)
{
    jsonObject[DH_ID] = capability.GetDHId();
    jsonObject[DEV_ID] = capability.GetDeviceId();
    jsonObject[DEV_NAME] = capability.GetDeviceName();
    jsonObject[DEV_TYPE] = capability.GetDeviceType();
    jsonObject[DH_TYPE] = capability.GetDHType();
    jsonObject[DH_ATTRS] = capability.GetDHAttrs();
}

void FromJson(const nlohmann::json &jsonObject, CapabilityInfo &capability)
{
    if (jsonObject.find(DH_ID) != jsonObject.end()) {
        capability.SetDHId(jsonObject.at(DH_ID).get<std::string>());
    }
    if (jsonObject.find(DEV_ID) != jsonObject.end()) {
        capability.SetDeviceId(jsonObject.at(DEV_ID).get<std::string>());
    }
    if (jsonObject.find(DEV_NAME) != jsonObject.end()) {
        capability.SetDeviceName(jsonObject.at(DEV_NAME).get<std::string>());
    }
    if (jsonObject.find(DEV_TYPE) != jsonObject.end()) {
        capability.SetDeviceType(jsonObject.at(DEV_TYPE).get<uint16_t>());
    }
    if (jsonObject.find(DH_TYPE) != jsonObject.end()) {
        capability.SetDHType(jsonObject.at(DH_TYPE).get<DHType>());
    }
    if (jsonObject.find(DH_ATTRS) != jsonObject.end()) {
        capability.SetDHAttrs(jsonObject.at(DH_ATTRS).get<std::string>());
    }
}
} // namespace DistributedHardware
} // namespace OHOS
