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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_H

#include <map>
#include <set>
#include <unordered_map>
#include <mutex>
#include <future>

#include "single_instance.h"
#include "device_type.h"
#include "idistributed_hardware.h"
#include "idistributed_hardware_sink.h"
#include "idistributed_hardware_source.h"
#include "version_info.h"

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

    void DumpLoadedComps(std::set<DHType> &compSourceType, std::set<DHType> &compSinkType);

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
    int32_t GetSinkVersionFromVerMgr(const std::string &uuid, const DHType dhType,
        std::string &sinkVersion);
    int32_t GetSinkVersionFromVerInfoMgr(const std::string &uuid, const DHType dhType,
        std::string &sinkVersion);
    int32_t GetSinkVersionFromRPC(const std::string &networkId, const std::string &uuid,
        DHType dhType, std::string &sinkVersion);
    int32_t GetSinkVersion(const std::string &networkId, const std::string &uuid,
        DHType dhType, std::string &sinkVersion);
    void UpdateVersionCache(const std::string &uuid, const VersionInfo &versionInfo);
    void UpdateVersionCache(const std::string &uuid, const std::unordered_map<DHType, std::string> &versions);
    sptr<IDistributedHardware> GetRemoteDHMS(const std::string &networkId) const;

private:
    std::map<DHType, IDistributedHardwareSource*> compSource_;
    std::map<DHType, IDistributedHardwareSink*> compSink_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
