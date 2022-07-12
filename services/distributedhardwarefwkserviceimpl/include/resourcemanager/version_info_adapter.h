/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_ADAPTER_H
#define OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_ADAPTER_H

#include <condition_variable>
#include <map>
#include <set>

#include "db_adapter.h"
#include "impl_utils.h"
#include "single_instance.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
class VersionInfoAdapter {
DECLARE_SINGLE_INSTANCE(VersionInfoAdapter);

public:
    int32_t Init();
    int32_t UnInit();

    int32_t AddVersion(const DHVersion &version);
    int32_t GetVersionInfoFromDB(const std::string &deviceId, DHVersion &dhVersion);
    int32_t SyncRemoteVersionInfos(std::unordered_map<std::string, DHVersion> &dhVersions);
    int32_t RemoveVersionInfoInDB(const std::string &deviceId);
    void CreateManualSyncCount(const std::string &deviceId);
    void RemoveManualSyncCount(const std::string &deviceId);
    int32_t ManualSync(const std::string &networkId);


private:
    mutable std::mutex verAdapterMutex_;
    std::shared_ptr<DBAdapter> dbAdapterPtr_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
