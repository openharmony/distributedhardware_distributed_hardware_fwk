/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
    if (!IsJsonLengthValid(jsonStr)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
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

void ToJson(cJSON *jsonObject, const CompVersion &compVer)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON_AddStringToObject(jsonObject, NAME.c_str(), compVer.name.c_str());
    cJSON_AddNumberToObject(jsonObject, TYPE.c_str(), (double)compVer.dhType);
    cJSON_AddStringToObject(jsonObject, HANDLER.c_str(), compVer.handlerVersion.c_str());
    cJSON_AddStringToObject(jsonObject, SOURCE_VER.c_str(), compVer.sourceVersion.c_str());
    cJSON_AddStringToObject(jsonObject, SINK_VER.c_str(), compVer.sinkVersion.c_str());
    if (compVer.haveFeature) {
        cJSON *sourceFeatureFilters = cJSON_CreateArray();
        if (sourceFeatureFilters == NULL) {
            DHLOGE("Failed to create cJSON object.");
            return;
        }
        cJSON *sinkSupportedFeatures = cJSON_CreateArray();
        if (sinkSupportedFeatures == NULL) {
            cJSON_Delete(sourceFeatureFilters);
            DHLOGE("Failed to create cJSON object.");
            return;
        }
        for (const auto &filter : compVer.sourceFeatureFilters) {
            cJSON_AddItemToArray(sourceFeatureFilters, cJSON_CreateString(filter.c_str()));
        }
        cJSON_AddItemToObject(jsonObject, SOURCE_FEATURE_FILTER.c_str(), sourceFeatureFilters);
        for (const auto &feature : compVer.sinkSupportedFeatures) {
            cJSON_AddItemToArray(sinkSupportedFeatures, cJSON_CreateString(feature.c_str()));
        }
        cJSON_AddItemToObject(jsonObject, SINK_SUPPORTED_FEATURE.c_str(), sinkSupportedFeatures);
    }
}

void ToJson(cJSON *jsonObject, const VersionInfo &versionInfo)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
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
        ToJson(compVer, compVersion.second);
        cJSON_AddItemToArray(compVers, compVer);
    }
    cJSON_AddItemToObject(jsonObject, COMP_VER.c_str(), compVers);
}

void FromJson(const cJSON *jsonObject, CompVersion &compVer)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *nameJson = cJSON_GetObjectItem(jsonObject, NAME.c_str());
    if (IsString(nameJson)) {
        compVer.name = nameJson->valuestring;
    }
    cJSON *typeJson = cJSON_GetObjectItem(jsonObject, TYPE.c_str());
    if (IsUInt32(typeJson) && (DHType)typeJson->valueint <= DHType::MAX_DH) {
        compVer.dhType = (DHType)typeJson->valueint;
    }
    cJSON *handlerJson = cJSON_GetObjectItem(jsonObject, HANDLER.c_str());
    if (IsString(handlerJson)) {
        compVer.handlerVersion = handlerJson->valuestring;
    }
    cJSON *sourceVerJson = cJSON_GetObjectItem(jsonObject, SOURCE_VER.c_str());
    if (IsString(sourceVerJson)) {
        compVer.sourceVersion = sourceVerJson->valuestring;
    }
    cJSON *sinkVerJson = cJSON_GetObjectItem(jsonObject, SINK_VER.c_str());
    if (IsString(sinkVerJson)) {
        compVer.sinkVersion = sinkVerJson->valuestring;
    }
    cJSON *sourceFeatureFilters = cJSON_GetObjectItem(jsonObject, SOURCE_FEATURE_FILTER.c_str());
    cJSON *sinkSupportedFeatures = cJSON_GetObjectItem(jsonObject, SINK_SUPPORTED_FEATURE.c_str());
    if (sourceFeatureFilters || sinkSupportedFeatures) {
        compVer.haveFeature = true;
        if (sourceFeatureFilters) {
            cJSON *filterObj = nullptr;
            compVer.sourceFeatureFilters.clear();
            cJSON_ArrayForEach(filterObj, sourceFeatureFilters) {
                compVer.sourceFeatureFilters.push_back(filterObj->valuestring);
            }
        }
        if (sinkSupportedFeatures) {
            cJSON *featureObj = nullptr;
            compVer.sinkSupportedFeatures.clear();
            cJSON_ArrayForEach(featureObj, sinkSupportedFeatures) {
                compVer.sinkSupportedFeatures.push_back(featureObj->valuestring);
            }
        }
    } else {
        compVer.haveFeature = false;
    }
}

void FromJson(const cJSON *jsonObject, VersionInfo &versionInfo)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *devIdJson = cJSON_GetObjectItem(jsonObject, DEV_ID.c_str());
    if (IsString(devIdJson)) {
        versionInfo.deviceId = devIdJson->valuestring;
    }

    cJSON *dhVerJson = cJSON_GetObjectItem(jsonObject, DH_VER.c_str());
    if (IsString(dhVerJson)) {
        versionInfo.dhVersion = dhVerJson->valuestring;
    }

    const cJSON *compVer = cJSON_GetObjectItem(jsonObject, COMP_VER.c_str());
    if (compVer != NULL) {
        cJSON *compVerObj = nullptr;
        cJSON_ArrayForEach(compVerObj, compVer) {
            CompVersion compVerValue;
            FromJson(compVerObj, compVerValue);
            versionInfo.compVersions.insert(std::pair<DHType, CompVersion>(compVerValue.dhType, compVerValue));
        }
    }
}
} // namespace DistributedHardware
} // namespace OHOS
