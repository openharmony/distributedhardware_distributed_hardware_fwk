/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "capability_info.h"
#include "device_type.h"
#include "dh_comm_tool.h"
#include "dh_modem_context_ext.h"
#include "event_handler.h"
#include "iauthorization_result_callback.h"
#include "idistributed_hardware.h"
#include "idistributed_hardware_sink.h"
#include "idistributed_hardware_source.h"
#include "impl_utils.h"
#include "ipublisher_listener.h"
#include "low_latency_listener.h"
#include "meta_capability_info.h"
#include "task_board.h"
#include "task_factory.h"
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
        const DHType dhType, bool isActive = false);
    int32_t Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        const DHType dhType);
    void UpdateBusinessState(const std::string &uuid, const std::string &dhId, BusinessState state);
    BusinessState QueryBusinessState(const std::string &uuid, const std::string &dhId);
    void DumpLoadedCompsource(std::set<DHType> &compSourceType);
    void DumpLoadedCompsink(std::set<DHType> &compSinkType);
    void Recover(DHType dhType);
    std::map<DHType, IDistributedHardwareSink*> GetDHSinkInstance();
    void TriggerFullCapsSync(const std::string &networkId);
    void SaveNeedRefreshTask(const TaskParam &taskParam);
    IDistributedHardwareSource* GetDHSourceInstance(DHType dhType);
    /**
     * @brief find the task param and return it.
     *        If the task param exist, get and remove from the cached task params,
     *        save it at the second task param, then return true.
     *        If the task param not exist, return false.
     *
     * @param taskKey the task param uuid and dhid pair.
     * @param taskParam if the task param exist, save it.
     * @return true if the task param exist, return true.
     * @return false if the task param not exist, return false.
     */
    bool FetchNeedRefreshTask(const std::pair<std::string, std::string> &taskKey, TaskParam &taskParam);

    int32_t CheckSinkConfigStart(const DHType dhType, bool &enableSink);
    int32_t CheckDemandStart(const std::string &uuid, const DHType dhType, bool &enableSource);
    int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener, int32_t callingUid, int32_t callingPid);
    int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener, int32_t callingUid, int32_t callingPid);
    int32_t RegisterDHStatusListener(const std::string &networkId,
        sptr<IHDSourceStatusListener> listener, int32_t callingUid, int32_t callingPid);
    int32_t UnregisterDHStatusListener(const std::string &networkId,
        sptr<IHDSourceStatusListener> listener, int32_t callingUid, int32_t callingPid);
    int32_t EnableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid);
    int32_t DisableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid);
    int32_t EnableSource(const std::string &networkId,
        const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid);
    int32_t DisableSource(const std::string &networkId,
        const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid);
    int32_t ForceDisableSink(const DHDescriptor &dhDescriptor);
    int32_t ForceDisableSource(const std::string &networkId, const DHDescriptor &dhDescriptor);
    int32_t CheckIdenticalAccount(const std::string &networkId,
        const std::string &uuid, const DHDescriptor &dhDescriptor);
    int32_t EnableMetaSource(const std::string &networkId, const DHDescriptor &dhDescriptor,
        std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr);
    int32_t DisableMetaSource(const std::string &networkId, const DHDescriptor &dhDescriptor,
        std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr);
    void SyncRemoteDeviceInfoBySoftbus(const std::string &realNetworkId, EnableStep enableStep,
        const sptr<IGetDhDescriptorsCallback> callback);
    void OnGetDescriptors(const std::string &realNetworkId, const std::vector<DHDescriptor> &descriptors);
    void SetAVSyncScene(const DHTopic topic);
    void UpdateSinkBusinessState(const std::string &networkId, const std::string &dhId, BusinessSinkState state);
    int32_t InitAVSyncSharedMemory();
    void DeinitAVSyncSharedMemory();
    int32_t GetDHIdByDHSubtype(const DHSubtype dhSubtype, std::string &networkId, std::string &dhId);
    int32_t GetDHSubtypeByDHId(DHSubtype &dhSubtype, const std::string &networkId, const std::string &dhId);
    void HandleIdleStateChange(const std::string &networkId, const std::string &dhId, const DHType dhType);
    void HandleBusinessStateChange(const std::string &networkId, const std::string &dhId, const DHSubtype dhSubType,
        const BusinessState state);
    void NotifyBusinessStateChange(const DHSubtype dhSubType, const BusinessState state);
    int32_t AddAccessListener(const DHType dhType, int32_t &timeOut, const std::string &pkgName,
        const sptr<IAuthorizationResultCallback> &callback);
    int32_t RemoveAccessListener(const DHType dhType, const std::string &pkgName);
    sptr<IAuthorizationResultCallback> GetAccessListener(const DHType dhType, const std::string &pkgName);
    bool IsSourceEnabled();
    bool IsSinkActiveEnabled();
    class ComponentManagerEventHandler : public AppExecFwk::EventHandler {
    public:
        ComponentManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner);
        ~ComponentManagerEventHandler() override = default;
        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    };
    std::shared_ptr<ComponentManager::ComponentManagerEventHandler> GetEventHandler();

