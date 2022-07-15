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

#include "enabled_comps_dump.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(EnabledCompsDump);

void EnabledCompsDump::DumpEnabledComp(const std::string &networkId, const DHType dhType, const std::string &dhId)
{
    HidumpCompInfo info(networkId, dhType, dhId);

    std::lock_guard<std::mutex> lock(compInfosMutex_);
    compInfoSet_.emplace(info);
}

void EnabledCompsDump::DumpDisabledComp(const std::string &networkId, const DHType dhType, const std::string &dhId)
{
    HidumpCompInfo info(networkId, dhType, dhId);

    std::lock_guard<std::mutex> lock(compInfosMutex_);
    auto it = compInfoSet_.find(info);
    if (it != compInfoSet_.end()) {
        compInfoSet_.erase(it);
    }
}

void EnabledCompsDump::Dump(std::set<HidumpCompInfo> &compInfoSet)
{
    std::lock_guard<std::mutex> lock(compInfosMutex_);
    compInfoSet = compInfoSet_;
}
} // namespace DistributedHardware
} // namespace OHOS
