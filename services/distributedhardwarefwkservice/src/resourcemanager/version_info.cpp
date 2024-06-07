/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "version_info.h"

#include <string>

#include "cJSON.h"

#include "anonymous_string.h"
#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "VersionInfo"

int32_t VersionInfo::FromJsonString(const std::string &jsonStr)
{
    cJSON *jsonObj = cJSON_Parse(jsonStr.c_str());
    if (jsonObj == NULL) {
        DHLOGE("json string parse failed");
        return ERR_DH_FWK_JSON_PARSE_FAILED;
    }
    FromJson(jsonObj, *this);
    cJSON_Delete(jsonObj);
    return DH_FWK_SUCCESS;
}

std::string VersionInfo::ToJsonString() const
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
    std::string result(cjson);
    cJSON_free(cjson);
    cJSON_Delete(jsonObj);
    return result;
}

void ToJson(cJSON *jsonObject, const VersionInfo &versionInfo)
{
    cJSON_AddStringToObject(jsonObject, DEV_ID.c_str(), versionInfo.deviceId.c_str());
    cJSON_AddStringToObject(jsonObject, DH_VER.c_str(), versionInfo.dhVersion.c_str());

    cJSON *compVers = cJSON_CreateArray();
    if (compVers == NULL) {
        DHLOGE("Failed to create cJSON array.");
        return;
    }
    for (const auto &compVersion : versionInfo.compVersions) {
        cJSON *compVer = cJSON_CreateObject();
        if (compVer == NULL) {
            cJSON_Delete(compVers);
            DHLOGE("Failed to create cJSON object.");
            return;
        }
        cJSON_AddStringToObject(compVer, NAME.c_str(), compVersion.second.name.c_str());
        cJSON_AddNumberToObject(compVer, TYPE.c_str(), (double)compVersion.second.dhType);
        cJSON_AddStringToObject(compVer, HANDLER.c_str(), compVersion.second.handlerVersion.c_str());
        cJSON_AddStringToObject(compVer, SOURCE_VER.c_str(), compVersion.second.sourceVersion.c_str());
        cJSON_AddStringToObject(compVer, SINK_VER.c_str(), compVersion.second.sinkVersion.c_str());
        cJSON_AddItemToArray(compVers, compVer);
    }
    cJSON_AddItemToObject(jsonObject, COMP_VER.c_str(), compVers);
}

void FromJson(const cJSON *jsonObject, CompVersion &compVer)
{
    if (IsString(jsonObject, NAME)) {
        compVer.name = cJSON_GetObjectItem(jsonObject, NAME.c_str())->valuestring;
    }
    if (IsUInt32(jsonObject, TYPE) &&
        (DHType)cJSON_GetObjectItem(jsonObject, TYPE.c_str())->valuedouble <= DHType::MAX_DH) {
        compVer.dhType = (DHType)(cJSON_GetObjectItem(jsonObject, TYPE.c_str())->valuedouble);
    }
    if (IsString(jsonObject, HANDLER)) {
        compVer.handlerVersion = cJSON_GetObjectItem(jsonObject, HANDLER.c_str())->valuestring;
    }
    if (IsString(jsonObject, SOURCE_VER)) {
        compVer.sourceVersion = cJSON_GetObjectItem(jsonObject, SOURCE_VER.c_str())->valuestring;
    }
    if (IsString(jsonObject, SINK_VER)) {
        compVer.sinkVersion = cJSON_GetObjectItem(jsonObject, SINK_VER.c_str())->valuestring;
    }
}

void FromJson(const cJSON *jsonObject, VersionInfo &versionInfo)
{
    if (IsString(jsonObject, DEV_ID)) {
        versionInfo.deviceId = cJSON_GetObjectItem(jsonObject, DEV_ID.c_str())->valuestring;
    }

    if (IsString(jsonObject, DH_VER)) {
        versionInfo.dhVersion = cJSON_GetObjectItem(jsonObject, DH_VER.c_str())->valuestring;
    }

    const cJSON *compVer = cJSON_GetObjectItem(jsonObject, COMP_VER.c_str());
    if (compVer != NULL) {
        cJSON *compVerObj;
        cJSON_ArrayForEach(compVerObj, compVer) {
            CompVersion compVerValue;
            FromJson(compVerObj, compVerValue);
            versionInfo.compVersions.insert(std::pair<DHType, CompVersion>(compVerValue.dhType, compVerValue));
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
