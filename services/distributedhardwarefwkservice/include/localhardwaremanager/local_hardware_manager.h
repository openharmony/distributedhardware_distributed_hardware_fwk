/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_LOCAL_HARDWARE_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_LOCAL_HARDWARE_MANAGER_H

#include <map>
#include <memory>
#include <vector>

#include "capability_info.h"
#include "device_type.h"
#include "ihardware_handler.h"
#include "meta_capability_info.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class LocalHardwareManager {
    DECLARE_SINGLE_INSTANCE_BASE(LocalHardwareManager);

public:
    LocalHardwareManager();
    ~LocalHardwareManager();
    void Init();
    void UnInit();

private:
    void QueryLocalHardware(const DHType dhType, IHardwareHandler *hardwareHandler);
    void AddLocalCapabilityInfo(const std::vector<DHItem> &dhItems, const DHType dhType,
                                std::vector<std::shared_ptr<CapabilityInfo>> &capabilityInfos);
    void AddLocalMetaCapInfo(const std::vector<DHItem> &dhItems, const DHType dhType,
        std::vector<std::shared_ptr<MetaCapabilityInfo>> &metaCapInfos);
    void CheckNonExistCapabilityInfo(const std::vector<DHItem> &dhItems, const DHType dhType);
    void GetLocalCapabilityMapByPrefix(const DHType dhType, CapabilityInfoMap &capabilityInfoMap);

private:
    std::map<DHType, IHardwareHandler*> compToolFuncsMap_;
    std::map<DHType, std::shared_ptr<PluginListener>> pluginListenerMap_;
    std::unordered_map<DHType, std::vector<DHItem>> localDHItemsMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
