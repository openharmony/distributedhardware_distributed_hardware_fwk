/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "meta_capability_info.h"

#include <string>

#include "cJSON.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "version_info.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "MetaCapabilityInfo"

std::string MetaCapabilityInfo::GetUdidHash() const
{
    return udidHash_;
}

void MetaCapabilityInfo::SetUdidHash(const std::string &udidHash)
{
    this->udidHash_ = udidHash;
}

std::string MetaCapabilityInfo::GetSinkVersion() const
{
    return compVersion_.sinkVersion;
}

void MetaCapabilityInfo::SetSinkVersion(const std::string &sinkVersion)
{
    this->compVersion_.sinkVersion = sinkVersion;
}

CompVersion& MetaCapabilityInfo::GetCompVersion()
{
    return compVersion_;
}

CompVersion MetaCapabilityInfo::GetCompVersion() const
{
    return compVersion_;
}

void MetaCapabilityInfo::SetCompVersion(const CompVersion &compVersion)
{
    this->compVersion_ = compVersion;
}

int32_t MetaCapabilityInfo::FromJsonString(const std::string &jsonStr)
{
    if (!IsJsonLengthValid(jsonStr)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    cJSON *jsonObj = cJSON_Parse(jsonStr.c_str());
    if (jsonObj == NULL) {
        DHLOGE("jsonStr parse failed");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }

    FromJson(jsonObj, *this);
    cJSON_Delete(jsonObj);
    return DH_FWK_SUCCESS;
}

std::string MetaCapabilityInfo::ToJsonString()
{
    cJSON *jsonObj = cJSON_CreateObject();
    if (jsonObj == NULL) {
        DHLOGE("Failed to create cJSON object.");
        return "";
    }
    ToJson(jsonObj, *this);
    char *cjson = cJSON_PrintUnformatted(jsonObj);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObj);
        return "";
    }
    std::string jsonString(cjson);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    return jsonString;
}

bool MetaCapabilityInfo::Compare(const MetaCapabilityInfo& metaCapInfo)
{
    if (strcmp(this->GetDeviceId().c_str(), metaCapInfo.GetDeviceId().c_str()) != 0) {
        DHLOGE("deviceId is not equal");
        return false;
    }
    if (strcmp(this->GetDHId().c_str(), metaCapInfo.GetDHId().c_str()) != 0) {
        DHLOGE("dhId is not equal");
        return false;
    }
    if (strcmp(this->GetDeviceName().c_str(), metaCapInfo.GetDeviceName().c_str()) != 0) {
        DHLOGE("deviceName is not equal");
        return false;
    }
    if (this->GetDeviceType() != metaCapInfo.GetDeviceType()) {
        DHLOGE("deviceType is not equal");
        return false;
    }
    if (this->GetDHType() != metaCapInfo.GetDHType()) {
        DHLOGE("dhType is not equal");
        return false;
    }
    if (strcmp(this->GetDHAttrs().c_str(), metaCapInfo.GetDHAttrs().c_str()) != 0) {
        DHLOGE("dhAttrs is not equal");
        return false;
    }
    if (strcmp(this->GetDHSubtype().c_str(), metaCapInfo.GetDHSubtype().c_str()) != 0) {
        DHLOGE("dhSubtype is not equal");
        return false;
    }
    if (strcmp(this->GetUdidHash().c_str(), metaCapInfo.GetUdidHash().c_str()) != 0) {
        DHLOGE("udidHash is not equal");
        return false;
    }
    if (strcmp(this->GetSinkVersion().c_str(), metaCapInfo.GetSinkVersion().c_str()) != 0) {
        DHLOGE("sinkVersion is not equal");
        return false;
    }
    return true;
}

std::string MetaCapabilityInfo::GetKey() const
{
    std::string kvStoreKey;
    kvStoreKey.append(udidHash_);
    kvStoreKey.append(RESOURCE_SEPARATOR);
    kvStoreKey.append(this->GetDHId());
    return kvStoreKey;
}

std::string MetaCapabilityInfo::GetAnonymousKey() const
{
    std::string kvStoreKey;
    kvStoreKey.append(GetAnonyString(udidHash_));
    kvStoreKey.append(RESOURCE_SEPARATOR);
    kvStoreKey.append(GetAnonyString(this->GetDHId()));
    return kvStoreKey;
}

