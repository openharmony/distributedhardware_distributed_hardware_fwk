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

#include "distributed_hardware_service.h"

#include <cinttypes>

#include "accesstoken_kit.h"
#include "cJSON.h"
#include "device_manager.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "ipublisher_listener.h"
#include "iservice_registry.h"
#include "mem_mgr_client.h"
#include "mem_mgr_proxy.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "access_manager.h"
#include "anonymous_string.h"
#include "av_trans_control_center.h"
#include "constants.h"
#include "capability_info_manager.h"
#include "component_manager.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "dh_utils_hisysevent.h"
#include "distributed_hardware_fwk_kit_paras.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager_factory.h"
#include "hdf_operate.h"
#include "local_capability_info_manager.h"
#include "meta_info_manager.h"
#include "publisher.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareService"
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedHardwareService, DISTRIBUTED_HARDWARE_SA_ID, true);
namespace {
    constexpr int32_t SA_READY_TO_UNLOAD = 0;
    constexpr int32_t SA_REFUSE_TO_UNLOAD = -1;
    constexpr int32_t INIT_BUSINESS_DELAY_TIME_MS = 5 * 100;
    const std::string INIT_TASK_ID = "CheckAndInitDH";
    const std::string LOCAL_NETWORKID_ALIAS = "local";
    constexpr int32_t DMSDP_ADAPTER_SA_ID = 4812;
    constexpr int32_t DHMS_SERVICE_SA_ID = 4801;
}

DistributedHardwareService::DistributedHardwareService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
}

void DistributedHardwareService::OnStart()
{
    DHLOGI("DistributedHardwareService::OnStart start");
    HiSysEventWriteMsg(DHFWK_INIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa start on demand.");

    if (state_ == ServiceRunningState::STATE_RUNNING) {
        DHLOGI("DistributedHardwareService has already started.");
        return;
    }
    if (!Init()) {
        DHLOGE("failed to init DistributedHardwareService");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    DistributedHardwareManagerFactory::GetInstance().SetSAProcessState(false);
    AddSystemAbilityListener(MEMORY_MANAGER_SA_ID);
    DHLOGI("DistributedHardwareService::OnStart start service success.");
}

bool DistributedHardwareService::Init()
{
    DHLOGI("DistributedHardwareService::Init ready to init.");
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("DistributedHardwareService::Init Publish failed!");
            HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
                "dhfwk sa init publish failed.");
            return false;
        }
        registerToService_ = true;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    auto executeInnerFunc = [this] { DoBusinessInit(); };
    eventHandler_->PostTask(executeInnerFunc, INIT_TASK_ID, 0);
    DHLOGI("DistributedHardwareService::Init success.");
    HiSysEventWriteMsg(DHFWK_INIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa init success.");
    return true;
}

void DistributedHardwareService::InitLocalDevInfo()
{
    DHLOGI("Init Local device info in DB");
    DistributedHardwareManagerFactory::GetInstance().InitLocalDevInfo();
}

bool DistributedHardwareService::DoBusinessInit()
{
    if (!IsDepSAStart()) {
        DHLOGW("Depend sa not start");
        auto executeInnerFunc = [this] { DoBusinessInit(); };
        eventHandler_->PostTask(executeInnerFunc, INIT_TASK_ID, INIT_BUSINESS_DELAY_TIME_MS);
        return false;
    }

    DHLOGI("Init AccessManager");
    auto ret = AccessManager::GetInstance()->Init();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DistributedHardwareService::Init failed.");
        HiSysEventWriteErrCodeMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            ret, "dhfwk sa AccessManager init fail.");
    }
    InitLocalDevInfo();
    AccessManager::GetInstance()->CheckTrustedDeviceOnline();
    return true;
}

bool DistributedHardwareService::IsDepSAStart()
{
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        DHLOGE("Get System Ability Manager failed");
        return false;
    }
    DHLOGI("Check DSoftbus sa");
    sptr<IRemoteObject> remoteObject = saMgr->CheckSystemAbility(SOFTBUS_SERVER_SA_ID);
    if (remoteObject == nullptr) {
        DHLOGW("DSoftbus not start");
        return false;
    }
    DHLOGI("Check KVDB sa");
    remoteObject = saMgr->CheckSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    if (remoteObject == nullptr) {
        DHLOGW("KVDB not start");
        return false;
    }
    DHLOGI("Check DM sa");
    remoteObject = saMgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (remoteObject == nullptr) {
        DHLOGW("DM not start");
        return false;
    }
    return true;
}

