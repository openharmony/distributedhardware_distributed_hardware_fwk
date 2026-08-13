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

#include "mock_dh_context.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IDHContext> IDHContext::dHContextInstance_;

std::shared_ptr<IDHContext> IDHContext::GetOrCreateInstance()
{
    if (!dHContextInstance_) {
        dHContextInstance_ = std::make_shared<MockDHContext>();
    }
    return dHContextInstance_;
}

void IDHContext::ReleaseInstance()
{
    dHContextInstance_.reset();
    dHContextInstance_ = nullptr;
}

const DeviceInfo& DHContext::GetDeviceInfo()
{
    return IDHContext::GetOrCreateInstance()->GetDeviceInfo();
}

std::string DHContext::GetUUIDByNetworkId(const std::string &networkId)
{
    return IDHContext::GetOrCreateInstance()->GetUUIDByNetworkId(networkId);
}

std::string DHContext::GetUUIDByDeviceId(const std::string &deviceId)
{
    return IDHContext::GetOrCreateInstance()->GetUUIDByDeviceId(deviceId);
}
} // namespace DistributedHardware
} // namespace OHOS
