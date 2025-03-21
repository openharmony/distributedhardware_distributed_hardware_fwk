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

#include "component_enable.h"
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {
int32_t ComponentEnable::Enable(const std::string &networkId, const std::string &dhId, const EnableParam &param,
    IDistributedHardwareSource *handler)
{
    return DH_FWK_SUCCESS;
}

int32_t ComponentEnable::OnRegisterResult(const std::string &networkId, const std::string &dhId, int32_t status,
    const std::string &data)
{
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