private:
    enum class Action : int32_t {
        START_SOURCE,
        START_SINK,
        STOP_SOURCE,
        STOP_SINK
    };

    enum class EnableState : int32_t {
        DISABLED,
        ENABLED
    };

    struct SyncShareData {
        volatile int lock;
        uint64_t audio_current_pts;
        uint64_t audio_update_clock;
        float audio_speed;
        uint64_t video_current_pts;
        uint64_t video_update_clock;
        float video_speed;
        uint64_t sync_strategy;
        bool reset;
    };

    enum class AVscene : uint32_t {
        BROADCAST = 1,
        VIDEOCALL = 2
    };

    struct DHStatusCtrlKey {
        int32_t uid;
        int32_t pid;

        bool operator == (const DHStatusCtrlKey &other) const
        {
            return (uid == other.uid) && (pid == other.pid);
        }

        bool operator < (const DHStatusCtrlKey &other) const
        {
            if (uid < other.uid) {
                return true;
            } else if (uid > other.uid) {
                return false;
            }
            return pid < other.pid;
        }
    };

    struct DHStatusCtrl {
        EnableState enableState;
        DHStatusCtrl()
        {
            enableState = EnableState::DISABLED;
        }
    };

    struct DHStatusEnableInfo {
        int32_t refEnable;
        std::map<DHStatusCtrlKey, DHStatusCtrl> dhStatusCtrl;
        DHStatusEnableInfo()
        {
            refEnable = 0;
        }
    };

    struct DHStatusSourceEnableInfoKey {
        std::string networkId;
        std::string dhId;
        bool operator == (const DHStatusSourceEnableInfoKey &other) const
        {
            return (networkId == other.networkId) && (dhId == other.dhId);
        }
        bool operator < (const DHStatusSourceEnableInfoKey &other) const
        {
            if (networkId < other.networkId) {
                return true;
            } else if (networkId > other.networkId) {
                return false;
            }
            return dhId < other.dhId;
        }
    };

    struct DHSinkStatus {
        int32_t refLoad;
        std::map<std::string, DHStatusEnableInfo> enableInfos;  // key is dhid
        std::map<DHStatusCtrlKey, sptr<IHDSinkStatusListener>> listeners;
        DHSinkStatus()
        {
            refLoad = 0;
        }
    };

    struct DHSourceStatus {
        int32_t refLoad;
        std::map<DHStatusSourceEnableInfoKey, DHStatusEnableInfo> enableInfos;
        std::map<DHStatusCtrlKey, sptr<IHDSourceStatusListener>> listeners;
        DHSourceStatus()
        {
            refLoad = 0;
        }
    };

    DHType GetDHType(const std::string &uuid, const std::string &dhId) const;
    int32_t InitCompSource(DHType dhType);
    int32_t UninitCompSource(DHType dhType);
    int32_t InitCompSink(DHType dhType);
    int32_t UninitCompSink(DHType dhType);
    void InitDHCommTool();
    void UnInitDHCommTool();
    int32_t StartSource(DHType dhType, ActionResult &sourceResult);
    int32_t StopSource(DHType dhType, ActionResult &sourceResult);
    int32_t StartSink(DHType dhType, ActionResult &sinkResult);
    int32_t StopSink(DHType dhType, ActionResult &sinkResult);
    bool WaitForResult(const Action &action, ActionResult result);
    int32_t GetEnableParam(const std::string &networkId, const std::string &uuid, const std::string &dhId,
        DHType dhType, EnableParam &param);
    int32_t GetVersionFromVerMgr(const std::string &uuid, const DHType dhType, std::string &version, bool isSink);
    int32_t GetVersionFromVerInfoMgr(const std::string &uuid, const DHType dhType, std::string &version, bool isSink);
    int32_t GetVersion(const std::string &uuid, DHType dhType, std::string &version, bool isSink);
    void UpdateVersionCache(const std::string &uuid, const VersionInfo &versionInfo);

    void DoRecover(DHType dhType);
    bool IsIdenticalAccount(const std::string &networkId);
    int32_t RetryGetEnableParam(const std::string &networkId, const std::string &uuid,
        const std::string &dhId, const DHType dhType, EnableParam &param);
    void StopPrivacy();
    int32_t GetEnableCapParam(const std::string &networkId, const std::string &uuid, DHType dhType, EnableParam &param,
        std::shared_ptr<CapabilityInfo> capability);
    int32_t GetEnableMetaParam(const std::string &networkId, const std::string &uuid, DHType dhType, EnableParam &param,
        std::shared_ptr<MetaCapabilityInfo> metaCapPtr);
    int32_t GetCapParam(const std::string &uuid, const std::string &dhId, std::shared_ptr<CapabilityInfo> &capability);
    int32_t GetMetaParam(const std::string &uuid, const std::string &dhId,
        std::shared_ptr<MetaCapabilityInfo> &metaCapPtr);
    int32_t CheckSubtypeResource(const std::string &subtype, const std::string &networkId);

    int32_t GetRemoteVerInfo(CompVersion &compVersion, const std::string &uuid, DHType dhType);
    bool IsFeatureMatched(const std::vector<std::string> &sourceFeatureFilters,
        const std::vector<std::string> &sinkSupportedFeatures);
    int32_t EnableSinkInternal(const DHDescriptor &dhDescriptor,
        int32_t callingUid, int32_t callingPid, sptr<IHDSinkStatusListener> &listener);
    int32_t DisableSinkInternal(const DHDescriptor &dhDescriptor,
        int32_t callingUid, int32_t callingPid, sptr<IHDSinkStatusListener> &listener);
    int32_t EnableSourceInternal(const std::string &networkId, const DHDescriptor &dhDescriptor,
        int32_t callingUid, int32_t callingPid, sptr<IHDSourceStatusListener> &listener);
    int32_t DisableSourceInternal(const std::string &networkId, const DHDescriptor &dhDescriptor,
        int32_t callingUid, int32_t callingPid, sptr<IHDSourceStatusListener> &listener);
    int32_t ForceDisableSinkInternal(
        const DHDescriptor &dhDescriptor, std::vector<sptr<IHDSinkStatusListener>> &listeners);
    int32_t ForceDisableSourceInternal(const std::string &networkId,
        const DHDescriptor &dhDescriptor, std::vector<sptr<IHDSourceStatusListener>> &listeners);
    int32_t RealEnableSource(const std::string &networkId, const std::string &uuid, const DHDescriptor &dhDescriptor,
        DHStatusCtrl &statusCtrl, DHStatusEnableInfo &enableInfo, DHSourceStatus &status, bool isActive);
    int32_t RealDisableSource(const std::string &networkId, const std::string &uuid, const DHDescriptor &dhDescriptor,
        DHStatusCtrl &statusCtrl, DHStatusEnableInfo &enableInfo, DHSourceStatus &status);
    int32_t EnableMetaSourceInternal(const std::string &networkId, const DHDescriptor &dhDescriptor,
        DHStatusCtrl &statusCtrl, DHStatusEnableInfo &enableInfo, DHSourceStatus &status,
        std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr);
    int32_t DisableMetaSourceInternal(const std::string &networkId, const DHDescriptor &dhDescriptor,
        DHStatusCtrl &statusCtrl, DHStatusEnableInfo &enableInfo, DHSourceStatus &status,
        std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr);
    void ResetSinkEnableStatus(DHType dhType);
    void ResetSourceEnableStatus(DHType dhType);
    void RecoverAutoEnableSink(DHType dhType);
    void RecoverAutoEnableSource(DHType dhType);
    void RecoverActiveEnableSink(DHType dhType);
    void RecoverActiveEnableSource(DHType dhType);
    void OnGetDescriptorsError();
