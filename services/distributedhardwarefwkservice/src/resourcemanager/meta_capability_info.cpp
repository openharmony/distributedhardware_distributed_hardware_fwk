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

#include "meta_capability_info.h"

#include <string>

#include "cJSON.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "MetaCapabilityInfo"


std::string MetaCapabilityInfo::GetSinkVersion() const
{
    return sinkVersion_;
}

void MetaCapabilityInfo::SetSinkVersion(const std::string &sinkVersion)
{
    this->sinkVersion_ = sinkVersion;
}

int32_t MetaCapabilityInfo::FromJsonString(const std::string &jsonStr)
{
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
    if (strcmp(this->GetSinkVersion().c_str(), metaCapInfo.GetSinkVersion().c_str()) != 0) {
        DHLOGE("sinkVersion is not equal");
        return false;
    }
    return true;
}

void ToJson(cJSON *jsonObject, const MetaCapabilityInfo &metaCapInfo)
{
    cJSON_AddStringToObject(jsonObject, DH_ID.c_str(), metaCapInfo.GetDHId().c_str());
    cJSON_AddStringToObject(jsonObject, DEV_ID.c_str(), metaCapInfo.GetDeviceId().c_str());
    cJSON_AddStringToObject(jsonObject, DEV_NAME.c_str(), metaCapInfo.GetDeviceName().c_str());
    cJSON_AddNumberToObject(jsonObject, DEV_TYPE.c_str(), (double)metaCapInfo.GetDeviceType());
    cJSON_AddNumberToObject(jsonObject, DH_TYPE.c_str(), (double)metaCapInfo.GetDHType());
    cJSON_AddStringToObject(jsonObject, DH_ATTRS.c_str(), metaCapInfo.GetDHAttrs().c_str());
    cJSON_AddStringToObject(jsonObject, DH_SUBTYPE.c_str(), metaCapInfo.GetDHSubtype().c_str());
    cJSON_AddStringToObject(jsonObject, SINK_VER.c_str(), metaCapInfo.GetSinkVersion().c_str());
}

void FromJson(const cJSON *jsonObject, MetaCapabilityInfo &metaCapInfo)
{
    if (!IsString(jsonObject, DH_ID)) {
        DHLOGE("DH_ID is invalid!");
        return;
    }
    metaCapInfo.SetDHId(cJSON_GetObjectItem(jsonObject, DH_ID.c_str())->valuestring);

    if (!IsString(jsonObject, DEV_ID)) {
        DHLOGE("DEV_ID is invalid!");
        return;
    }
    metaCapInfo.SetDeviceId(cJSON_GetObjectItem(jsonObject, DEV_ID.c_str())->valuestring);

    if (!IsString(jsonObject, DEV_NAME)) {
        DHLOGE("DEV_NAME is invalid!");
        return;
    }
    metaCapInfo.SetDeviceName(cJSON_GetObjectItem(jsonObject, DEV_NAME.c_str())->valuestring);

    if (!IsUInt16(jsonObject, DEV_TYPE)) {
        DHLOGE("DEV_TYPE is invalid!");
        return;
    }
    metaCapInfo.SetDeviceType((uint16_t)cJSON_GetObjectItem(jsonObject, DEV_TYPE.c_str())->valuedouble);

    if (!IsUInt32(jsonObject, DH_TYPE)) {
        DHLOGE("DH_TYPE is invalid!");
        return;
    }
    metaCapInfo.SetDHType((DHType)cJSON_GetObjectItem(jsonObject, DH_TYPE.c_str())->valuedouble);

    if (!IsString(jsonObject, DH_ATTRS)) {
        DHLOGE("DH_ATTRS is invalid!");
        return;
    }
    metaCapInfo.SetDHAttrs(cJSON_GetObjectItem(jsonObject, DH_ATTRS.c_str())->valuestring);

    if (!IsString(jsonObject, DH_SUBTYPE)) {
        DHLOGE("DH_SUBTYPE is invalid!");
        return;
    }
    metaCapInfo.SetDHSubtype(cJSON_GetObjectItem(jsonObject, DH_SUBTYPE.c_str())->valuestring);

    if (!IsString(jsonObject, SINK_VER)) {
        DHLOGE("SINK_VER is invalid!");
        return;
    }
    metaCapInfo.SetSinkVersion(cJSON_GetObjectItem(jsonObject, SINK_VER.c_str())->valuestring);
}
} // namespace DistributedHardware
} // namespace OHOS
