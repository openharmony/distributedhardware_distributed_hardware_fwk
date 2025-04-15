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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_DEVICE_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_DEVICE_MANAGER_H

#include <gmock/gmock.h>

#include "device_manager.h"

namespace OHOS {
namespace DistributedHardware {
class IDeviceManager {
public:
    virtual ~IDeviceManager() = default;
    virtual int32_t GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList) = 0;
    static std::shared_ptr<IDeviceManager> GetOrCtreateInstance();
    static void ReleaseInstance();
private:
    static std::shared_ptr<IDeviceManager> deviceManagerInstance_;
};

class MockDeviceManager : public IDeviceManager {
public:
    MOCK_METHOD(int32_t, GetTrustedDeviceList,
        (const std::string &, const std::string &, std::vector<DmDeviceInfo> &));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_DEVICE_MANAGER_H