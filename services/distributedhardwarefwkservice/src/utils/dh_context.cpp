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

#include <algorithm>

#include "anonymous_string.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DHContext);
DHContext::DHContext()
{
    DHLOGI("Ctor DHContext");
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DHContext::CommonEventHandler>(runner);
    RegisterPowerStateLinstener();
}

DHContext::~DHContext()
{
    DHLOGI("Dtor DHContext");
}

void DHContext::RegisterPowerStateLinstener()
{
    sptr<PowerMgr::IPowerStateCallback> powerStateCallback_(new DHFWKPowerStateCallback());
    if (powerStateCallback_ == nullptr) {
        DHLOGE("DHFWK subscribe create power state callback Create Error");
        return;
    }

    bool ret = PowerMgr::PowerMgrClient::GetInstance().RegisterPowerStateCallback(powerStateCallback_);
    if (!ret) {
        DHLOGE("DHFWK register power state callback failed");
    } else {
        DHLOGE("DHFWK register power state callback success");
    }
}

void DHContext::DHFWKPowerStateCallback::OnPowerStateChanged(PowerMgr::PowerState state)
{
    DHLOGI("DHFWK OnPowerStateChanged state: %{public}u", static_cast<uint32_t>(state));
    if (state == PowerMgr::PowerState::SLEEP || state == PowerMgr::PowerState::HIBERNATE ||
        state == PowerMgr::PowerState::SHUTDOWN) {
        DHLOGI("DHFWK set in sleeping");
        DHContext::GetInstance().SetIsSleeping(true);
        return;
    }

    DHLOGI("DHFWK set NOT in sleeping");
    DHContext::GetInstance().SetIsSleeping(false);
}

bool DHContext::IsSleeping()
{
    return isSleeping_;
}

void DHContext::SetIsSleeping(bool isSleeping)
{
    isSleeping_ = isSleeping;
}

DHContext::CommonEventHandler::CommonEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    DHLOGI("Ctor CommonEventHandler");
}

std::shared_ptr<DHContext::CommonEventHandler> DHContext::GetEventHandler()
{
    return eventHandler_;
}

bool DHContext::CommonEventHandler::PostTask(const Callback &callback, const std::string &name, int64_t delayTime)
{
    return AppExecFwk::EventHandler::PostTask(callback, name, delayTime);
}

void DHContext::CommonEventHandler::RemoveTask(const std::string &name)
{
    AppExecFwk::EventHandler::RemoveTask(name);
}

const DeviceInfo& DHContext::GetDeviceInfo()
{
    std::lock_guard<std::mutex> lock(devMutex_);
    if (!devInfo_.uuid.empty()) {
        return devInfo_;
    }
    devInfo_ = GetLocalDeviceInfo();
    return devInfo_;
}

void DHContext::AddOnlineDevice(const std::string &uuid, const std::string &networkId)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (onlineDeviceMap_.size() > MAX_ONLINE_DEVICE_SIZE || deviceIdUUIDMap_.size() > MAX_ONLINE_DEVICE_SIZE) {
        DHLOGE("OnlineDeviceMap or deviceIdUUIDMap is over size!");
        return;
    }
    onlineDeviceMap_[uuid] = networkId;
    deviceIdUUIDMap_[GetDeviceIdByUUID(uuid)] = uuid;
}

void DHContext::RemoveOnlineDevice(const std::string &uuid)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    auto iter = onlineDeviceMap_.find(uuid);
    if (iter != onlineDeviceMap_.end()) {
        onlineDeviceMap_.erase(iter);
        deviceIdUUIDMap_.erase(GetDeviceIdByUUID(uuid));
    }
}

bool DHContext::IsDeviceOnline(const std::string &uuid)
{
    std::shared_lock<std::shared_mutex> lock(onlineDevMutex_);
    return onlineDeviceMap_.find(uuid) != onlineDeviceMap_.end();
}

size_t DHContext::GetOnlineCount()
{
    std::shared_lock<std::shared_mutex> lock(onlineDevMutex_);
    return onlineDeviceMap_.size();
}

std::string DHContext::GetNetworkIdByUUID(const std::string &uuid)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (onlineDeviceMap_.find(uuid) == onlineDeviceMap_.end()) {
        DHLOGE("Can not find networkId, uuid: %{public}s", GetAnonyString(uuid).c_str());
        return "";
    }
    return onlineDeviceMap_[uuid];
}

std::string DHContext::GetUUIDByNetworkId(const std::string &networkId)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    auto iter = std::find_if(onlineDeviceMap_.begin(), onlineDeviceMap_.end(),
        [networkId](const auto &item) {return networkId.compare(item.second) == 0; });
    if (iter == onlineDeviceMap_.end()) {
        DHLOGE("Can not find uuid, networkId: %{public}s", GetAnonyString(networkId).c_str());
        return "";
    }
    return iter->first;
}

std::string DHContext::GetUUIDByDeviceId(const std::string &deviceId)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (deviceIdUUIDMap_.find(deviceId) == deviceIdUUIDMap_.end()) {
        DHLOGE("Can not find uuid, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
        return "";
    }
    return deviceIdUUIDMap_[deviceId];
}
} // namespace DistributedHardware
} // namespace OHOS
