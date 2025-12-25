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

#include "distributed_hardware_manager_factory.h"

#include <cstdlib>
#include <dlfcn.h>
#include <pthread.h>
#include <string>
#include <thread>
#include <vector>

#include "dm_device_info.h"
#include "device_manager.h"
#include "iservice_registry.h"
#include "mem_mgr_client.h"
#include "mem_mgr_proxy.h"
#include "system_ability_definition.h"

#include "anonymous_string.h"
#include "constants.h"
#include "component_manager.h"
#include "dh_context.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager.h"
#include "device_param_mgr.h"
#include "event_handler_factory.h"
#include "hdf_operate.h"
#include "local_capability_info_manager.h"
#include "meta_info_manager.h"
#include "task_board.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t OLD_HO_DEVICE_TYPE = -1;
    constexpr int32_t NEW_HO_DEVICE_TYPE = 11;
    const std::string SA_STATUS_TASK_ID = "sa_status_task";
    constexpr int32_t DELAY_TIME_MS = 480000;
}
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareManagerFactory"

IMPLEMENT_SINGLE_INSTANCE(DistributedHardwareManagerFactory);
bool DistributedHardwareManagerFactory::InitLocalDevInfo()
{
    DHLOGI("InitLocalDevInfo start");
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() > 0 && deviceList.size() <= MAX_ONLINE_DEVICE_SIZE) {
        DHLOGI("There is other device online, on need just init db, use normal logic");
        return true;
    }
    auto initResult = DistributedHardwareManager::GetInstance().LocalInit();
    if (initResult != DH_FWK_SUCCESS) {
        DHLOGE("InitLocalDevInfo failed, errCode = %{public}d", initResult);
        return false;
    }
    DHLOGI("init local dev info, create exit dfwk task!");
    TaskParam taskParam;
    auto task = TaskFactory::GetInstance().CreateTask(TaskType::EXIT_DFWK, taskParam, nullptr);
    TaskExecutor::GetInstance().PushTask(task);
    return true;
}

bool DistributedHardwareManagerFactory::Init()
{
    DHLOGI("start");
    auto initResult = DistributedHardwareManager::GetInstance().Initialize();
    if (initResult != DH_FWK_SUCCESS) {
        DHLOGE("Initialize failed, errCode = %{public}d", initResult);
        return false;
    }
    isInit_.store(true);
    releaseStatus_.store(false);
    if (CreateSaStatusHandler() == DH_FWK_SUCCESS) {
        DHLOGI("CreateSaStatusHandler success!");
        DelaySaStatusTask();
    }
    DHLOGI("success");
    return true;
}

void DistributedHardwareManagerFactory::UnInit()
{
    DHLOGI("start");
    std::lock_guard<std::mutex> lock(releaseProcessMutex_);
    if (releaseStatus_.load()) {
        DHLOGE("Releasing resources is complete, not need to be released again.");
        return;
    }
    releaseStatus_.store(true);
    DHTraceStart(COMPONENT_UNLOAD_START);
    HiSysEventWriteMsg(DHFWK_EXIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa exit begin.");

    // release all the resources synchronously
    DistributedHardwareManager::GetInstance().Release();
    isInit_.store(false);
    flagUnInit_.store(false);
    DHTraceEnd();
    CheckExitSAOrNot();
}

void DistributedHardwareManagerFactory::ExitDHFWK()
{
    DHLOGI("No device online or deviceList is over size, exit sa process");
    SetSAProcessState(true);
    HiSysEventWriteMsg(DHFWK_EXIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa exit end.");
    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        DHLOGE("systemAbilityMgr is null");
        return;
    }
    int32_t ret = systemAbilityMgr->UnloadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("systemAbilityMgr UnLoadSystemAbility failed, ret: %{public}d", ret);
        return;
    }
    DHLOGI("systemAbilityMgr UnLoadSystemAbility success");
}

