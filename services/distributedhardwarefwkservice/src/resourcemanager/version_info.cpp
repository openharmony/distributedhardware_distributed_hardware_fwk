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
    cJSON_AddStringToObject(jsonObject, NAME, compVer.name.c_str());
    cJSON_AddNumberToObject(jsonObject, TYPE, (double)compVer.dhType);
    cJSON_AddStringToObject(jsonObject, HANDLER, compVer.handlerVersion.c_str());
    cJSON_AddStringToObject(jsonObject, SOURCE_VER, compVer.sourceVersion.c_str());
    cJSON_AddStringToObject(jsonObject, SINK_VER, compVer.sinkVersion.c_str());
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
        cJSON_AddItemToObject(jsonObject, SOURCE_FEATURE_FILTER, sourceFeatureFilters);
        for (const auto &feature : compVer.sinkSupportedFeatures) {
            cJSON_AddItemToArray(sinkSupportedFeatures, cJSON_CreateString(feature.c_str()));
        }
        cJSON_AddItemToObject(jsonObject, SINK_SUPPORTED_FEATURE, sinkSupportedFeatures);
    }
}

void ToJson(cJSON *jsonObject, const VersionInfo &versionInfo)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON_AddStringToObject(jsonObject, DEV_ID, versionInfo.deviceId.c_str());
    cJSON_AddStringToObject(jsonObject, DH_VER, versionInfo.dhVersion.c_str());

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
    cJSON_AddItemToObject(jsonObject, COMP_VER, compVers);
}

void FromJson(const cJSON *jsonObject, CompVersion &compVer)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *nameJson = cJSON_GetObjectItem(jsonObject, NAME);
    if (IsString(nameJson)) {
        compVer.name = nameJson->valuestring;
    }
    cJSON *typeJson = cJSON_GetObjectItem(jsonObject, TYPE);
    if (IsUInt32(typeJson) && (DHType)typeJson->valueint <= DHType::MAX_DH) {
        compVer.dhType = (DHType)typeJson->valueint;
    }
    cJSON *handlerJson = cJSON_GetObjectItem(jsonObject, HANDLER);
    if (IsString(handlerJson)) {
        compVer.handlerVersion = handlerJson->valuestring;
    }
    cJSON *sourceVerJson = cJSON_GetObjectItem(jsonObject, SOURCE_VER);
    if (IsString(sourceVerJson)) {
        compVer.sourceVersion = sourceVerJson->valuestring;
    }
    cJSON *sinkVerJson = cJSON_GetObjectItem(jsonObject, SINK_VER);
    if (IsString(sinkVerJson)) {
        compVer.sinkVersion = sinkVerJson->valuestring;
    }
    FromJsoncompVerContinue(jsonObject, compVer);
}

void FromJsoncompVerContinue(const cJSON *jsonObject, CompVersion &compVer)
{
    cJSON *sourceFeatureFilters = cJSON_GetObjectItem(jsonObject, SOURCE_FEATURE_FILTER);
    cJSON *sinkSupportedFeatures = cJSON_GetObjectItem(jsonObject, SINK_SUPPORTED_FEATURE);
    if (!IsArray(sourceFeatureFilters) && !IsArray(sinkSupportedFeatures)) {
        compVer.haveFeature = false;
        return;
    }
    compVer.haveFeature = true;
    if (IsArray(sourceFeatureFilters)) {
        cJSON *filterObj = nullptr;
        compVer.sourceFeatureFilters.clear();
        cJSON_ArrayForEach(filterObj, sourceFeatureFilters) {
            if (filterObj != nullptr && filterObj->type == cJSON_String) {
                compVer.sourceFeatureFilters.push_back(filterObj->valuestring);
            }
        }
    }
    if (IsArray(sinkSupportedFeatures)) {
        cJSON *featureObj = nullptr;
        compVer.sinkSupportedFeatures.clear();
        cJSON_ArrayForEach(featureObj, sinkSupportedFeatures) {
            if (featureObj != nullptr && featureObj->type == cJSON_String) {
                compVer.sinkSupportedFeatures.push_back(featureObj->valuestring);
            }
        }
    }
}

void FromJson(const cJSON *jsonObject, VersionInfo &versionInfo)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *devIdJson = cJSON_GetObjectItem(jsonObject, DEV_ID);
    if (IsString(devIdJson)) {
        versionInfo.deviceId = devIdJson->valuestring;
    }

    cJSON *dhVerJson = cJSON_GetObjectItem(jsonObject, DH_VER);
    if (IsString(dhVerJson)) {
        versionInfo.dhVersion = dhVerJson->valuestring;
    }

    const cJSON *compVer = cJSON_GetObjectItem(jsonObject, COMP_VER);
    if (IsArray(compVer)) {
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