void ToJson(cJSON *jsonObject, const MetaCapabilityInfo &metaCapInfo)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON_AddStringToObject(jsonObject, DH_ID, metaCapInfo.GetDHId().c_str());
    cJSON_AddStringToObject(jsonObject, DEV_ID, metaCapInfo.GetDeviceId().c_str());
    cJSON_AddStringToObject(jsonObject, DEV_NAME, metaCapInfo.GetDeviceName().c_str());
    cJSON_AddNumberToObject(jsonObject, DEV_TYPE, static_cast<uint16_t>(metaCapInfo.GetDeviceType()));
    cJSON_AddNumberToObject(jsonObject, DH_TYPE, static_cast<uint32_t>(metaCapInfo.GetDHType()));
    cJSON_AddStringToObject(jsonObject, DH_ATTRS, metaCapInfo.GetDHAttrs().c_str());
    cJSON_AddStringToObject(jsonObject, DH_SUBTYPE, metaCapInfo.GetDHSubtype().c_str());
    cJSON_AddStringToObject(jsonObject, DEV_UDID_HASH, metaCapInfo.GetUdidHash().c_str());
    cJSON_AddStringToObject(jsonObject, SINK_VER, metaCapInfo.GetSinkVersion().c_str());
    cJSON *jsonObjCompVersion = cJSON_CreateObject();
    if (jsonObjCompVersion == NULL) {
        DHLOGE("Failed to create cJSON object.");
        return;
    }
    ToJson(jsonObjCompVersion, metaCapInfo.GetCompVersion());
    cJSON_AddItemToObject(jsonObject, COMP_VER, jsonObjCompVersion);
}

void FromJson(const cJSON *jsonObject, MetaCapabilityInfo &metaCapInfo)
{
    cJSON *dhIdJson = cJSON_GetObjectItem(jsonObject, DH_ID);
    if (!IsString(dhIdJson)) {
        DHLOGE("DH_ID is invalid!");
        return;
    }
    metaCapInfo.SetDHId(dhIdJson->valuestring);

    cJSON *devIdJson = cJSON_GetObjectItem(jsonObject, DEV_ID);
    if (!IsString(devIdJson)) {
        DHLOGE("DEV_ID is invalid!");
        return;
    }
    metaCapInfo.SetDeviceId(devIdJson->valuestring);

    cJSON *devNameJson = cJSON_GetObjectItem(jsonObject, DEV_NAME);
    if (!IsString(devNameJson)) {
        DHLOGE("DEV_NAME is invalid!");
        return;
    }
    metaCapInfo.SetDeviceName(devNameJson->valuestring);

    cJSON *devTypeJson = cJSON_GetObjectItem(jsonObject, DEV_TYPE);
    if (!IsUInt16(devTypeJson)) {
        DHLOGE("DEV_TYPE is invalid!");
        return;
    }
    metaCapInfo.SetDeviceType(static_cast<uint16_t>(devTypeJson->valueint));

    cJSON *dhTypeJson = cJSON_GetObjectItem(jsonObject, DH_TYPE);
    if (!IsUInt32(dhTypeJson)) {
        DHLOGE("DH_TYPE is invalid!");
        return;
    }
    metaCapInfo.SetDHType((DHType)dhTypeJson->valueint);

    cJSON *dhAttrsObj = cJSON_GetObjectItem(jsonObject, DH_ATTRS);
    if (!IsString(dhAttrsObj)) {
        DHLOGE("DH_ATTRS is invalid!");
        return;
    }
    metaCapInfo.SetDHAttrs(dhAttrsObj->valuestring);

    FromJsonContinue(jsonObject, metaCapInfo);
}

void FromJsonContinue(const cJSON *jsonObject, MetaCapabilityInfo &metaCapInfo)
{
    cJSON *dhSubtypeJson = cJSON_GetObjectItem(jsonObject, DH_SUBTYPE);
    if (!IsString(dhSubtypeJson)) {
        DHLOGE("DH_SUBTYPE is invalid!");
        return;
    }
    metaCapInfo.SetDHSubtype(dhSubtypeJson->valuestring);

    cJSON *udidHashJson = cJSON_GetObjectItem(jsonObject, DEV_UDID_HASH);
    if (!IsString(udidHashJson)) {
        DHLOGE("DEV_UDID_HASH is invalid!");
        return;
    }
    metaCapInfo.SetUdidHash(udidHashJson->valuestring);

    cJSON *sinkVerJson = cJSON_GetObjectItem(jsonObject, SINK_VER);
    if (!IsString(sinkVerJson)) {
        DHLOGE("SINK_VER is invalid!");
        return;
    }
    metaCapInfo.SetSinkVersion(sinkVerJson->valuestring);

    cJSON *compVersionJson = cJSON_GetObjectItem(jsonObject, COMP_VER);
    if (compVersionJson == nullptr) {
        DHLOGE("CompVersion is invalid!");
        return;
    }
    FromJson(compVersionJson, metaCapInfo.GetCompVersion());
}
} // namespace DistributedHardware
} // namespace OHOS
