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
class DBAdapter : public std::enable_shared_from_this<DBAdapter>,
    public EventSender,
    public DistributedKv::KvStoreDeathRecipient {
public:
    DBAdapter(const std::string &appId, const std::string &storeId,
        const std::shared_ptr<DistributedKv::KvStoreObserver> &changeListener);

    virtual ~DBAdapter();
    // default init auto-sync kv store
    int32_t Init(bool isAutoSync);
    // init local kv store
    int32_t InitLocal();
    void UnInit();
    int32_t ReInit(bool isAutoSync);
    int32_t GetDataByKey(const std::string &key, std::string &data);
    int32_t GetDataByKeyPrefix(const std::string &keyPrefix, std::vector<std::string> &values);
    int32_t PutData(const std::string &key, const std::string &value);
    int32_t PutDataBatch(const std::vector<std::string> &keys, const std::vector<std::string> &values);
    void SyncDBForRecover();
    virtual void OnRemoteDied() override;
    void DeleteKvStore();
    int32_t RemoveDeviceData(const std::string &deviceId);
    int32_t RemoveDataByKey(const std::string &key);
    std::vector<DistributedKv::Entry> GetEntriesByKeys(const std::vector<std::string> &keys);

private:
    int32_t RegisterChangeListener();
    int32_t UnRegisterChangeListener();
    void RegisterKvStoreDeathListener();
    void UnRegisterKvStoreDeathListener();
    // get default kv store with auto sync
    DistributedKv::Status GetKvStorePtr(bool isAutoSync);
    // get local kv store with no sync with other devices
    DistributedKv::Status GetLocalKvStorePtr();
    bool DBDiedOpt(int32_t &times);

private:
    DistributedKv::AppId appId_;
    DistributedKv::StoreId storeId_;
    DistributedKv::DistributedKvDataManager kvDataMgr_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStoragePtr_;
    std::shared_ptr<DistributedKv::KvStoreObserver> dataChangeListener_;
    std::mutex dbAdapterMutex_;
    bool isAutoSync {false};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
