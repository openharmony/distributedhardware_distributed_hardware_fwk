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

#include <atomic>
#include <memory>
#include <string>
#include <unordered_set>
#include <shared_mutex>

#ifdef POWER_MANAGER_ENABLE
#include "power_mgr_client.h"
#include "power_state_callback_stub.h"
#endif

#include "device_type.h"
#include "event_handler.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DHContext {
DECLARE_SINGLE_INSTANCE_BASE(DHContext);
public:
    DHContext();
    ~DHContext();
    const DeviceInfo& GetDeviceInfo();

    /* Save online device UUID and networkId when devices online */
    void AddOnlineDevice(const std::string &uuid, const std::string &networkId);
    void RemoveOnlineDevice(const std::string &uuid);
    bool IsDeviceOnline(const std::string &uuid);
    size_t GetOnlineCount();
    std::string GetNetworkIdByUUID(const std::string &uuid);
    std::string GetUUIDByNetworkId(const std::string &networkId);

    /* DeviceId is which is hashed by sha256 */
    std::string GetUUIDByDeviceId(const std::string &deviceId);

    class CommonEventHandler : public AppExecFwk::EventHandler {
        public:
            CommonEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
            ~CommonEventHandler() override = default;

            bool PostTask(const Callback &callback, const std::string &name = std::string(), int64_t delayTime = 0);
            void RemoveTask(const std::string &name);
    };
    std::shared_ptr<DHContext::CommonEventHandler> GetEventHandler();

    bool IsSleeping();
    void SetIsSleeping(bool isSleeping);

private:
#ifdef POWER_MANAGER_ENABLE
    class DHFWKPowerStateCallback : public OHOS::PowerMgr::PowerStateCallbackStub {
    public:
        void OnPowerStateChanged(OHOS::PowerMgr::PowerState state) override;
    };
#endif
    void RegisterPowerStateLinstener();

private:
    DeviceInfo devInfo_ { "", "", "", 0 };
    std::mutex devMutex_;

    /* Save online device uuid and networkId */
    std::unordered_map<std::string, std::string> onlineDeviceMap_ = {};

    /* Save online device hashed uuid and uuid */
    std::unordered_map<std::string, std::string> deviceIdUUIDMap_ = {};
    std::shared_mutex onlineDevMutex_;

    std::shared_ptr<DHContext::CommonEventHandler> eventHandler_;
    /* true for system in sleeping, false for NOT in sleeping */
    std::atomic<bool> isSleeping_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
