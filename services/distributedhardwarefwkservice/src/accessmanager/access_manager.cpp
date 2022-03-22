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

#include "access_manager.h"

#include <unistd.h>
#include <vector>

#include "anonymous_string.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager_factory.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AccessManager"

constexpr int32_t DH_RETRY_INIT_DM_COUNT = 6;
constexpr int32_t DH_RETRY_INIT_DM_INTERVAL_US = 1000 * 500;
AccessManager::~AccessManager()
{
    UnInit();
}

std::shared_ptr<AccessManager> AccessManager::GetInstance()
{
    static std::shared_ptr<AccessManager> instance(new AccessManager);
    return instance;
}

int32_t AccessManager::Init()
{
    DHLOGI("start");
    if (InitDeviceManager() != DH_FWK_SUCCESS) {
        DHLOGE("InitDeviceManager failed");
        return ERR_DH_FWK_ACCESS_INIT_DM_FAILED;
    }

    if (RegisterDevStateCallback() != DH_FWK_SUCCESS) {
        DHLOGE("RegisterDevStateCallback failed");
        return ERR_DH_FWK_ACCESS_REGISTER_DM_FAILED;
    }
    SendTrustedDeviceOnline();
    return DH_FWK_SUCCESS;
}

int32_t AccessManager::UnInit()
{
    DHLOGI("start");
    if (UnInitDeviceManager() != DH_FWK_SUCCESS) {
        DHLOGE("UnInitDeviceManager failed");
        return ERR_DH_FWK_ACCESS_UNINIT_DM_FAILED;
    }

    if (UnRegisterDevStateCallback() != DH_FWK_SUCCESS) {
        DHLOGE("UnRegisterDevStateCallback failed");
        return ERR_DH_FWK_ACCESS_UNREGISTER_DM_FAILED;
    }
    return DH_FWK_SUCCESS;
}

int32_t AccessManager::InitDeviceManager()
{
    DHLOGI("start");
    return DeviceManager::GetInstance().InitDeviceManager(DH_FWK_PKG_NAME, shared_from_this());
}

int32_t AccessManager::UnInitDeviceManager()
{
    DHLOGI("start");
    return DeviceManager::GetInstance().UnInitDeviceManager(DH_FWK_PKG_NAME);
}

int32_t AccessManager::RegisterDevStateCallback()
{
    return DeviceManager::GetInstance().RegisterDevStateCallback(DH_FWK_PKG_NAME, "", shared_from_this());
}

int32_t AccessManager::UnRegisterDevStateCallback()
{
    return DeviceManager::GetInstance().UnRegisterDevStateCallback(DH_FWK_PKG_NAME);
}

void AccessManager::OnRemoteDied()
{
    for (int32_t tryCount = 0; tryCount < DH_RETRY_INIT_DM_COUNT; ++tryCount) {
        usleep(DH_RETRY_INIT_DM_INTERVAL_US);
        if (Init() == DH_FWK_SUCCESS) {
            DHLOGI("DeviceManager onDied, try to init success, tryCount = %d", tryCount);
            return;
        }
        DHLOGW("DeviceManager onDied, try to init failed, tryCount = %d", tryCount);
    }
    DHLOGE("DeviceManager onDied, try to init has reached the maximum, but still failed");
    return;
}

void AccessManager::OnDeviceOnline(const DmDeviceInfo &deviceInfo)
{
    std::lock_guard<std::mutex> lock(accessMutex_);
    DHLOGI("start, networkId = %s, deviceName = %s, deviceTypeId = %d", GetAnonyString(deviceInfo.deviceId).c_str(),
        deviceInfo.deviceName, deviceInfo.deviceTypeId);

    auto networkId = std::string(deviceInfo.deviceId); // deviceId of DM actually is networkId
    auto uuid = GetUUIDBySoftBus(networkId);
    auto ret =
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, deviceInfo.deviceTypeId);
    DHLOGI("online result = %d, networkId = %s, uuid = %s", ret, GetAnonyString(networkId).c_str(),
        GetAnonyString(uuid).c_str());
}

void AccessManager::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    std::lock_guard<std::mutex> lock(accessMutex_);
    DHLOGI("start, networkId = %s, deviceName = %s, deviceTypeId = %d", GetAnonyString(deviceInfo.deviceId).c_str(),
        deviceInfo.deviceName, deviceInfo.deviceTypeId);

    auto networkId = std::string(deviceInfo.deviceId); // deviceId of DM actually is networkId
    auto uuid = GetUUIDBySoftBus(networkId);
    auto ret =
        DistributedHardwareManagerFactory::GetInstance().SendOffLineEvent(networkId, uuid, deviceInfo.deviceTypeId);
    DHLOGI("offline result = %d, networkId = %s, uuid = %s", ret, GetAnonyString(networkId).c_str(),
        GetAnonyString(uuid).c_str());
}

void AccessManager::OnDeviceReady(const DmDeviceInfo &deviceInfo)
{
    (void)deviceInfo;
    return;
}

void AccessManager::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
    (void)deviceInfo;
    return;
}

void AccessManager::SendTrustedDeviceOnline()
{
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    for (const auto &deviceInfo : deviceList) {
        const auto networkId = std::string(deviceInfo.deviceId);
        const auto uuid = GetUUIDBySoftBus(networkId);
        DHLOGI("Send trusted device online, networkId = %s, uuid = %s", GetAnonyString(networkId).c_str(),
            GetAnonyString(uuid).c_str());
        DistributedHardwareManagerFactory::GetInstance().SendOnLineEvent(networkId, uuid, deviceInfo.deviceTypeId);
    }
}
}
}
