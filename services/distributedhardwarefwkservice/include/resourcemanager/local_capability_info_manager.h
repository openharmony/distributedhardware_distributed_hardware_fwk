/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_LOCAL_CAPABILITY_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_LOCAL_CAPABILITY_INFO_MANAGER_H

#include <condition_variable>
#include <map>
#include <set>

#include "kvstore_observer.h"

#include "capability_info.h"
#include "capability_utils.h"
#include "db_adapter.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
class LocalCapabilityInfoManager : public std::enable_shared_from_this<LocalCapabilityInfoManager>,
    public DistributedKv::KvStoreObserver {
public:
    LocalCapabilityInfoManager(const LocalCapabilityInfoManager &) = delete;
    LocalCapabilityInfoManager &operator = (const LocalCapabilityInfoManager &) = delete;
    LocalCapabilityInfoManager(LocalCapabilityInfoManager &&) = delete;
    LocalCapabilityInfoManager &operator = (LocalCapabilityInfoManager &&) = delete;
    static std::shared_ptr<LocalCapabilityInfoManager> GetInstance();
    LocalCapabilityInfoManager();
    virtual ~LocalCapabilityInfoManager();
    int32_t Init();
    int32_t UnInit();
    /* update the database record to memory */
    int32_t SyncDeviceInfoFromDB(const std::string &deviceId);
    /* Add Distributed hardware information, Save in memory and database */
    int32_t AddCapability(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos);
    /* Deleting Database Records by key */
    int32_t RemoveCapabilityInfoByKey(const std::string &key);
    void GetCapabilitiesByDeviceId(const std::string &deviceId,
        std::vector<std::shared_ptr<CapabilityInfo>> &resInfos);
    /* Queries capability information based on deviceId and dhId. */
    int32_t GetCapability(const std::string &deviceId, const std::string &dhId,
        std::shared_ptr<CapabilityInfo> &capPtr);
    int32_t GetDataByKey(const std::string &key, std::shared_ptr<CapabilityInfo> &capInfoPtr);
    /* Query batch records by dhtype */
    int32_t GetDataByDHType(const DHType dhType, CapabilityInfoMap &capabilityMap);
    /* Queries batch records in the database based on the prefix of the key. */
    int32_t GetDataByKeyPrefix(const std::string &keyPrefix, CapabilityInfoMap &capabilityMap);

private:
    mutable std::mutex capInfoMgrMutex_;
    std::shared_ptr<DBAdapter> dbAdapterPtr_;
    CapabilityInfoMap globalCapInfoMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
