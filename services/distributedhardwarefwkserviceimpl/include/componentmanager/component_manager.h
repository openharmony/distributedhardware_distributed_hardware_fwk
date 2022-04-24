/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_H

#include <map>
#include <unordered_map>
#include <mutex>
#include <future>

#include "single_instance.h"
#include "device_type.h"
#include "idistributed_hardware.h"
#include "idistributed_hardware_sink.h"
#include "idistributed_hardware_source.h"

namespace OHOS {
namespace DistributedHardware {
class ComponentManager {
    DECLARE_SINGLE_INSTANCE_BASE(ComponentManager);

public:
    ComponentManager() {}
    ~ComponentManager();

public:
    int32_t Init();
    int32_t UnInit();
    int32_t Enable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        const DHType dhType);
    int32_t Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        const DHType dhType);

private:
    enum class Action : int32_t {
        START_SOURCE,
        START_SINK,
        STOP_SOURCE,
        STOP_SINK
    };

    using ActionResult = std::unordered_map<DHType, std::shared_future<int32_t>>;

    DHType GetDHType(const std::string &uuid, const std::string &dhId) const;
    bool InitCompSource();
    bool InitCompSink();
    ActionResult StartSource();
    ActionResult StopSource();
    ActionResult StartSink();
    ActionResult StopSink();
    bool WaitForResult(const Action &action, ActionResult result);
    int32_t GetEnableParam(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        DHType dhType, EnableParam &param);
    std::string GetSinkVersion(const std::string &networkId, const std::string &uuid, DHType dhType);
    std::string GetVersionFromCache(const std::string &uuid, DHType dhType);
    int32_t UpdateVersionCache(const std::string &networkId, const std::string &uuid);
    sptr<IDistributedHardware> GetRemoteDHMS(const std::string &networkId) const;

private:
    std::map<DHType, IDistributedHardwareSource*> compSource_;
    std::map<DHType, IDistributedHardwareSink*> compSink_;
    std::unordered_map<std::string, std::unordered_map<DHType, std::string>> sinkVersions_;
    std::mutex sinkVersionMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
