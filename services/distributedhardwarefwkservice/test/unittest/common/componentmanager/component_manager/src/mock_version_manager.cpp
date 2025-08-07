/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "mock_version_manager.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IVersionManager> IVersionManager::versionManagerInstance_;

std::shared_ptr<IVersionManager> IVersionManager::GetOrCreateInstance()
{
    if (!versionManagerInstance_) {
        versionManagerInstance_ = std::make_shared<MockVersionManager>();
    }
    return versionManagerInstance_;
}

void IVersionManager::ReleaseInstance()
{
    versionManagerInstance_.reset();
    versionManagerInstance_ = nullptr;
}

int32_t VersionManager::GetCompVersion(const std::string &uuid, const DHType dhType, CompVersion &compVersion)
{
    return IVersionManager::GetOrCreateInstance()->GetCompVersion(uuid, dhType, compVersion);
}
} // namespace DistributedHardware
} // namespace OHOS
