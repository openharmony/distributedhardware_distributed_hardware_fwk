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

#include "kvstore_observer.h"

#include "db_adapter.h"
#include "eventbus_handler.h"
#include "event_bus.h"
#include "event_sender.h"
#include "distributed_hardware_errno.h"
#include "device_type.h"
#include "impl_utils.h"
#include "version_info_event.h"

namespace OHOS {
namespace DistributedHardware {
const std::string DH_LOCAL_VERSION = "1.0";
class VersionManager : public std::enable_shared_from_this<VersionManager>,
                       public EventSender,
                       public DistributedKv::KvStoreObserver,
                       public EventBusHandler<VersionInfoEvent> {
public:
    VersionManager(const VersionManager &) = delete;
    VersionManager &operator = (const VersionManager &) = delete;
    VersionManager(VersionManager &&) = delete;
    VersionManager &operator = (VersionManager &&) = delete;
    static std::shared_ptr<VersionManager> GetInstance();
    virtual ~VersionManager();

    int32_t Init();
    void UnInit();
    int32_t AddDHVersion(const std::string &uuid, const DHVersion &dhVersion);
    int32_t RemoveDHVersion(const std::string &uuid);
    int32_t GetDHVersion(const std::string &uuid, DHVersion &dhVersion);
    int32_t GetCompVersion(const std::string &uuid, const DHType dhType, CompVersion &compVersion);
    int32_t SyncDHVersionFromDB(const std::string &uuid);
    int32_t SyncRemoteVersionInfos();
    std::string GetLocalDeviceVersion();
    int32_t AddLocalVersion();
    void ShowLocalVersion(const DHVersion &dhVersion) const;

    void CreateManualSyncCount(const std::string &deviceId);
    void RemoveManualSyncCount(const std::string &deviceId);
    int32_t ManualSync(const std::string &networkId);

    void OnChange(const DistributedKv::ChangeNotification &changeNotification) override;
    void OnEvent(VersionInfoEvent &ev) override;

private:
    VersionManager();
    void HandleVersionAddChange(const std::vector<DistributedKv::Entry> &insertRecords);
    void HandleVersionUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords);
    void HandleVersionDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords);

private:
    std::unordered_map<std::string, DHVersion> dhVersions_;
    std::shared_ptr<DBAdapter> dbAdapterPtr_;
    std::mutex versionMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
