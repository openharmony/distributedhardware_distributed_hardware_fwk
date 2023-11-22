/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <atomic>
#include <map>
#include <set>
#include <unordered_map>
#include <mutex>
#include <future>

#include "single_instance.h"
#include "component_monitor.h"
#include "device_type.h"
#include "idistributed_hardware.h"
#include "idistributed_hardware_sink.h"
#include "idistributed_hardware_source.h"
#include "low_latency_listener.h"
#include "monitor_task_timer.h"
#include "version_info.h"
#include "component_privacy.h"

namespace OHOS {
namespace DistributedHardware {
using ActionResult = std::unordered_map<DHType, std::shared_future<int32_t>>;
class ComponentManager {
    DECLARE_SINGLE_INSTANCE_BASE(ComponentManager);

public:
    ComponentManager();
    ~ComponentManager();

public:
    int32_t Init();
    int32_t UnInit();
    int32_t Enable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        const DHType dhType);
    int32_t Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        const DHType dhType);

    void DumpLoadedComps(std::set<DHType> &compSourceType, std::set<DHType> &compSinkType);
    void Recover(DHType dhType);
    std::map<DHType, IDistributedHardwareSink*> GetDHSinkInstance();

private:
    enum class Action : int32_t {
        START_SOURCE,
        START_SINK,
        STOP_SOURCE,
        STOP_SINK
    };

    DHType GetDHType(const std::string &uuid, const std::string &dhId) const;
    bool InitCompSource();
    bool InitCompSink();
    ActionResult StartSource();
    ActionResult StartSource(DHType dhType);
    ActionResult StopSource();
    ActionResult StartSink();
    ActionResult StartSink(DHType dhType);
    ActionResult StopSink();
    bool WaitForResult(const Action &action, ActionResult result);
    int32_t GetEnableParam(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        DHType dhType, EnableParam &param);
    int32_t GetVersionFromVerMgr(const std::string &uuid, const DHType dhType, std::string &version, bool isSink);
    int32_t GetVersionFromVerInfoMgr(const std::string &uuid, const DHType dhType, std::string &version, bool isSink);
    int32_t GetVersion(const std::string &networkId, const std::string &uuid,
        DHType dhType, std::string &version, bool isSink);
    void UpdateVersionCache(const std::string &uuid, const VersionInfo &versionInfo);

    void DoRecover(DHType dhType);
    void ReStartSA(DHType dhType);
    void RecoverDistributedHardware(DHType dhType);
    bool IsIdenticalAccount(const std::string &networkId);
    int32_t RetryGetEnableParam(const std::string &networkId, const std::string &uuid,
        const std::string &dhId, const DHType dhType, EnableParam &param);

private:
    std::map<DHType, IDistributedHardwareSource*> compSource_;
    std::map<DHType, IDistributedHardwareSink*> compSink_;
    std::map<DHType, int32_t> compSrcSaId_;
    std::shared_ptr<ComponentPrivacy> audioCompPrivacy_ = nullptr;
    std::shared_ptr<ComponentPrivacy> cameraCompPrivacy_ = nullptr;
    std::shared_ptr<ComponentMonitor> compMonitorPtr_ = nullptr;
    sptr<LowLatencyListener> lowLatencyListener_ = nullptr;
    std::shared_ptr<DHTimer> monitorTaskTimer_ = nullptr;

    std::atomic<bool> isUnInitTimeOut_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
