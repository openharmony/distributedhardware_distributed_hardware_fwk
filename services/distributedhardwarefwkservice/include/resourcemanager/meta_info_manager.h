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

#ifndef OHOS_DISTRIBUTED_HARDWARE_META_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_META_INFO_MANAGER_H

#include <condition_variable>
#include <map>

#include "kvstore_observer.h"

#include "db_adapter.h"
#include "event_handler.h"
#include "meta_capability_info.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
class MetaInfoManager : public std::enable_shared_from_this<MetaInfoManager>,
                        public DistributedKv::KvStoreObserver {
public:
    MetaInfoManager(const MetaInfoManager &) = delete;
    MetaInfoManager &operator = (const MetaInfoManager &) = delete;
    MetaInfoManager(MetaInfoManager &&) = delete;
    MetaInfoManager &operator = (MetaInfoManager &&) = delete;
    static std::shared_ptr<MetaInfoManager> GetInstance();
    virtual ~MetaInfoManager();
    int32_t Init();
    int32_t UnInit();
    int32_t AddMetaCapInfos(const std::vector<std::shared_ptr<MetaCapabilityInfo>> &meatCapInfos);
    int32_t SyncMetaInfoFromDB(const std::string &udidHash);
    int32_t SyncRemoteMetaInfos();
    int32_t GetDataByKeyPrefix(const std::string &keyPrefix, MetaCapInfoMap &metaCapMap);
    int32_t RemoveMetaInfoByKey(const std::string &key);
    int32_t GetMetaCapInfo(const std::string &udidHash, const std::string &dhId,
        std::shared_ptr<MetaCapabilityInfo> &metaCapPtr);
    void GetMetaCapInfosByUdidHash(const std::string &udidHash,
        std::vector<std::shared_ptr<MetaCapabilityInfo>> &metaCapInfos);
    /* Database data changes callback */
    virtual void OnChange(const DistributedKv::ChangeNotification &changeNotification) override;
    /* Cloud data changes callback */
    void OnChange(const DistributedKv::DataOrigin &origin, Keys &&keys) override;

    class MetaInfoManagerEventHandler : public AppExecFwk::EventHandler {
        public:
            MetaInfoManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
                std::shared_ptr<MetaInfoManager> metaInfoMgrPtr);
            ~MetaInfoManagerEventHandler() override = default;
            void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
        private:
            std::weak_ptr<MetaInfoManager> metaInfoMgrWPtr_;
    };
    std::shared_ptr<MetaInfoManager::MetaInfoManagerEventHandler> GetEventHandler();

private:
    MetaInfoManager();
    int32_t GetMetaCapByValue(const std::string &value, std::shared_ptr<MetaCapabilityInfo> &metaCapPtr);
    void HandleMetaCapabilityAddChange(const std::vector<DistributedKv::Entry> &insertRecords);
    void HandleMetaCapabilityUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords);
    void HandleMetaCapabilityDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords);
    std::vector<DistributedKv::Entry> GetEntriesByKeys(const std::vector<std::string> &keys);
private:
    mutable std::mutex metaInfoMgrMutex_;
    std::shared_ptr<DBAdapter> dbAdapterPtr_;
    MetaCapInfoMap globalMetaInfoMap_;

    std::shared_ptr<MetaInfoManager::MetaInfoManagerEventHandler> eventHandler_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif