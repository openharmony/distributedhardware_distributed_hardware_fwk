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

#ifndef OHOS_DISTRIBUTED_HARDWARE_ACCESS_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_ACCESS_MANAGER_H

#include <cstdint>
#include <string>
#include <memory>
#include <mutex>
#include <vector>

#include "device_manager_callback.h"
#include "dm_device_info.h"

namespace OHOS {
namespace DistributedHardware {
class AccessManager : public std::enable_shared_from_this<AccessManager>,
    public DmInitCallback,
    public DeviceStateCallback,
    public DevTrustChangeCallback {
public:
    AccessManager(const AccessManager &) = delete;
    AccessManager &operator = (const AccessManager &) = delete;
    AccessManager(AccessManager &&) = delete;
    AccessManager &operator = (AccessManager &&) = delete;
    AccessManager() = default;
    virtual ~AccessManager();
    static std::shared_ptr<AccessManager> GetInstance();
    int32_t Init();
    int32_t UnInit();
    bool IsDeviceOnline(const std::string &uuid);
    virtual void OnRemoteDied() override;
    virtual void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override;
    virtual void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override;
    virtual void OnDeviceReady(const DmDeviceInfo &deviceInfo) override;
    virtual void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override;
    virtual void OnDeviceTrustChange(const std::string &peerudid, const std::string &peeruuid,
        DmAuthForm authform) override;
    /* Send device online event which is already online */
    void CheckTrustedDeviceOnline();
    int32_t Dump(const std::vector<std::string> &argsStr, std::string &result);

private:
    int32_t RegisterDevStateCallback();
    int32_t UnRegisterDevStateCallback();
    int32_t InitDeviceManager();
    int32_t UnInitDeviceManager();
    int32_t RegDevTrustChangeCallback();
    std::mutex accessMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
