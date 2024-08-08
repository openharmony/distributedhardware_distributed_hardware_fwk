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

#ifndef OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_MANAGER_H

#include <condition_variable>
#include <map>
#include <set>

#include "kvstore_observer.h"

#include "capability_info.h"
#include "capability_utils.h"
#include "db_adapter.h"
#include "event_handler.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
class CapabilityInfoManager : public std::enable_shared_from_this<CapabilityInfoManager>,
                              public DistributedKv::KvStoreObserver {
public:
    CapabilityInfoManager(const CapabilityInfoManager &) = delete;
    CapabilityInfoManager &operator = (const CapabilityInfoManager &) = delete;
    CapabilityInfoManager(CapabilityInfoManager &&) = delete;
    CapabilityInfoManager &operator = (CapabilityInfoManager &&) = delete;
    static std::shared_ptr<CapabilityInfoManager> GetInstance();
    CapabilityInfoManager();
    virtual ~CapabilityInfoManager();
    int32_t Init();
    int32_t UnInit();
    /* update the database record to memory */
    int32_t SyncDeviceInfoFromDB(const std::string &deviceId);
    /* update the database record to memory in abnormal scene */
    int32_t SyncRemoteCapabilityInfos();
    /* Add Distributed hardware information, Save in memory and database */
    int32_t AddCapability(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos);
    /* Save CapabilityInfo in memory */
    int32_t AddCapabilityInMem(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos);
    /* Deleting Database Records */
    int32_t RemoveCapabilityInfoInDB(const std::string &deviceId);
    /* Deleting Database Records by key */
    int32_t RemoveCapabilityInfoByKey(const std::string &key);
    /* Delete data from memory cache */
    int32_t RemoveCapabilityInfoInMem(const std::string &deviceId);
    /* Queries distributed hardware information based on filter criteria. */
    std::map<std::string, std::shared_ptr<CapabilityInfo>> QueryCapabilityByFilters(
        const std::map<CapabilityInfoFilter, std::string> &filters);
    bool IsCapabilityMatchFilter(const std::shared_ptr<CapabilityInfo> &cap, const CapabilityInfoFilter &filter,
        const std::string &value);
    bool HasCapability(const std::string &deviceId, const std::string &dhId);
    void GetCapabilitiesByDeviceId(const std::string &deviceId,
        std::vector<std::shared_ptr<CapabilityInfo>> &resInfos);

    /* Queries capability information based on deviceId and dhId. */
    int32_t GetCapability(const std::string &deviceId, const std::string &dhId,
        std::shared_ptr<CapabilityInfo> &capPtr);
    int32_t GetDataByKey(const std::string &key, std::shared_ptr<CapabilityInfo>& capInfoPtr);
    /* Query batch records by dhtype */
    int32_t GetDataByDHType(const DHType dhType, CapabilityInfoMap &capabilityMap);
    /* Queries batch records in the database based on the prefix of the key. */
    int32_t GetDataByKeyPrefix(const std::string &keyPrefix, CapabilityInfoMap &capabilityMap);
    /* Database data changes callback */
    virtual void OnChange(const DistributedKv::ChangeNotification &changeNotification) override;
    /* Cloud data changes callback */
    void OnChange(const DistributedKv::DataOrigin &origin, Keys &&keys) override;

    class CapabilityInfoManagerEventHandler : public AppExecFwk::EventHandler {
        public:
            CapabilityInfoManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
                std::shared_ptr<CapabilityInfoManager> capabilityInfoMgrPtr);
            ~CapabilityInfoManagerEventHandler() override = default;
            void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
        private:
            std::weak_ptr<CapabilityInfoManager> capabilityInfoMgrWPtr_;
    };
    std::shared_ptr<CapabilityInfoManager::CapabilityInfoManagerEventHandler> GetEventHandler();

    void DumpCapabilityInfos(std::vector<CapabilityInfo> &capInfos);

private:
    void HandleCapabilityAddChange(const std::vector<DistributedKv::Entry> &insertRecords);
    void HandleCapabilityUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords);
    void HandleCapabilityDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords);
    std::vector<DistributedKv::Entry> GetEntriesByKeys(const std::vector<std::string> &keys);

private:
    mutable std::mutex capInfoMgrMutex_;
    std::shared_ptr<DBAdapter> dbAdapterPtr_;
    CapabilityInfoMap globalCapInfoMap_;

    std::shared_ptr<CapabilityInfoManager::CapabilityInfoManagerEventHandler> eventHandler_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
