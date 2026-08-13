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

#include "mock_device_manager.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IDeviceManager> IDeviceManager::deviceManagerInstance_;

std::shared_ptr<IDeviceManager> IDeviceManager::GetOrCreateInstance()
{
    if (!deviceManagerInstance_) {
        deviceManagerInstance_ = std::make_shared<MockDeviceManager>();
    }
    return deviceManagerInstance_;
}

void IDeviceManager::ReleaseInstance()
{
    deviceManagerInstance_.reset();
    deviceManagerInstance_ = nullptr;
}
 
int32_t DeviceManager::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
    return IDeviceManager::GetOrCreateInstance()->GetTrustedDeviceList(pkgName, extra, deviceList);
}
} // namespace DistributedHardware
} // namespace OHOS