void DistributedHardwareService::OnStop()
{
    DHLOGI("DistributedHardwareService::OnStop ready to stop service.");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    int pid = getpid();
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(pid, 1, 0, DISTRIBUTED_HARDWARE_SA_ID);
}

int32_t DistributedHardwareService::OnIdle(const SystemAbilityOnDemandReason& idleReason)
{
    bool saState = DistributedHardwareManagerFactory::GetInstance().GetSAProcessState();
    DHLOGI("OnIdle, idleReason name: %{public}s, id: %{public}d, value: %{public}s, sa process state: %{public}d",
        idleReason.GetName().c_str(), idleReason.GetId(), idleReason.GetValue().c_str(), saState);
    return saState ? SA_READY_TO_UNLOAD : SA_REFUSE_TO_UNLOAD;
}

int32_t DistributedHardwareService::RegisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> listener)
{
    Publisher::GetInstance().RegisterListener(topic, listener);
    ComponentManager::GetInstance().SetAVSyncScene(topic);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> listener)
{
    Publisher::GetInstance().UnregisterListener(topic, listener);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::PublishMessage(const DHTopic topic, const std::string &msg)
{
    if (topic == DHTopic::TOPIC_CREATE_SESSION_READY) {
        // because permission check, fwk no need send RPC, dmsdp send RPC.
        DHLOGI("Sink DMSDP create session server success, no need fwk notify remote.");
        return DH_FWK_SUCCESS;
    }
    DHContext::GetInstance();
    Publisher::GetInstance().PublishMessage(topic, msg);
    return DH_FWK_SUCCESS;
}

std::string DistributedHardwareService::QueryLocalSysSpec(const QueryLocalSysSpecType spec)
{
    DeviceInfo localDevInfo = DHContext::GetInstance().GetDeviceInfo();
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(localDevInfo.deviceId, resInfos);
    DHType targetDhType = DHType::UNKNOWN;
    std::string targetKey = "";
    switch (spec) {
        case QueryLocalSysSpecType::HISTREAMER_AUDIO_ENCODER:
            targetKey = KEY_HISTREAMER_AUDIO_ENCODER;
            targetDhType = DHType::AUDIO;
            break;
        case QueryLocalSysSpecType::HISTREAMER_AUDIO_DECODER:
            targetKey = KEY_HISTREAMER_AUDIO_DECODER;
            targetDhType = DHType::AUDIO;
            break;
        case QueryLocalSysSpecType::HISTREAMER_VIDEO_ENCODER:
            targetKey = KEY_HISTREAMER_VIDEO_ENCODER;
            targetDhType = DHType::SCREEN;
            break;
        case QueryLocalSysSpecType::HISTREAMER_VIDEO_DECODER:
            targetKey = KEY_HISTREAMER_VIDEO_DECODER;
            targetDhType = DHType::SCREEN;
            break;
        default:
            break;
    }

    DHLOGE("QueryLocalSysSpec targetKey: %{public}s, targetDhType: %{public}" PRIu32, targetKey.c_str(),
        (uint32_t)targetDhType);
    if (targetDhType == DHType::UNKNOWN) {
        DHLOGE("Can not find matched dhtype");
        return "";
    }

    std::string attrs = "";
    for (const auto &cap : resInfos) {
        if (cap->GetDHType() != targetDhType) {
            continue;
        }
        attrs = cap->GetDHAttrs();
        break;
    }
    if (attrs.empty()) {
        DHLOGE("Can not find dh attrs");
        return "";
    }

    return QueryDhSysSpec(targetKey, attrs);
}

std::string DistributedHardwareService::QueryDhSysSpec(const std::string &targetKey, std::string &attrs)
{
    cJSON *attrJson = cJSON_Parse(attrs.c_str());
    if (attrJson == NULL) {
        DHLOGE("attrs json is invalid, attrs: %{public}s", attrs.c_str());
        return "";
    }
    cJSON *targetKeyJson = cJSON_GetObjectItem(attrJson, targetKey.c_str());
    if (!IsString(targetKeyJson)) {
        DHLOGE("Attrs Json not contains key: %{public}s", targetKey.c_str());
        cJSON_Delete(attrJson);
        return "";
    }
    std::string result = targetKeyJson->valuestring;
    cJSON_Delete(attrJson);
    return result;
}

int32_t DistributedHardwareService::InitializeAVCenter(const TransRole &transRole, int32_t &engineId)
{
    return AVTransControlCenter::GetInstance().InitializeAVCenter(transRole, engineId);
}

int32_t DistributedHardwareService::ReleaseAVCenter(int32_t engineId)
{
    return AVTransControlCenter::GetInstance().ReleaseAVCenter(engineId);
}

int32_t DistributedHardwareService::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    return AVTransControlCenter::GetInstance().CreateControlChannel(engineId, peerDevId);
}

