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
    if (!devInfo_.deviceId.empty()) {
        return devInfo_;
    }
    devInfo_ = GetLocalDeviceInfo();
    return devInfo_;
}

void DHContext::AddOnlineDevice(const std::string &devId, const std::string &networkId)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (!devId.empty() && !networkId.empty()) {
        onlineDeviceMap_[devId] = networkId;
    }
}

void DHContext::RemoveOnlineDevice(const std::string &devId)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    auto iter = onlineDeviceMap_.find(devId);
    if (iter != onlineDeviceMap_.end()) {
        onlineDeviceMap_.erase(iter);
    }
}

bool DHContext::IsDeviceOnline(const std::string &devId)
{
    std::shared_lock<std::shared_mutex> lock(onlineDevMutex_);
    return onlineDeviceMap_.find(devId) != onlineDeviceMap_.end();
}

size_t DHContext::GetOnlineCount()
{
    std::shared_lock<std::shared_mutex> lock(onlineDevMutex_);
    return onlineDeviceMap_.size();
}

std::string DHContext::GetNetworkIdByUUID(const std::string &devId)
{
    std::unique_lock<std::shared_mutex> lock(onlineDevMutex_);
    if (onlineDeviceMap_.find(devId) == onlineDeviceMap_.end()) {
        DHLOGE("Can not find networkId in onlineDeviceMap_: %s", GetAnonyString(devId).c_str());
        return "";
    }
    return onlineDeviceMap_[devId];
}
}
}
