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

namespace OHOS {
namespace DistributedHardware {
void DHVersion::FromJsonString(const std::string &jsonStr)
{
    nlohmann::json jsonObj = nlohmann::json::parse(jsonStr);
    FromJson(jsonObj, *this);
    return DH_FWK_SUCCESS;
}

void ToJson(nlohmann::json &jsonObject, const DHVersion &dhVersion)
{
    jsonObject[UUID] = dhVersion.uuid;
    jsonObject[DH_VERSION] = dhVersion.dhVersion;
    
}
} // namespace DistributedHardware
} // namespace OHOS