int32_t DistributedHardwareService::NotifyAVCenter(int32_t engineId, const AVTransEvent &event)
{
    return AVTransControlCenter::GetInstance().NotifyAVCenter(engineId, event);
}

int32_t DistributedHardwareService::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAvTransControlCenterCallback> callback)
{
    return AVTransControlCenter::GetInstance().RegisterCtlCenterCallback(engineId, callback);
}

int32_t DistributedHardwareService::NotifySourceRemoteSinkStarted(std::string &udid)
{
    if (CheckDHAccessPermission(udid) != DH_FWK_SUCCESS) {
        DHLOGE("check permission failed.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHLOGI("Notify source remote init sink DHMS ready start.");
    Publisher::GetInstance().PublishMessage(DHTopic::TOPIC_INIT_DHMS_READY, udid);
    DHLOGI("Notify source remote init sink DHMS ready End.");
    return DH_FWK_SUCCESS;
}

int DistributedHardwareService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("DistributedHardwareService  Dump.");

    std::vector<std::string> argsStr {};
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    std::string result("");
    int ret = AccessManager::GetInstance()->Dump(argsStr, result);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Dump error, ret = %{public}d", ret);
    }

    if (dprintf(fd, "%s\n", result.c_str()) < 0) {
        DHLOGE("Hidump dprintf error");
        ret = ERR_DH_FWK_HIDUMP_DPRINTF_ERROR;
    }

    return ret;
}

