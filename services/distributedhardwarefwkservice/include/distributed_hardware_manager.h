/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_MANAGER_H

#include "idistributed_hardware_manager.h"
#include "single_instance.h"
#include "device_type.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareManager : public IDistributedHardwareManager {
DECLARE_SINGLE_INSTANCE(DistributedHardwareManager);
public:
    int32_t LocalInit();
    int32_t Initialize() override;
    int32_t Release() override;
    int32_t SendOnLineEvent(const std::string &networkId, const std::string &uuid, uint16_t deviceType) override;
    int32_t SendOffLineEvent(const std::string &networkId, const std::string &uuid, uint16_t deviceType) override;
    size_t GetOnLineCount() override;
    int32_t GetComponentVersion(std::unordered_map<DHType, std::string> &versionMap) override;

    int32_t Dump(const std::vector<std::string> &argsStr, std::string &result) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
