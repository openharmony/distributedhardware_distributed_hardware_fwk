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

#include "mock_capability_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<ICapabilityInfoManager> ICapabilityInfoManager::CapabilityInfoManagerInstance_;

std::shared_ptr<ICapabilityInfoManager> ICapabilityInfoManager::GetOrCreateInstance()
{
    if (!CapabilityInfoManagerInstance_) {
        CapabilityInfoManagerInstance_ = std::make_shared<MockCapabilityInfoManager>();
    }
    return CapabilityInfoManagerInstance_;
}

void ICapabilityInfoManager::ReleaseInstance()
{
    CapabilityInfoManagerInstance_.reset();
    CapabilityInfoManagerInstance_ = nullptr;
}

int32_t CapabilityInfoManager::GetCapability(const std::string &deviceId, const std::string &dhId,
    std::shared_ptr<CapabilityInfo> &capPtr)
{
    return ICapabilityInfoManager::GetOrCreateInstance()->GetCapability(deviceId, dhId, capPtr);
}

void CapabilityInfoManager::GetCapabilitiesByDeviceId(const std::string &deviceId,
    std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    ICapabilityInfoManager::GetOrCreateInstance()->GetCapabilitiesByDeviceId(deviceId, resInfos);
}
} // namespace DistributedHardware
} // namespace OHOS