private:
    std::map<DHType, IDistributedHardwareSource*> compSource_;
    std::shared_mutex compSourceMutex_;
    std::map<DHType, IDistributedHardwareSink*> compSink_;
    std::shared_mutex compSinkMutex_;
    std::map<DHType, int32_t> compSrcSaId_;
    std::shared_ptr<ComponentPrivacy> audioCompPrivacy_ = nullptr;
    std::shared_ptr<ComponentPrivacy> cameraCompPrivacy_ = nullptr;
    std::shared_ptr<ComponentMonitor> compMonitorPtr_ = nullptr;
    sptr<LowLatencyListener> lowLatencyListener_ = nullptr;

    std::atomic<bool> isUnInitTimeOut_;
    // record the remote device business state, {{deviceUUID, dhId}, BusinessState}.
    std::map<std::pair<std::string, std::string>, BusinessState> dhBizStates_;
    std::mutex bizStateMtx_;
    std::shared_ptr<DistributedHardwareStateListener> dhStateListener_;
    std::shared_ptr<DistributedHardwareSinkStateListener> dhSinkStateListener_;
    std::shared_ptr<DataSyncTriggerListener> dataSyncTriggerListener_;

    std::shared_ptr<ComponentManager::ComponentManagerEventHandler> eventHandler_;
    std::shared_ptr<DHCommTool> dhCommToolPtr_;

    // save those remote dh that need refresh by full capability, {{device networkId, dhId}, TaskParam}.
    std::map<std::pair<std::string, std::string>, TaskParam> needRefreshTaskParams_;
    std::mutex needRefreshTaskParamsMtx_;

    // distributed hardware enable status maintenance.
    std::map<DHType, DHSinkStatus> dhSinkStatus_;
    std::mutex dhSinkStatusMtx_;
    std::map<DHType, DHSourceStatus> dhSourceStatus_;
    std::mutex dhSourceStatusMtx_;
    std::map<std::string, std::pair<EnableStep, sptr<IGetDhDescriptorsCallback>>> syncDeviceInfoMap_;
    std::mutex syncDeviceInfoMapMutex_;

    WorkModeParam workModeParam_;
    std::mutex workModeParamMtx_;

    sptr<Ashmem> syncSharedMem_ = nullptr;
    std::shared_ptr<SyncShareData> syncShareData_ = nullptr;

    DHTopic dhTopic_ = DHTopic::TOPIC_MIN;
    std::mutex dhTopicMtx_;
    std::map<std::pair<DHType, std::string>, sptr<IAuthorizationResultCallback>> accessListenerMap_;
    std::mutex accessListenerMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
