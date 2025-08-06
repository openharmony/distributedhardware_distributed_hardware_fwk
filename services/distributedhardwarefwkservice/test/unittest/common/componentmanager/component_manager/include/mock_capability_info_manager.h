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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_CAPABILITY_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_CAPABILITY_INFO_MANAGER_H

#include <gmock/gmock.h>

#include "capability_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
class ICapabilityInfoManager {
public:
    virtual ~ICapabilityInfoManager() = default;

    virtual int32_t GetCapability(const std::string &deviceId, const std::string &dhId,
        std::shared_ptr<CapabilityInfo> &capPtr) = 0;
    virtual void GetCapabilitiesByDeviceId(const std::string &deviceId,
        std::vector<std::shared_ptr<CapabilityInfo>> &resInfos);
    static std::shared_ptr<ICapabilityInfoManager> GetOrCreateInstance();
    static void ReleaseInstance();

private:
    static std::shared_ptr<ICapabilityInfoManager> CapabilityInfoManagerInstance_;
};
    
class MockCapabilityInfoManager : public ICapabilityInfoManager {
public:
    MOCK_METHOD(int32_t, GetCapability, (const std::string &, const std::string &,
        std::shared_ptr<CapabilityInfo> &));
    MOCK_METHOD(void, GetCapabilitiesByDeviceId, (const std::string &,
        std::vector<std::shared_ptr<CapabilityInfo>> &));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_CAPABILITY_INFO_MANAGER_H
