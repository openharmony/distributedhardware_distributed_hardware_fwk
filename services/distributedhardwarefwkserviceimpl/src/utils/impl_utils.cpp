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

#include "impl_utils.h"

#include "dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
void DHVersion::FromJsonString(const std::string &jsonStr)
{
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr);
    FromJson(jsonObj, *this);
    return DH_FWK_SUCCESS;
}

std::string  DHVersion::ToJsonString()
{
    nlohmann::json jsonObj;
    ToJson(jsonObj, *this);
    return jsonObj.dump();
}

void ToJson(nlohmann::json &jsonObject, const DHVersion &dhVersion)
{
    jsonObject[DEV_ID] = GetDeviceIdByUUID(dhVersion.uuid);
    jsonObject[DH_VER] = dhVersion.dhVersion;

    nlohmann::json compVers;
    for(const auto &compVersion : dhVersion.compVersions) {
        nlohmann::json compVer;
        compVer[NAME] = compVersion.second.name;
        compVer[TYPE] = compVersion.second.dhType;
        compVer[HANDLER] = compVersion.second.handlerVersion;
        compVer[SOURCE_VER] = compVersion.second.sourceVersion;
        compVer[SINK_VER] = compVersion.second.sinkVersion;
        compVers.push_back(compVer);
    }

    jsonObject[COMP_VER] = compVers;
}

void FromJson(const nlohmann::json &jsonObject, DHVersion &dhVersion)
{
    if (jsonObject.find(DEV_ID) != jsonObject.end()) {
        dhVersion.deviceId = jsonObject.at(DEV_ID).get<std::string>();
    }

    if (jsonObject.find(DH_VER) != jsonObject.end()) {
       dhVersion.dhVersion = jsonObject.at(DH_VER).get<std::string>();
    }

    nlohmann::json compVers;
    if (jsonObject.find(COMP_VER) != jsonObject.end()) {
        = jsonObject.at(COMP_VER);
    }
}

} // namespace DistributedHardware
} // namespace OHOS