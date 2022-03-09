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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DHCONTEXT_H
#define OHOS_DISTRIBUTED_HARDWARE_DHCONTEXT_H

#include <memory>
#include <string>
#include <unordered_set>
#include <shared_mutex>

#include "device_type.h"
#include "event_bus.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DHContext {
DECLARE_SINGLE_INSTANCE_BASE(DHContext);
public:
    DHContext();
    ~DHContext();
    std::shared_ptr<EventBus> GetEventBus();
    const DeviceInfo& GetDeviceInfo();

    /* Save online device UUID and networkId when devices online */
    void AddOnlineDevice(const std::string &uuid, const std::string &networkId);
    void RemoveOnlineDevice(const std::string &uuid);
    bool IsDeviceOnline(const std::string &uuid);
    size_t GetOnlineCount();
    std::string GetNetworkIdByUUID(const std::string &uuid);

    /* DeviceId is which is hashed by sha256 */
    std::string GetUUIDByDeviceId(const std::string &deviceId);

private:
    std::shared_ptr<EventBus> eventBus_;
    DeviceInfo devInfo_ { "", "", "", 0 };
    std::mutex devMutex_;

    /* Save online device uuid and networkId */
    std::unordered_map<std::string, std::string> onlineDeviceMap_;

    /* Save online device hashed uuid and uuid */
    std::unordered_map<std::string, std::string> deviceIdUUIDMap_;
    std::shared_mutex onlineDevMutex_;
};
}
}
#endif