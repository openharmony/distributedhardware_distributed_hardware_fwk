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

#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IDHContext> IDHContext::dhContext_;

std::shared_ptr<IDHContext> IDHContext::GetOrCreateInstance()
{
    if (!dhContext_) {
        dhContext_ = std::make_shared<MockDHContext>();
    }
    return dhContext_;
}

void IDHContext::ReleaseInstance()
{
    dhContext_.reset();
    dhContext_ = nullptr;
}

size_t DHContext::GetRealTimeOnlineDeviceCount()
{
    return IDHContext::GetOrCreateInstance()->GetRealTimeOnlineDeviceCount();
}

uint32_t DHContext::GetIsomerismConnectCount()
{
    return IDHContext::dhContext_->GetIsomerismConnectCount();
}
} // namespace DistributedHardware
} // namespace OHOS
 