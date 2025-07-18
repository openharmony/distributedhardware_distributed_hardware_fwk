/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MANAGER_FACTORY_H
#define OHOS_DISTRIBUTED_HARDWARE_MANAGER_FACTORY_H

#include <cstdint>
#include <mutex>
#include <unordered_map>

#include "device_type.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareManagerFactory {
    DECLARE_SINGLE_INSTANCE(DistributedHardwareManagerFactory);

public:
    bool InitLocalDevInfo();
    bool IsInit();
    int32_t SendOnLineEvent(const std::string &networkId, const std::string &uuid, const std::string &udid,
        uint16_t deviceType, int32_t osType);
    int32_t SendOffLineEvent(const std::string &networkId, const std::string &uuid, const std::string &udid,
        uint16_t deviceType);
    int32_t GetComponentVersion(std::unordered_map<DHType, std::string> &versionMap);
    void ClearRemoteDeviceMetaInfoData(const std::string &peerudid, const std::string &peeruuid);
    void ClearRemoteDeviceLocalInfoData(const std::string &peeruuid);

    int Dump(const std::vector<std::string> &argsStr, std::string &result);
    void UnInit();
    bool GetUnInitFlag();
    void SetSAProcessState(bool saState);
    bool GetSAProcessState();
    void CheckExitSAOrNot();
private:
    bool Init();
    void ExitDHFWK();

private:
    std::atomic<bool> isInit_ = false;
    std::atomic<bool> flagUnInit_ = false;
    std::atomic<bool> releaseStatus_ = false;
    std::mutex releaseProcessMutex_;
    std::atomic<bool> isIdle_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