void DistributedHardwareManagerFactory::CheckExitSAOrNot()
{
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if ((deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) &&
        DHContext::GetInstance().GetIsomerismConnectCount() == 0 &&
        HdfOperateManager::GetInstance().IsAnyHdfInuse() == false) {
        ExitDHFWK();
        return;
    }

    DHLOGI("After uninit, DM report devices online, reinit");
    Init();
    for (const auto &deviceInfo : deviceList) {
        const auto networkId = std::string(deviceInfo.networkId, strnlen(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN));
        const auto uuid = GetUUIDByDm(networkId);
        const auto udid = GetUDIDByDm(networkId);
        DHLOGI("Send trusted device online, networkId = %{public}s, uuid = %{public}s",
            GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str());
        uint16_t deviceType = deviceInfo.deviceTypeId;
        int32_t osType = GetDeviceSystemType(deviceInfo.extraData);
        std::thread([this, networkId, uuid, udid, deviceType, osType]() {
            this->SendOnLineEvent(networkId, uuid, udid, deviceType, osType);
        }).detach();
    }
}

bool DistributedHardwareManagerFactory::IsInit()
{
    return isInit_.load();
}

int32_t DistributedHardwareManagerFactory::SendOnLineEvent(const std::string &networkId, const std::string &uuid,
    const std::string &udid, uint16_t deviceType, int32_t osType)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(udid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = pthread_setname_np(pthread_self(), SEND_ONLINE);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SendOnLineEvent setname failed.");
    }

    if (flagUnInit_.load()) {
        DHLOGE("is in uniniting, can not process online event.");
        return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
    }

    if (DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGW("device is already online, uuid = %{public}s", GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_ONLINE;
    }

    DHContext::GetInstance().AddOnlineDevice(udid, uuid, networkId);
    DHContext::GetInstance().AddRealTimeOnlineDeviceNetworkId(networkId);
    DHContext::GetInstance().AddOnlineDeviceType(networkId, deviceType);
    DHContext::GetInstance().AddOnlineDeviceOSType(networkId, osType);

    if (!isInit_.load() && !Init()) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
    }

    if (!ComponentManager::GetInstance().IsSourceEnabled() && !ComponentManager::GetInstance().IsSinkActiveEnabled() &&
        !HdfOperateManager::GetInstance().IsAnyHdfInuse()) {
        int pid = getpid();
        Memory::MemMgrClient::GetInstance().SetCritical(pid, true, DISTRIBUTED_HARDWARE_SA_ID);
        DHLOGI("New device online, set sa status to critical");
        DelaySaStatusTask();
    }

    if (osType == OLD_HO_DEVICE_TYPE || osType == NEW_HO_DEVICE_TYPE) {
        DHLOGE("double frame device, networkId = %{public}s, uuid = %{public}s, udid = %{public}s, need clear data.",
            GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str(), GetAnonyString(udid).c_str());
        ClearRemoteDeviceMetaInfoData(udid, uuid);
        ClearRemoteDeviceLocalInfoData(uuid);
    }
    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId, uuid, udid, deviceType);
    if (onlineResult != DH_FWK_SUCCESS) {
        DHLOGE("online failed, errCode = %{public}d", onlineResult);
        return onlineResult;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::SendOffLineEvent(const std::string &networkId, const std::string &uuid,
    const std::string &udid, uint16_t deviceType)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(udid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (!isInit_.load() && !Init()) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
    }

    if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGE("Device not online, networkId: %{public}s, uuid: %{public}s",
            GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_OFFLINE;
    }

    DHContext::GetInstance().DeleteRealTimeOnlineDeviceNetworkId(networkId);
    if (DHContext::GetInstance().GetRealTimeOnlineDeviceCount() == 0 &&
        DHContext::GetInstance().GetIsomerismConnectCount() == 0) {
        flagUnInit_.store(true);
        DHLOGI("no online device, set uninit flag true");
    }

    auto offlineResult = DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId, uuid, udid, deviceType);
    if (offlineResult != DH_FWK_SUCCESS) {
        DHLOGE("offline failed, errCode = %{public}d", offlineResult);
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::GetComponentVersion(std::unordered_map<DHType, std::string> &versionMap)
{
    DHLOGI("start");
    return DistributedHardwareManager::GetInstance().GetComponentVersion(versionMap);
}

int32_t DistributedHardwareManagerFactory::Dump(const std::vector<std::string> &argsStr, std::string &result)
{
    return DistributedHardwareManager::GetInstance().Dump(argsStr, result);
}

bool DistributedHardwareManagerFactory::GetUnInitFlag()
{
    return flagUnInit_.load();
}

void DistributedHardwareManagerFactory::ClearRemoteDeviceMetaInfoData(const std::string &peerudid,
    const std::string &peeruuid)
{
    MetaInfoManager::GetInstance()->ClearRemoteDeviceMetaInfoData(peerudid, peeruuid);
}

void DistributedHardwareManagerFactory::ClearRemoteDeviceLocalInfoData(const std::string &peeruuid)
{
    LocalCapabilityInfoManager::GetInstance()->ClearRemoteDeviceLocalInfoData(peeruuid);
}

void DistributedHardwareManagerFactory::SetSAProcessState(bool saState)
{
    DHLOGI("Set SA process state: %{public}d", saState);
    isIdle_.store(saState);
}
 
bool DistributedHardwareManagerFactory::GetSAProcessState()
{
    return isIdle_.load();
}

bool DistributedHardwareManagerFactory::GetDHardwareInitState()
{
    return DistributedHardwareManager::GetInstance().GetDHardwareInitState();
}

void DistributedHardwareManagerFactory::ActiveSyncDataByNetworkId(const std::string &networkId)
{
    DHLOGI("active sync data, networkId: %{public}s", GetAnonyString(networkId).c_str());
    MetaInfoManager::GetInstance()->SyncDataByNetworkId(networkId);
}

int32_t DistributedHardwareManagerFactory::CreateSaStatusHandler()
{
    DHLOGI("call!");
    std::lock_guard<std::mutex> lock(saStatusMutex_);
    if (saStatusHandler_ == nullptr) {
        saStatusHandler_ = EventHandlerFactory::GetInstance().GetEventHandler();
    }
    if (saStatusHandler_ == nullptr) {
        DHLOGE("saStatusHandler_ is nullptr!");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DH_FWK_SUCCESS;
}

void DistributedHardwareManagerFactory::DelaySaStatusTask()
{
    DHLOGI("delay sa status task begin");
    auto task = []() {
        if (!ComponentManager::GetInstance().IsSourceEnabled() &&
            !ComponentManager::GetInstance().IsSinkActiveEnabled() &&
            !HdfOperateManager::GetInstance().IsAnyHdfInuse()) {
            int pid = getpid();
            Memory::MemMgrClient::GetInstance().SetCritical(pid, false, DISTRIBUTED_HARDWARE_SA_ID);
            DHLOGI("Delay task timeout, set sa status to Non-critical");
            return;
        }
        DHLOGI("Has business use, no need set sa status");
    };
    {
        std::lock_guard<std::mutex> lock(saStatusMutex_);
        if (saStatusHandler_ == nullptr) {
            DHLOGE("saStatusHandler_ is nullptr ");
            return;
        }
        saStatusHandler_->RemoveTask(SA_STATUS_TASK_ID);
        DHLOGI("delay sa status post task");
        saStatusHandler_->PostTask(task, SA_STATUS_TASK_ID, DELAY_TIME_MS);
    }
}

int32_t DistributedHardwareManagerFactory::DestroySaStatusHandler()
{
    DHLOGI("call!");
    std::lock_guard<std::mutex> lock(saStatusMutex_);
    if (saStatusHandler_ == nullptr) {
        DHLOGE("saStatusHandler_ is nullptr!");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    saStatusHandler_->RemoveTask(SA_STATUS_TASK_ID);
    saStatusHandler_ = nullptr;
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
