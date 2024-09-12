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
    #ifdef POWER_MANAGER_ENABLE
    sptr<PowerMgr::IPowerStateCallback> powerStateCallback_(new DHFWKPowerStateCallback());
    if (powerStateCallback_ == nullptr) {
        DHLOGE("DHFWK subscribe create power state callback Create Error");
        return;
    }

    bool ret = PowerMgr::PowerMgrClient::GetInstance().RegisterPowerStateCallback(powerStateCallback_);
    if (!ret) {
        DHLOGE("DHFWK register power state callback failed");
    } else {
        DHLOGI("DHFWK register power state callback success");
    }
    #endif
}

#ifdef POWER_MANAGER_ENABLE
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
#endif

bool DHContext::IsSleeping()
{
    return isSleeping_;
}

void DHContext::SetIsSleeping(bool isSleeping)
{
    isSleeping_ = isSleeping;
}

DHContext::CommonEventHandler::CommonEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner)
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

void DHContext::AddOnlineDevice(const std::string &udid, const std::string &uuid, const std::string &networkId)
{
    if (!IsIdLengthValid(udid) || !IsIdLengthValid(uuid) || !IsIdLengthValid(networkId)) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (devIdEntrySet_.size() > MAX_ONLINE_DEVICE_SIZE) {
        DHLOGE("devIdEntrySet_ is over size!");
        return;
    }
    std::string deviceId = Sha256(uuid);
    std::string udidHash = Sha256(udid);
    DeviceIdEntry idEntry = {
        .networkId = networkId,
        .uuid = uuid,
        .deviceId = deviceId,
        .udid = udid,
        .udidHash = udidHash
    };
    devIdEntrySet_.insert(idEntry);
}

void DHContext::RemoveOnlineDeviceIdEntryByNetworkId(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->networkId == networkId) {
            devIdEntrySet_.erase(iter);
            break;
        }
    }
}

bool DHContext::IsDeviceOnline(const std::string &uuid)
{
    if (!IsIdLengthValid(uuid)) {
        return false;
    }
    std::shared_lock<std::shared_mutex> lock(onlineDevMutex_);
    bool flag = false;
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->uuid == uuid) {
            flag = true;
            break;
        }
    }
    return flag;
}

size_t DHContext::GetOnlineCount()
{
    std::shared_lock<std::shared_mutex> lock(onlineDevMutex_);
    return devIdEntrySet_.size();
}

std::string DHContext::GetNetworkIdByUUID(const std::string &uuid)
{
    if (!IsIdLengthValid(uuid)) {
        return "";
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    std::string networkId = "";
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->uuid == uuid) {
            networkId = iter->networkId;
            break;
        }
    }
    return networkId;
}

std::string DHContext::GetNetworkIdByUDID(const std::string &udid)
{
    if (!IsIdLengthValid(udid)) {
        return "";
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    std::string networkId = "";
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->udid == udid) {
            networkId = iter->networkId;
            break;
        }
    }
    return networkId;
}

std::string DHContext::GetUdidHashIdByUUID(const std::string &uuid)
{
    if (!IsIdLengthValid(uuid)) {
        return "";
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    std::string udidHash = "";
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->uuid == uuid) {
            udidHash = iter->udidHash;
            break;
        }
    }
    return udidHash;
}

std::string DHContext::GetUUIDByNetworkId(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return "";
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    std::string uuid = "";
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->networkId == networkId) {
            uuid = iter->uuid;
            break;
        }
    }
    return uuid;
}

std::string DHContext::GetUDIDByNetworkId(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return "";
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    std::string udid = "";
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->networkId == networkId) {
            udid = iter->udid;
            break;
        }
    }
    return udid;
}

std::string DHContext::GetUUIDByDeviceId(const std::string &deviceId)
{
    if (!IsIdLengthValid(deviceId)) {
        return "";
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    std::string uuid = "";
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->deviceId == deviceId || iter->udidHash == deviceId) {
            uuid = iter->uuid;
            break;
        }
    }
    return uuid;
}

std::string DHContext::GetNetworkIdByDeviceId(const std::string &deviceId)
{
    if (!IsIdLengthValid(deviceId)) {
        return "";
    }
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    std::string networkId = "";
    for (auto iter = devIdEntrySet_.begin(); iter != devIdEntrySet_.end(); iter++) {
        if (iter->deviceId == deviceId) {
            networkId = iter->networkId;
            break;
        }
    }
    return networkId;
}

std::string DHContext::GetDeviceIdByDBGetPrefix(const std::string &prefix)
{
    if (!IsIdLengthValid(prefix)) {
        return "";
    }
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

void DHContext::AddRealTimeOnlineDeviceNetworkId(const std::string &networkId)
{
    DHLOGI("AddRealTimeOnlineDeviceNetworkId: %{public}s", GetAnonyString(networkId).c_str());
    std::shared_lock<std::shared_mutex> lock(realTimeNetworkIdMutex_);
    realTimeOnLineNetworkIdSet_.insert(networkId);
}

void DHContext::DeleteRealTimeOnlineDeviceNetworkId(const std::string &networkId)
{
    DHLOGI("DeleteRealTimeOnlineDeviceNetworkId: %{public}s", GetAnonyString(networkId).c_str());
    std::shared_lock<std::shared_mutex> lock(realTimeNetworkIdMutex_);
    realTimeOnLineNetworkIdSet_.erase(networkId);
}

size_t DHContext::GetRealTimeOnlineDeviceCount()
{
    std::shared_lock<std::shared_mutex> lock(realTimeNetworkIdMutex_);
    return realTimeOnLineNetworkIdSet_.size();
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
    if (!IsMessageLengthValid(message)) {
        return;
    }
    DHLOGI("OnMessage topic: %{public}u", static_cast<uint32_t>(topic));
    if (topic != DHTopic::TOPIC_ISOMERISM) {
        DHLOGE("OnMessage topic is wrong");
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
    if (!IsIdLengthValid(IsomerismDeviceId)) {
        return;
    }
    DHLOGI("AddIsomerismConnectDev id = %{public}s", GetAnonyString(IsomerismDeviceId).c_str());
    std::shared_lock<std::shared_mutex> lock(connectDevMutex_);
    connectedDevIds_.insert(IsomerismDeviceId);
}

void DHContext::DelIsomerismConnectDev(const std::string &IsomerismDeviceId)
{
    if (!IsIdLengthValid(IsomerismDeviceId)) {
        return;
    }
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
