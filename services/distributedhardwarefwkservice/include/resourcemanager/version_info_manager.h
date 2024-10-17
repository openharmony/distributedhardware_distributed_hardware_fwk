/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_MANAGER_H
#define OHOS_DISTRIBUTED_HARDWARE_VERSION_INFO_MANAGER_H

#include <condition_variable>
#include <map>
#include <set>

#include "kvstore_observer.h"

#include "db_adapter.h"
#include "event_handler.h"
#include "impl_utils.h"
#include "single_instance.h"
#include "version_info.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
class VersionInfoManager : public std::enable_shared_from_this<VersionInfoManager>,
                           public DistributedKv::KvStoreObserver {
public:
    VersionInfoManager(const VersionInfoManager &) = delete;
    VersionInfoManager &operator = (const VersionInfoManager &) = delete;
    VersionInfoManager(VersionInfoManager &&) = delete;
    VersionInfoManager &operator = (VersionInfoManager &&) = delete;
    static std::shared_ptr<VersionInfoManager> GetInstance();
    VersionInfoManager();
    virtual ~VersionInfoManager();

    int32_t Init();
    int32_t UnInit();

    int32_t AddVersion(const VersionInfo &versionInfo);
    int32_t GetVersionInfoByDeviceId(const std::string &deviceId, VersionInfo &versionInfo);
    int32_t RemoveVersionInfoByDeviceId(const std::string &deviceId);
    int32_t SyncVersionInfoFromDB(const std::string &deviceId);
    int32_t SyncRemoteVersionInfos();

    void OnChange(const DistributedKv::ChangeNotification &changeNotification) override;
    class VersionInfoManagerEventHandler : public AppExecFwk::EventHandler {
        public:
            VersionInfoManagerEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner,
                std::shared_ptr<VersionInfoManager> versionInfoMgrPtr);
            ~VersionInfoManagerEventHandler() override = default;
            void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
        private:
            std::weak_ptr<VersionInfoManager> versionInfoMgrWPtr_;
    };
    std::shared_ptr<VersionInfoManager::VersionInfoManagerEventHandler> GetEventHandler();

private:
    void UpdateVersionCache(const VersionInfo &versionInfo);
    void HandleVersionAddChange(const std::vector<DistributedKv::Entry> &insertRecords);
    void HandleVersionUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords);
    void HandleVersionDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords);

private:
    mutable std::mutex verInfoMgrMutex_;
    std::shared_ptr<DBAdapter> dbAdapterPtr_;
    std::shared_ptr<VersionInfoManager::VersionInfoManagerEventHandler> eventHandler_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
