/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_H
#define OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_H

#include "nlohmann/json.hpp"

#include "impl_utils.h"

namespace OHOS {
namespace DistributedHardware {
struct VersionInfo {
    std::string deviceId;
    std::string dhVersion;
    std::unordered_map<DHType, CompVersion> compVersions;

    void FromJsonString(const std::string &jsonStr);
    std::string ToJsonString() const;
};

void ToJson(nlohmann::json &jsonObject, const VersionInfo &versionInfo);
void FromJson(const nlohmann::json &jsonObject, CompVersion &compVer);
void FromJson(const nlohmann::json &jsonObject, VersionInfo &versionInfo);
} // namespace DistributedHardware
} // namespace OHOS
#endif