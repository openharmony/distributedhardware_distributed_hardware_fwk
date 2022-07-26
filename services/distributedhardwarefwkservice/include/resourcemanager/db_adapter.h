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

#ifndef OHOS_DISTRIBUTED_HARDWARE_DB_ADAPTER_H
#define OHOS_DISTRIBUTED_HARDWARE_DB_ADAPTER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "distributed_kv_data_manager.h"
#include "kvstore_observer.h"

#include "capability_info.h"
#include "event_sender.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t MAX_INIT_RETRY_TIMES = 20;
constexpr int32_t INIT_RETRY_SLEEP_INTERVAL = 200 * 1000; // 200ms
constexpr int32_t MANUAL_SYNC_TIMES = 6;
constexpr int32_t MANUAL_SYNC_INTERVAL = 100 * 1000; // 100ms
constexpr int32_t DIED_CHECK_MAX_TIMES = 300;
constexpr int32_t DIED_CHECK_INTERVAL = 100 * 1000; // 100ms
}
class DBAdapter : public std::enable_shared_from_this<DBAdapter>,
    public EventSender,
    public DistributedKv::KvStoreSyncCallback,
    public DistributedKv::KvStoreDeathRecipient {
public:
    DBAdapter(const std::string &appId, const std::string &storeId,
        const std::shared_ptr<DistributedKv::KvStoreObserver> &changeListener);

    virtual ~DBAdapter();

    int32_t Init();
    void UnInit();
    int32_t ReInit();
    void SyncCompleted(const std::map<std::string, DistributedKv::Status> &results) override;
    int32_t GetDataByKey(const std::string &key, std::string &data);
    int32_t GetDataByKeyPrefix(const std::string &keyPrefix, std::vector<std::string> &values);
    int32_t PutData(const std::string &key, std::string &value);
    int32_t PutDataBatch(const std::vector<std::string> &keys, const std::vector<std::string> &values);
    void CreateManualSyncCount(const std::string &deviceId);
    void RemoveManualSyncCount(const std::string &deviceId);
    int32_t ManualSync(const std::string &networkId);
    void SyncDBForRecover();
    virtual void OnRemoteDied() override;
    void DeleteKvStore();
    int32_t RemoveDeviceData(const std::string &deviceId);
    int32_t RemoveDataByKey(const std::string &key);

private:
    int32_t RegisterChangeListener();
    int32_t UnRegisterChangeListener();
    void RegisterKvStoreDeathListener();
    void UnRegisterKvStoreDeathListener();
    void RegisterManualSyncListener();
    void UnRegisterManualSyncListener();
    DistributedKv::Status GetKvStorePtr();

private:
    DistributedKv::AppId appId_;
    DistributedKv::StoreId storeId_;
    DistributedKv::DistributedKvDataManager kvDataMgr_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStoragePtr_;
    std::shared_ptr<DistributedKv::KvStoreObserver> dataChangeListener_;
    std::mutex dbAdapterMutex_;
    std::unordered_map<std::string, int32_t> manualSyncCountMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
