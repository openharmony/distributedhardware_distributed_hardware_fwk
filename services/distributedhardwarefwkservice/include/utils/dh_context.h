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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DHCONTEXT_H
#define OHOS_DISTRIBUTED_HARDWARE_DHCONTEXT_H

#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <shared_mutex>

#include "power_mgr_client.h"
#include "power_state_callback_stub.h"

#include "device_type.h"
#include "event_handler.h"
#include "ipublisher_listener.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
struct DeviceIdEntry {
    std::string networkId;
    std::string uuid;
    // deviceId is uuid hash
    std::string deviceId;
    std::string udid;
    std::string udidHash;

    bool operator == (const DeviceIdEntry &other) const
    {
        return (networkId == other.networkId) &&
            (uuid == other.uuid) &&
            (deviceId == other.deviceId) &&
            (udid == other.udid) &&
            (udidHash == other.udidHash);
    }

    bool operator < (const DeviceIdEntry &other) const
    {
        return (networkId.compare(other.networkId) < 0) ||
            ((networkId.compare(other.networkId) == 0) && (uuid.compare(other.uuid) < 0)) ||
            ((networkId.compare(other.networkId) == 0) && (uuid.compare(other.uuid) == 0) &&
            (deviceId.compare(other.deviceId) < 0)) ||
            ((networkId.compare(other.networkId) == 0) && (uuid.compare(other.uuid) == 0) &&
            (deviceId.compare(other.deviceId) == 0) && (udid.compare(other.udid) < 0)) ||
            ((networkId.compare(other.networkId) == 0) && (uuid.compare(other.uuid) == 0) &&
            (deviceId.compare(other.deviceId) == 0) && (udid.compare(other.udid) == 0) &&
            (udidHash.compare(other.udidHash) < 0));
    }
};

class DHContext {
DECLARE_SINGLE_INSTANCE_BASE(DHContext);
public:
    DHContext();
    ~DHContext();
    const DeviceInfo& GetDeviceInfo();

    /* Save online device UUID and networkId when devices online */
    void AddOnlineDevice(const std::string &udid, const std::string &uuid, const std::string &networkId);
    void RemoveOnlineDeviceIdEntryByNetworkId(const std::string &networkId);
    bool IsDeviceOnline(const std::string &uuid);
    size_t GetOnlineCount();
    std::string GetNetworkIdByUUID(const std::string &uuid);
    std::string GetNetworkIdByUDID(const std::string &udid);
    std::string GetUdidHashIdByUUID(const std::string &uuid);
    std::string GetUUIDByNetworkId(const std::string &networkId);
    std::string GetUDIDByNetworkId(const std::string &networkId);
    void AddRealTimeOnlineDeviceNetworkId(const std::string &networkId);
    void DeleteRealTimeOnlineDeviceNetworkId(const std::string &networkId);
    size_t GetRealTimeOnlineDeviceCount();
    /* DeviceId is which is hashed by sha256 */
    std::string GetUUIDByDeviceId(const std::string &deviceId);
    /**
     * @brief Get the Network Id By Device Id object
     *
     * @param deviceId the device form uuid hash
     * @return std::string the networkId for the deviceId
     */
    std::string GetNetworkIdByDeviceId(const std::string &deviceId);
    std::string GetDeviceIdByDBGetPrefix(const std::string &prefix);

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
    uint32_t GetIsomerismConnectCount();
    void AddIsomerismConnectDev(const std::string &IsomerismDeviceId);
    void DelIsomerismConnectDev(const std::string &IsomerismDeviceId);

private:
    class DHFWKPowerStateCallback : public OHOS::PowerMgr::PowerStateCallbackStub {
    public:
        void OnPowerStateChanged(OHOS::PowerMgr::PowerState state) override;
    };
    void RegisterPowerStateLinstener();

private:
    class DHFWKIsomerismListener : public IPublisherListener {
    public:
        DHFWKIsomerismListener();
        ~DHFWKIsomerismListener() override;
        void OnMessage(const DHTopic topic, const std::string &message) override;
        sptr<IRemoteObject> AsObject() override;
    };
    void RegisDHFWKIsomerismListener();
private:
    DeviceInfo devInfo_ { "", "", "", "", "", "", 0 };
    std::mutex devMutex_;

    std::set<DeviceIdEntry> devIdEntrySet_;
    std::shared_mutex onlineDevMutex_;

    std::set<std::string> realTimeOnLineNetworkIdSet_;
    std::shared_mutex realTimeNetworkIdMutex_;

    std::shared_ptr<DHContext::CommonEventHandler> eventHandler_;
    /* true for system in sleeping, false for NOT in sleeping */
    std::atomic<bool> isSleeping_ = false;

    std::unordered_set<std::string> connectedDevIds_;

    std::shared_mutex connectDevMutex_;
    };
} // namespace DistributedHardware
} // namespace OHOS
#endif
