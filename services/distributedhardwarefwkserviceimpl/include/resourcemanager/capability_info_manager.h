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

#ifndef OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_CAPABILITY_INFO_MANAGER_H

#include <condition_variable>
#include <map>
#include <set>

#include "kvstore_observer.h"

#include "capability_info.h"
#include "capability_info_event.h"
#include "capability_utils.h"
#include "db_adapter.h"
#include "event.h"
#include "eventbus_handler.h"
#include "event_bus.h"
#include "event_sender.h"
#include "single_instance.h"

using OHOS::DistributedKv::Entry;

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
namespace {
    constexpr int32_t MANUAL_SYNC_TIMEOUT = 1;
}
class CapabilityInfoManager : public std::enable_shared_from_this<CapabilityInfoManager>,
                              public EventSender,
                              public DistributedKv::KvStoreObserver,
                              public EventBusHandler<CapabilityInfoEvent> {
public:
    CapabilityInfoManager(const CapabilityInfoManager &) = delete;
    CapabilityInfoManager &operator = (const CapabilityInfoManager &) = delete;
    CapabilityInfoManager(CapabilityInfoManager &&) = delete;
    CapabilityInfoManager &operator = (CapabilityInfoManager &&) = delete;
    static std::shared_ptr<CapabilityInfoManager> GetInstance();
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
    /* Queries batch records in the database based on the prefix of the key. */
    int32_t GetDataByKeyPrefix(const std::string &keyPrefix, CapabilityInfoMap &capabilityMap);
    /* Init the count of manual sync times */
    void CreateManualSyncCount(const std::string &deviceId);
    /* Clearing the count of manual sync times */
    void RemoveManualSyncCount(const std::string &deviceId);
    /* Actively synchronizes data */
    int32_t ManualSync(const std::string &networkId);
    /* Manual sync notify */
    void NotifySyncCompleted();
    /* Database data changes callback */
    virtual void OnChange(const DistributedKv::ChangeNotification &changeNotification) override;
    virtual void OnChange(const DistributedKv::ChangeNotification &changeNotification,
        std::shared_ptr<DistributedKv::KvStoreSnapshot> snapshot) override;
    /* EventBus async processing callback */
    void OnEvent(CapabilityInfoEvent &e) override;

private:
    CapabilityInfoManager();
    void HandleCapabilityAddChange(const std::vector<Entry> &insertRecords);
    void HandleCapabilityUpdateChange(const std::vector<Entry> &updateRecords);
    void HandleCapabilityDeleteChange(const std::vector<Entry> &deleteRecords);

private:
    mutable std::mutex capInfoMgrMutex_;
    int32_t manualSyncResult_;
    std::condition_variable manualSyncCondVar_;
    std::shared_ptr<DBAdapter> dbAdapterPtr_;
    CapabilityInfoMap globalCapInfoMap_;
};
}
}
#endif