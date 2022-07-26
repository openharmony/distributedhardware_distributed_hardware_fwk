/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dh_context.h"

#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DHContext);
DHContext::DHContext() : eventBus_(std::make_shared<EventBus>()) {}

DHContext::~DHContext()
{
    if (eventBus_ != nullptr) {
        eventBus_.reset();
        eventBus_ = nullptr;
    }
}

std::shared_ptr<EventBus> DHContext::GetEventBus()
{
    return eventBus_;
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
    if (!uuid.empty() && !networkId.empty()) {
        onlineDeviceMap_[uuid] = networkId;
        deviceIdUUIDMap_[GetDeviceIdByUUID(uuid)] = uuid;
    }
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
        DHLOGE("Can not find networkId, uuid: %s", GetAnonyString(uuid).c_str());
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
        DHLOGE("Can not find uuid, networkId: %s", GetAnonyString(networkId).c_str());
        return "";
    }
    return iter->first;
}

std::string DHContext::GetUUIDByDeviceId(const std::string &deviceId)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (deviceIdUUIDMap_.find(deviceId) == deviceIdUUIDMap_.end()) {
        DHLOGE("Can not find uuid, deviceId: %s", GetAnonyString(deviceId).c_str());
        return "";
    }
    return deviceIdUUIDMap_[deviceId];
}
} // namespace DistributedHardware
} // namespace OHOS
