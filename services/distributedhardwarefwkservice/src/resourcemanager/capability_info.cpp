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
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

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

std::string CapabilityInfo::GetDHSubtype() const
{
    return dhSubtype_;
}

void CapabilityInfo::SetDHSubtype(const std::string &dhSubtype)
{
    this->dhSubtype_ = dhSubtype;
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
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        DHLOGE("jsonStr parse failed");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }

    FromJson(jsonObj, *this);
    return DH_FWK_SUCCESS;
}

std::string CapabilityInfo::ToJsonString()
{
    nlohmann::json jsonObj;
    ToJson(jsonObj, *this);
    return jsonObj.dump();
}

bool CapabilityInfo::Compare(const CapabilityInfo& capInfo)
{
    if (strcmp(this->deviceId_.c_str(), capInfo.deviceId_.c_str()) != 0) {
        DHLOGE("deviceId is not equal");
        return false;
    }
    if (strcmp(this->dhId_.c_str(), capInfo.dhId_.c_str()) != 0) {
        DHLOGE("dhId is not equal");
        return false;
    }
    if (strcmp(this->deviceName_.c_str(), capInfo.deviceName_.c_str()) != 0) {
        DHLOGE("deviceName is not equal");
        return false;
    }
    if (this->deviceType_ != capInfo.deviceType_) {
        DHLOGE("deviceType is not equal");
        return false;
    }
    if (this->dhType_ != capInfo.dhType_) {
        DHLOGE("dhType is not equal");
        return false;
    }
    if (strcmp(this->dhAttrs_.c_str(), capInfo.dhAttrs_.c_str()) != 0) {
        DHLOGE("dhAttrs is not equal");
        return false;
    }
    if (strcmp(this->dhSubtype_.c_str(), capInfo.dhSubtype_.c_str()) != 0) {
        DHLOGE("dhSubtype is not equal");
        return false;
    }
    return true;
}

void ToJson(nlohmann::json &jsonObject, const CapabilityInfo &capability)
{
    jsonObject[DH_ID] = capability.GetDHId();
    jsonObject[DEV_ID] = capability.GetDeviceId();
    jsonObject[DEV_NAME] = capability.GetDeviceName();
    jsonObject[DEV_TYPE] = capability.GetDeviceType();
    jsonObject[DH_TYPE] = capability.GetDHType();
    jsonObject[DH_ATTRS] = capability.GetDHAttrs();
    jsonObject[DH_SUBTYPE] = capability.GetDHSubtype();
}

void FromJson(const nlohmann::json &jsonObject, CapabilityInfo &capability)
{
    if (!IsString(jsonObject, DH_ID)) {
        DHLOGE("DH_ID is invalid!");
        return;
    }
    capability.SetDHId(jsonObject.at(DH_ID).get<std::string>());
    if (!IsString(jsonObject, DEV_ID)) {
        DHLOGE("DEV_ID is invalid!");
        return;
    }
    capability.SetDeviceId(jsonObject.at(DEV_ID).get<std::string>());
    if (!IsString(jsonObject, DEV_NAME)) {
        DHLOGE("DEV_NAME is invalid!");
        return;
    }
    capability.SetDeviceName(jsonObject.at(DEV_NAME).get<std::string>());
    if (!IsUInt16(jsonObject, DEV_TYPE)) {
        DHLOGE("DEV_TYPE is invalid!");
        return;
    }
    capability.SetDeviceType(jsonObject.at(DEV_TYPE).get<uint16_t>());
    if (!IsUInt32(jsonObject, DH_TYPE)) {
        DHLOGE("DH_TYPE is invalid!");
        return;
    }
    capability.SetDHType(jsonObject.at(DH_TYPE).get<DHType>());
    if (!IsString(jsonObject, DH_ATTRS)) {
        DHLOGE("DH_ATTRS is invalid!");
        return;
    }
    capability.SetDHAttrs(jsonObject.at(DH_ATTRS).get<std::string>());
    if (!IsString(jsonObject, DH_SUBTYPE)) {
        DHLOGE("DH_SUBTYPE is invalid!");
        return;
    }
    capability.SetDHSubtype(jsonObject.at(DH_SUBTYPE).get<std::string>());
}
} // namespace DistributedHardware
} // namespace OHOS