int32_t DistributedHardwareService::PauseDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::map<DHType, IDistributedHardwareSink*> sinkMap = ComponentManager::GetInstance().GetDHSinkInstance();
    if (sinkMap.find(dhType) == sinkMap.end()) {
        DHLOGE("PauseDistributedHardware for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (sinkMap[dhType] == nullptr) {
        DHLOGE("Sinkhandler ptr is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = sinkMap[dhType]->PauseDistributedHardware(networkId);
    if (ret != 0) {
        DHLOGE("PauseDistributedHardware for DHType: %{public}u failed, ret: %{public}d", (uint32_t)dhType, ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::ResumeDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::map<DHType, IDistributedHardwareSink*> sinkMap = ComponentManager::GetInstance().GetDHSinkInstance();
    if (sinkMap.find(dhType) == sinkMap.end()) {
        DHLOGE("ResumeDistributedHardware for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (sinkMap[dhType] == nullptr) {
        DHLOGE("Sinkhandler ptr is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = sinkMap[dhType]->ResumeDistributedHardware(networkId);
    if (ret != 0) {
        DHLOGE("ResumeDistributedHardware for DHType: %{public}u failed, ret: %{public}d", (uint32_t)dhType, ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::StopDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::map<DHType, IDistributedHardwareSink*> sinkMap = ComponentManager::GetInstance().GetDHSinkInstance();
    if (sinkMap.find(dhType) == sinkMap.end()) {
        DHLOGE("StopDistributedHardware for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (sinkMap[dhType] == nullptr) {
        DHLOGE("Sinkhandler ptr is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = sinkMap[dhType]->StopDistributedHardware(networkId);
    if (ret != 0) {
        DHLOGE("StopDistributedHardware for DHType: %{public}u failed, ret: %{public}d", (uint32_t)dhType, ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::GetDeviceDhInfo(const std::string &realNetworkId, const std::string &udidHash,
    const std::string &deviceId, EnableStep enableStep, const sptr<IGetDhDescriptorsCallback> callback)
{
    if (!IsIdLengthValid(realNetworkId) || !IsIdLengthValid(udidHash) || !IsIdLengthValid(deviceId)) {
        DHLOGE("Param is Invalid");
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Get device hardware info start.");
    std::vector<DHDescriptor> descriptors;
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(udidHash, metaCapInfos);
    if (!metaCapInfos.empty()) {
        for (const auto &metaCapInfo : metaCapInfos) {
            DHDescriptor descriptor;
            descriptor.id = metaCapInfo->GetDHId();
            descriptor.dhType = metaCapInfo->GetDHType();
            descriptors.push_back(descriptor);
        }
        DHLOGI("Get MetacapInfo Success, networkId: %{public}s.", GetAnonyString(realNetworkId).c_str());
        callback->OnSuccess(realNetworkId, descriptors, enableStep);
        return DH_FWK_SUCCESS;
    }

    std::vector<std::shared_ptr<CapabilityInfo>> capaInfos;
    LocalCapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(deviceId, capaInfos);
    if (!capaInfos.empty()) {
        for (const auto &capsInfo : capaInfos) {
            DHDescriptor descriptor;
            descriptor.id = capsInfo->GetDHId();
            descriptor.dhType = capsInfo->GetDHType();
            descriptors.push_back(descriptor);
        }
        DHLOGI("Get CapabilitieInfo Success, deviceId: %{public}s.", GetAnonyString(deviceId).c_str());
        callback->OnSuccess(realNetworkId, descriptors, enableStep);
        return DH_FWK_SUCCESS;
    }
    return ERR_DH_FWK_HARDWARE_MANAGER_GET_DHINFO_FAIL;
}

int32_t DistributedHardwareService::GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
    const sptr<IGetDhDescriptorsCallback> callback)
{
    if (!IsIdLengthValid(networkId) || callback == nullptr) {
        DHLOGE("networkId size is invalid or callback ptr is null");
        return ERR_DH_FWK_PARA_INVALID;
    }
    bool isInit = DistributedHardwareManagerFactory::GetInstance().GetDHardwareInitState();
    bool isOnline = DHContext::GetInstance().IsRealTimeOnlineDevice(networkId);
    if (((!isInit || !isOnline) && enableStep == EnableStep::ENABLE_SOURCE) ||
        (!isInit && enableStep == EnableStep::ENABLE_SINK)) {
        DHLOGE("DHManager is not init or the device is not online device.");
        return ERR_DH_FWK_HARDWARE_MANAGER_BUSY;
    }
    DHLOGI("GetDistributedHardware start");
    std::string deviceId;
    std::string realNetworkId;
    std::string udid;
    std::string udidHash;
    if (networkId == LOCAL_NETWORKID_ALIAS || networkId == GetLocalDeviceInfo().networkId) {
        deviceId = GetLocalDeviceInfo().deviceId;
        realNetworkId = GetLocalDeviceInfo().networkId;
        udid = GetLocalDeviceInfo().udid;
        udidHash = GetLocalDeviceInfo().udidHash;
    } else {
        deviceId = DHContext::GetInstance().GetDeviceIdByNetworkId(networkId);
        realNetworkId = networkId;
        udid = DHContext::GetInstance().GetUDIDByNetworkId(networkId);
        udidHash = Sha256(udid);
    }
    auto ret = GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGI("Need active sync deviceInfo by softbus.");
        ComponentManager::GetInstance().SyncRemoteDeviceInfoBySoftbus(realNetworkId, enableStep, callback);
    }
    DHLOGI("GetDistributedHardware end");
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    int32_t ret = ComponentManager::GetInstance().RegisterDHStatusListener(listener, callingUid, callingPid);
    if (ret != 0) {
        DHLOGE("RegisterDHStatusListener failed, ret: %{public}d.", ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    int32_t ret = ComponentManager::GetInstance().UnregisterDHStatusListener(listener, callingUid, callingPid);
    if (ret != 0) {
        DHLOGE("UnregisterDHStatusListener failed, ret: %{public}d.", ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::RegisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    int32_t ret = ComponentManager::GetInstance().RegisterDHStatusListener(
        networkId, listener, callingUid, callingPid);
    if (ret != 0) {
        DHLOGE("RegisterDHStatusListener failed, ret: %{public}d.", ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::UnregisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    int32_t ret = ComponentManager::GetInstance().UnregisterDHStatusListener(
        networkId, listener, callingUid, callingPid);
    if (ret != 0) {
        DHLOGE("UnregisterDHStatusListener failed, ret: %{public}d.", ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::EnableSink(const std::vector<DHDescriptor> &descriptors)
{
    if (!ComponentManager::GetInstance().IsSinkActiveEnabled()) {
        DHLOGI("No business enable sink, set sa status to critical and start delay task");
        int pid = getpid();
        Memory::MemMgrClient::GetInstance().SetCritical(pid, true, DISTRIBUTED_HARDWARE_SA_ID);
        DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();
    }
    for (const auto &descriptor : descriptors) {
        TaskParam taskParam = {
            .dhId = descriptor.id,
            .dhType = descriptor.dhType,
            .effectSink = true,
            .effectSource = false,
            .callingUid = IPCSkeleton::GetCallingUid(),
            .callingPid = IPCSkeleton::GetCallingPid()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::DisableSink(const std::vector<DHDescriptor> &descriptors)
{
    DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();
    for (const auto &descriptor : descriptors) {
        TaskParam taskParam = {
            .dhId = descriptor.id,
            .dhType = descriptor.dhType,
            .effectSink = true,
            .effectSource = false,
            .callingUid = IPCSkeleton::GetCallingUid(),
            .callingPid = IPCSkeleton::GetCallingPid()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::DISABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::EnableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (!ComponentManager::GetInstance().IsSourceEnabled() && !HdfOperateManager::GetInstance().IsAnyHdfInuse()) {
        DHLOGI("No business enable source, set sa status to critical and start delay task");
        int pid = getpid();
        Memory::MemMgrClient::GetInstance().SetCritical(pid, true, DISTRIBUTED_HARDWARE_SA_ID);
        DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();
    }
    for (const auto &descriptor : descriptors) {
        TaskParam taskParam = {
            .networkId = networkId,
            .dhId = descriptor.id,
            .dhType = descriptor.dhType,
            .effectSink = false,
            .effectSource = true,
            .callingUid = IPCSkeleton::GetCallingUid(),
            .callingPid = IPCSkeleton::GetCallingPid()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::DisableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();
    for (const auto &descriptor : descriptors) {
        TaskParam taskParam = {
            .networkId = networkId,
            .dhId = descriptor.id,
            .dhType = descriptor.dhType,
            .effectSink = false,
            .effectSource = true,
            .callingUid = IPCSkeleton::GetCallingUid(),
            .callingPid = IPCSkeleton::GetCallingPid()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::DISABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::LoadDistributedHDF(const DHType dhType)
{
    if (!ComponentManager::GetInstance().IsSourceEnabled() && !HdfOperateManager::GetInstance().IsAnyHdfInuse()) {
        DHLOGI("No business load hdf, set sa status to critical and start delay task");
        int pid = getpid();
        Memory::MemMgrClient::GetInstance().SetCritical(pid, true, DISTRIBUTED_HARDWARE_SA_ID);
        DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();
    }
    switch (dhType) {
        case DHType::AUDIO:
        case DHType::CAMERA:
            return HdfOperateManager::GetInstance().LoadDistributedHDF(dhType);
        default:
            break;
    }
    return ERR_DH_FWK_NO_HDF_SUPPORT;
}

int32_t DistributedHardwareService::UnLoadDistributedHDF(const DHType dhType)
{
    switch (dhType) {
        case DHType::AUDIO:
        case DHType::CAMERA:
            DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();
            return HdfOperateManager::GetInstance().UnLoadDistributedHDF(dhType);
        default:
            break;
    }
    return ERR_DH_FWK_NO_HDF_SUPPORT;
}

int32_t DistributedHardwareService::LoadSinkDMSDPService(const std::string &udid)
{
    if (CheckDHAccessPermission(udid) != DH_FWK_SUCCESS) {
        DHLOGE("check permission failed.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHLOGI("Load DMSDP SA start");
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        DHLOGE("Get System Ability Manager failed");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    sptr<IRemoteObject> remote = saMgr->CheckSystemAbility(DMSDP_ADAPTER_SA_ID);
    if (remote != nullptr) {
        DHLOGI("DMSDP service has already been loaded!");
        return DH_FWK_SUCCESS;
    }
    sptr<LoadDMSDPServiceCallback> loadDMSDPServiceCb(new LoadDMSDPServiceCallback);
    int32_t ret = saMgr->LoadSystemAbility(DMSDP_ADAPTER_SA_ID, loadDMSDPServiceCb);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Failed to load DMSDP service, ret: %{public}d", ret);
        return ret;
    }
    DHLOGI("Load DMSDP SA end");
    return DH_FWK_SUCCESS;
}

void DistributedHardwareService::LoadDMSDPServiceCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    DHLOGI("Load DMSDP service success. systemAbilityId: %{public}d", systemAbilityId);
    (void)remoteObject;
}

void DistributedHardwareService::LoadDMSDPServiceCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    DHLOGI("Load DMSDP service fail. systemAbilityId: %{public}d", systemAbilityId);
}

int32_t DistributedHardwareService::NotifySinkRemoteSourceStarted(const std::string &udid)
{
    if (CheckDHAccessPermission(udid) != DH_FWK_SUCCESS) {
        DHLOGE("check permission failed.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    DHLOGI("Notify sink remote init source DHMS ready start.");
    Publisher::GetInstance().PublishMessage(DHTopic::TOPIC_SOURCE_DHMS_READY, udid);
    DHLOGI("Notify sink remote init source DHMS ready end.");
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::CheckDHAccessPermission(const std::string &udid)
{
    std::string networkId = "";
    DeviceManager::GetInstance().GetNetworkIdByUdid(DH_FWK_PKG_NAME, udid, networkId);
    if (!IsIdLengthValid(networkId)) {
        DHLOGE("the networkId: %{public}s is invalid, not a trusted device.", GetAnonyString(networkId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("start check the device permission, udid: %{public}s, networkId: %{public}s", GetAnonyString(udid).c_str(),
        GetAnonyString(networkId).c_str());
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetDCallingTokenID();
    uint32_t dAccessToken = Security::AccessToken::AccessTokenKit::AllocLocalTokenID(networkId, callerToken);
    const std::string permissionName = "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE";
    int32_t result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(dAccessToken, permissionName);
    if (result != Security::AccessToken::PERMISSION_GRANTED) {
        DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
        return ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::RegisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, int32_t &timeOut, const std::string &pkgName)
{
    DHLOGI("RegisterHardwareAccessListener dhType=%{public}u, pkgName=%{public}s, timeOut=%{public}d",
        (uint32_t)dhType, pkgName.c_str(), timeOut);

    if (callback == nullptr) {
        DHLOGE("callback is nullptr");
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (pkgName.empty()) {
        DHLOGE("pkgName is empty");
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = ComponentManager::GetInstance().AddAccessListener(dhType, timeOut, pkgName, callback);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("AddAccessListener failed, ret=%{public}d", ret);
        return ret;
    }

    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::UnregisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, const std::string &pkgName)
{
    DHLOGI("UnregisterHardwareAccessListener dhType=%{public}u, pkgName=%{public}s",
        (uint32_t)dhType, pkgName.c_str());

    if (pkgName.empty()) {
        DHLOGE("pkgName is empty");
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = ComponentManager::GetInstance().RemoveAccessListener(dhType, pkgName);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RemoveAccessListener failed, ret=%{public}d", ret);
        return ret;
    }

    return DH_FWK_SUCCESS;
}

void DistributedHardwareService::SetAuthorizationResult(const DHType dhType, const std::string &requestId,
    bool &granted)
{
    DHLOGI("SetAuthorizationResult requestId=%{public}s, granted=%{public}d", requestId.c_str(), granted);

    std::map<DHType, IDistributedHardwareSink*> sinkMap = ComponentManager::GetInstance().GetDHSinkInstance();
    if (sinkMap.find(dhType) == sinkMap.end()) {
        DHLOGE("SetAuthorizationResult for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return;
    }
    if (sinkMap[dhType] == nullptr) {
        DHLOGE("Sinkhandler ptr is null");
        return;
    }
    int32_t ret = sinkMap[dhType]->SetAuthorizationResult(requestId, granted);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SetAuthorizationResult failed, ret=%{public}d", ret);
        return;
    }
}

void DistributedHardwareService::OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId)
{
    (void)deviceId;
    DHLOGI("called systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId == MEMORY_MANAGER_SA_ID) {
        DHLOGI("Notify service start and set sa status to critical");
        int pid = getpid();
        Memory::MemMgrClient::GetInstance().NotifyProcessStatus(pid, 1, 1, DISTRIBUTED_HARDWARE_SA_ID);
        Memory::MemMgrClient::GetInstance().SetCritical(pid, true, DISTRIBUTED_HARDWARE_SA_ID);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
