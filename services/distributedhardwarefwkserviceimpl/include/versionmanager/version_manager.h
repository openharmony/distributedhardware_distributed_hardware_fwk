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

#ifndef OHOS_DISTRIBUTED_HARDWARE_VERSION_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_VERSION_MANAGER_H

#include <mutex>
#include <vector>
#include <string>

#include "single_instance.h"
#include "distributed_hardware_errno.h"
#include "device_type.h"
#include "utils/impl_utils.h"

namespace OHOS {
namespace DistributedHardware {
const std::string DH_LOCAL_VERSION = "1.0";
class VersionManager {
    DECLARE_SINGLE_INSTANCE_BASE(VersionManager);

public:
    VersionManager() {}
    ~VersionManager() {}
    int32_t Init();
    void UnInit();
    int32_t AddDHVersion(const std::string &uuid, const DHVersion &dhVersion);
    int32_t RemoveDHVersion(const std::string &uuid);
    int32_t GetDHVersion(const std::string &uuid, DHVersion &dhVersion);
    int32_t GetCompVersion(const std::string &uuid, const DHType dhType, CompVersion &compVersion);
    std::string GetLocalDeviceVersion();
    void ShowLocalVersion(const DHVersion &dhVersion) const;

private:
    std::unordered_map<std::string, DHVersion> dhVersions_;
    std::mutex versionMutex_;
};
}
}
#endif