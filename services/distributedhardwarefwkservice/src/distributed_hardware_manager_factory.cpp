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

#include "distributed_hardware_manager_factory.h"

#include <cstdlib>
#include <dlfcn.h>
#include <string>
#include <thread>
#include <vector>

#include "dm_device_info.h"
#include "hisysevent.h"

#include "anonymous_string.h"
#include "constants.h"
#include "device_manager.h"
#include "dh_context.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareManagerFactory"

IMPLEMENT_SINGLE_INSTANCE(DistributedHardwareManagerFactory);
bool DistributedHardwareManagerFactory::Init()
{
    DHLOGI("start");
    isInit = true;
    auto initResult = DistributedHardwareManager::GetInstance().Initialize();
    if (initResult != DH_FWK_SUCCESS) {
        DHLOGE("Initialize failed, errCode = %d", initResult);
        return false;
    }
    DHLOGD("success");
    return true;
}

void DistributedHardwareManagerFactory::UnInit()
{
    DHLOGI("start");
    DHTraceStart(COMPONENT_UNLOAD_START);
    HiSysEventWriteMsg(DHFWK_EXIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa exit begin.");

    // release all the resources synchronously
    DistributedHardwareManager::GetInstance().Release();
    isInit = false;
    DHTraceEnd();
    CheckExitSAOrNot();
}

void DistributedHardwareManagerFactory::CheckExitSAOrNot()
{
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() == 0) {
        DHLOGI("DM report devices offline, exit sa process");
        HiSysEventWriteMsg(DHFWK_EXIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "dhfwk sa exit end.");

        _Exit(0);
    }

    DHLOGI("After uninit, DM report devices online, reinit");
    Init();
    for (const auto &deviceInfo : deviceList) {
        const auto networkId = std::string(deviceInfo.deviceId);
        const auto uuid = GetUUIDBySoftBus(networkId);
        DHLOGI("Send trusted device online, networkId = %s, uuid = %s", GetAnonyString(networkId).c_str(),
            GetAnonyString(uuid).c_str());
        std::thread(&DistributedHardwareManagerFactory::SendOnLineEvent, this, networkId, uuid,
            deviceInfo.deviceTypeId).detach();
    }
}

bool DistributedHardwareManagerFactory::IsInit()
{
    return isInit.load();
}

int32_t DistributedHardwareManagerFactory::SendOnLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return ERR_DH_FWK_REMOTE_NETWORK_ID_IS_EMPTY;
    }

    if (uuid.empty()) {
        DHLOGE("uuid is empty");
        return ERR_DH_FWK_REMOTE_DEVICE_ID_IS_EMPTY;
    }

    if (DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGW("device is already online, uuid = %s", GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_ONLINE;
    }

    DHContext::GetInstance().AddOnlineDevice(uuid, networkId);

    if (!isInit && !Init()) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
    }

    auto onlineResult = DistributedHardwareManager::GetInstance().SendOnLineEvent(networkId, uuid, deviceType);
    if (onlineResult != DH_FWK_SUCCESS) {
        DHLOGE("online failed, errCode = %d", onlineResult);
        return onlineResult;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareManagerFactory::SendOffLineEvent(const std::string &networkId, const std::string &uuid,
    uint16_t deviceType)
{
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return ERR_DH_FWK_REMOTE_NETWORK_ID_IS_EMPTY;
    }

    if (uuid.empty()) {
        DHLOGE("uuid is empty");
        return ERR_DH_FWK_REMOTE_DEVICE_ID_IS_EMPTY;
    }

    if (!isInit && !Init()) {
        DHLOGE("distributedHardwareMgr is null");
        return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
    }

    if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGE("Device not online, networkId: %s, uuid: %s",
            GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_OFFLINE;
    }

    auto offlineResult = DistributedHardwareManager::GetInstance().SendOffLineEvent(networkId, uuid, deviceType);
    if (offlineResult != DH_FWK_SUCCESS) {
        DHLOGE("offline failed, errCode = %d", offlineResult);
        return offlineResult;
    }

    DHContext::GetInstance().RemoveOnlineDevice(uuid);
    if (DistributedHardwareManager::GetInstance().GetOnLineCount() == 0) {
        DHLOGI("all devices are offline, start to free the resource");
        UnInit();
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
} // namespace DistributedHardware
} // namespace OHOS
