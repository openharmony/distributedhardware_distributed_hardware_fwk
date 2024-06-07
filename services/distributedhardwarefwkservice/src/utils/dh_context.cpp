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
#include "cJSON.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "publisher.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DHContext);
DHContext::DHContext()
{
    DHLOGI("Ctor DHContext");
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DHContext::CommonEventHandler>(runner);
    RegisterPowerStateLinstener();
    RegisDHFWKIsomerismListener();
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

std::string DHContext::GetNetworkIdByDeviceId(const std::string &deviceId)
{
    std::string id = "";
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (deviceIdUUIDMap_.find(deviceId) == deviceIdUUIDMap_.end()) {
        DHLOGE("Can not find uuid, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
        return id;
    }

    // current id is uuid
    id = deviceIdUUIDMap_[deviceId];
    if (onlineDeviceMap_.find(id) == onlineDeviceMap_.end()) {
        DHLOGE("Can not find networkId, uuid: %{public}s", GetAnonyString(id).c_str());
        return "";
    }
    return onlineDeviceMap_[id];
}

std::string DHContext::GetDeviceIdByDBGetPrefix(const std::string &prefix)
{
    std::string id = "";
    if (prefix.empty()) {
        return id;
    }

    if (prefix.find(RESOURCE_SEPARATOR) != std::string::npos) {
        id = prefix.substr(0, prefix.find_first_of(RESOURCE_SEPARATOR));
    } else {
        id = prefix;
    }

    return id;
}

void DHContext::RegisDHFWKIsomerismListener()
{
    sptr<IPublisherListener> dhFwkIsomerismListener(new (std::nothrow) DHFWKIsomerismListener());
    if (dhFwkIsomerismListener == nullptr) {
        DHLOGE("dhFwkIsomerismListener Create Error");
        return;
    }
    Publisher::GetInstance().RegisterListener(DHTopic::TOPIC_ISOMERISM, dhFwkIsomerismListener);
}

void DHContext::DHFWKIsomerismListener::OnMessage(const DHTopic topic, const std::string &message)
{
    DHLOGI("OnMessage topic: %{public}u", static_cast<uint32_t>(topic));
    if (topic != DHTopic::TOPIC_ISOMERISM) {
        DHLOGE("OnMessage topic is wrong");
        return;
    }
    if (message.length() > MAX_MESSAGE_LEN) {
        DHLOGE("OnMessage error, message too large");
        return;
    }
    cJSON *messageJson = cJSON_Parse(message.c_str());
    if (messageJson == nullptr) {
        DHLOGE("OnMessage error, parse failed");
        return;
    }
    cJSON *eventObj = cJSON_GetObjectItemCaseSensitive(messageJson, ISOMERISM_EVENT_KEY.c_str());
    if (eventObj == nullptr || !IsString(messageJson, ISOMERISM_EVENT_KEY)) {
        cJSON_Delete(messageJson);
        DHLOGE("OnMessage event invaild");
        return;
    }
    cJSON *devObj = cJSON_GetObjectItemCaseSensitive(messageJson, DEV_ID.c_str());
    if (devObj == nullptr || !IsString(messageJson, DEV_ID)) {
        cJSON_Delete(messageJson);
        DHLOGE("OnMessage deviceId invaild");
        return;
    }
    std::string event = eventObj->valuestring;
    std::string deviceId = devObj->valuestring;
    cJSON_Delete(messageJson);
    if (event == ISOMERISM_EVENT_CONNECT_VAL) {
        DHContext::GetInstance().AddIsomerismConnectDev(deviceId);
    } else if (event == ISOMERISM_EVENT_DISCONNECT_VAL) {
        DHContext::GetInstance().DelIsomerismConnectDev(deviceId);
    }
    DHLOGI("OnMessage end");
}

void DHContext::AddIsomerismConnectDev(const std::string &IsomerismDeviceId)
{
    DHLOGI("AddIsomerismConnectDev id = %{public}s", GetAnonyString(IsomerismDeviceId).c_str());
    std::shared_lock<std::shared_mutex> lock(connectDevMutex_);
    connectedDevIds_.insert(IsomerismDeviceId);
}

void DHContext::DelIsomerismConnectDev(const std::string &IsomerismDeviceId)
{
    DHLOGI("DelIsomerismConnectDev id = %{public}s", GetAnonyString(IsomerismDeviceId).c_str());
    std::shared_lock<std::shared_mutex> lock(connectDevMutex_);
    if (connectedDevIds_.find(IsomerismDeviceId) == connectedDevIds_.end()) {
        DHLOGI("DelIsomerismConnectDev is not exist.");
        return;
    }
    connectedDevIds_.erase(IsomerismDeviceId);
}

uint32_t DHContext::GetIsomerismConnectCount()
{
    std::shared_lock<std::shared_mutex> lock(connectDevMutex_);
    return static_cast<uint32_t>(connectedDevIds_.size());
}

DHContext::DHFWKIsomerismListener::DHFWKIsomerismListener()
{
    DHLOGI("DHFWKIsomerismListener ctor");
}

DHContext::DHFWKIsomerismListener::~DHFWKIsomerismListener()
{
    DHLOGI("DHFWKIsomerismListener dtor");
}

sptr<IRemoteObject> DHContext::DHFWKIsomerismListener::AsObject()
{
    return nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
