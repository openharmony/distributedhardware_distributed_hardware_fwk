/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "version_info_manager.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_event.h"
#include "version_manager.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "VersionInfoManager"

VersionInfoManager::VersionInfoManager() : dbAdapterPtr_(nullptr)
{}

VersionInfoManager::~VersionInfoManager()
{
    DHLOGI("VersionInfoManager Destruction!");
}

VersionInfoManager::VersionInfoManagerEventHandler::VersionInfoManagerEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, std::shared_ptr<VersionInfoManager> versionInfoMgrPtr)
    : AppExecFwk::EventHandler(runner)
{
    DHLOGI("Ctor VersionInfoManagerEventHandler");
    versionInfoMgrWPtr_ = versionInfoMgrPtr;
}

void VersionInfoManager::VersionInfoManagerEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    auto selfPtr = versionInfoMgrWPtr_.lock();
    if (!selfPtr) {
        DHLOGE("Can not get strong self ptr");
        return;
    }
    switch (eventId) {
        case EVENT_VERSION_INFO_DB_RECOVER:
            selfPtr->SyncRemoteVersionInfos();
            break;
        default:
            DHLOGE("event is undefined, id is %d", eventId);
            break;
    }
}

std::shared_ptr<VersionInfoManager::VersionInfoManagerEventHandler> VersionInfoManager::GetEventHandler()
{
    return this->eventHandler_;
}

std::shared_ptr<VersionInfoManager> VersionInfoManager::GetInstance()
{
    static std::shared_ptr<VersionInfoManager> instance(new(std::nothrow) VersionInfoManager);
    if (instance == nullptr) {
        DHLOGE("instance is nullptr, because applying memory fail!");
        return nullptr;
    }
    return instance;
}

int32_t VersionInfoManager::Init()
{
    DHLOGI("VersionInfoManager instance init!");
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, GLOBAL_VERSION_ID, shared_from_this());
    if (dbAdapterPtr_->Init() != DH_FWK_SUCCESS) {
        DHLOGE("Init dbAdapterPtr_ failed");
        return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
    }

    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<VersionInfoManager::VersionInfoManagerEventHandler>(
        runner, shared_from_this());
    DHLOGI("VersionInfoManager instance init success");
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoManager::UnInit()
{
    DHLOGI("VersionInfoManager UnInit");
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED;
    }
    dbAdapterPtr_->UnInit();
    dbAdapterPtr_.reset();
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoManager::AddVersion(const VersionInfo &versionInfo)
{
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }

    std::string data("");
    dbAdapterPtr_->GetDataByKey(versionInfo.deviceId, data);
    if (data.compare(versionInfo.ToJsonString()) == 0) {
        DHLOGI("dhversion already stored, Key: %s", GetAnonyString(versionInfo.deviceId).c_str());
        return DH_FWK_SUCCESS;
    }

    std::string key = versionInfo.deviceId;
    std::string value = versionInfo.ToJsonString();
    DHLOGI("AddVersion, Key: %s", GetAnonyString(versionInfo.deviceId).c_str());
    if (dbAdapterPtr_->PutData(key, value) != DH_FWK_SUCCESS) {
        DHLOGE("Fail to storage to kv");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoManager::GetVersionInfoByDeviceId(const std::string &deviceId, VersionInfo &versionInfo)
{
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::string data("");
    if (dbAdapterPtr_->GetDataByKey(deviceId, data) != DH_FWK_SUCCESS) {
        DHLOGE("Query data from DB by deviceId failed, deviceId: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return versionInfo.FromJsonString(data);
}

void VersionInfoManager::UpdateVersionCache(const VersionInfo &versionInfo)
{
    std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(versionInfo.deviceId);
    if (uuid.empty()) {
        DHLOGI("Find uuid failed, deviceId: %s", GetAnonyString(versionInfo.deviceId).c_str());
        return;
    }
    DHVersion dhVersion;
    dhVersion.uuid = uuid;
    dhVersion.dhVersion = versionInfo.dhVersion;
    dhVersion.compVersions = versionInfo.compVersions;
    VersionManager::GetInstance().AddDHVersion(uuid, dhVersion);
}

int32_t VersionInfoManager::RemoveVersionInfoByDeviceId(const std::string &deviceId)
{
    DHLOGI("Remove version device info, key: %s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }

    if (dbAdapterPtr_->RemoveDataByKey(deviceId) != DH_FWK_SUCCESS) {
        DHLOGE("Remove version info failed, key: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }

    std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    if (uuid.empty()) {
        DHLOGI("Find uuid failed, deviceId: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_UUID_NOT_FOUND;
    }
    DHLOGI("Delete version ,uuid: %s", GetAnonyString(uuid).c_str());
    VersionManager::GetInstance().RemoveDHVersion(uuid);

    return DH_FWK_SUCCESS;
}

int32_t VersionInfoManager::SyncVersionInfoFromDB(const std::string &deviceId)
{
    DHLOGI("Sync versionInfo from DB, deviceId: %s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::string data("");
    if (dbAdapterPtr_->GetDataByKey(deviceId, data) != DH_FWK_SUCCESS) {
        DHLOGE("Query data from DB by deviceId failed, deviceId: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }

    DHLOGI("Query data from DB by deviceId success, deviceId: %s", GetAnonyString(deviceId).c_str());
    VersionInfo versionInfo;
    int32_t ret = versionInfo.FromJsonString(data);
    if (ret != DH_FWK_SUCCESS) {
        return ret;
    }
    UpdateVersionCache(versionInfo);
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoManager::SyncRemoteVersionInfos()
{
    DHLOGI("Sync full remote version info from DB");
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix("", dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query all data from DB failed");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    if (dataVector.size() == 0 || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("DataVector Size is invalid!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        VersionInfo versionInfo;
        if (versionInfo.FromJsonString(data) != DH_FWK_SUCCESS) {
            continue;
        }
        const std::string &deviceId = versionInfo.deviceId;
        const std::string &localDeviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
        if (deviceId.compare(localDeviceId) == 0) {
            DHLOGE("Local device info not need sync from db");
            continue;
        }
        if (!DHContext::GetInstance().IsDeviceOnline(deviceId)) {
            DHLOGE("Offline device, no need sync to memory, deviceId : %s ",
                GetAnonyString(deviceId).c_str());
            continue;
        }
        UpdateVersionCache(versionInfo);
    }
    return DH_FWK_SUCCESS;
}

void VersionInfoManager::CreateManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->CreateManualSyncCount(deviceId);
}

void VersionInfoManager::RemoveManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->RemoveManualSyncCount(deviceId);
}

int32_t VersionInfoManager::ManualSync(const std::string &networkId)
{
    DHLOGI("ManualSync start, networkId: %s", GetAnonyString(networkId).c_str());
    std::unique_lock<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    if (dbAdapterPtr_->ManualSync(networkId) != DH_FWK_SUCCESS) {
        DHLOGE("ManualSync failed");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

void VersionInfoManager::OnChange(const DistributedKv::ChangeNotification &changeNotification)
{
    DHLOGI("DB data OnChange");
    if (!changeNotification.GetInsertEntries().empty() &&
        changeNotification.GetInsertEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle version data add change");
        HandleVersionAddChange(changeNotification.GetInsertEntries());
    }
    if (!changeNotification.GetUpdateEntries().empty() &&
        changeNotification.GetUpdateEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle version data update change");
        HandleVersionUpdateChange(changeNotification.GetUpdateEntries());
    }
    if (!changeNotification.GetDeleteEntries().empty() &&
        changeNotification.GetDeleteEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle version data delete change");
        HandleVersionDeleteChange(changeNotification.GetDeleteEntries());
    }
}

void VersionInfoManager::HandleVersionAddChange(const std::vector<DistributedKv::Entry> &insertRecords)
{
    DHLOGI("Version add change");
    for (const auto &item : insertRecords) {
        const std::string value = item.value.ToString();
        VersionInfo versionInfo;
        if (versionInfo.FromJsonString(value) != DH_FWK_SUCCESS) {
            continue;
        }
        UpdateVersionCache(versionInfo);
    }
}

void VersionInfoManager::HandleVersionUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords)
{
    DHLOGI("Version update change");
    for (const auto &item : updateRecords) {
        const std::string value = item.value.ToString();
        VersionInfo versionInfo;
        if (versionInfo.FromJsonString(value) != DH_FWK_SUCCESS) {
            continue;
        }
        UpdateVersionCache(versionInfo);
    }
}

void VersionInfoManager::HandleVersionDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords)
{
    DHLOGI("Version delete change");
    for (const auto &item : deleteRecords) {
        const std::string value = item.value.ToString();
        VersionInfo versionInfo;
        if (versionInfo.FromJsonString(value) != DH_FWK_SUCCESS) {
            continue;
        }
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(versionInfo.deviceId);
        if (uuid.empty()) {
            DHLOGI("Find uuid failed, deviceId: %s", GetAnonyString(versionInfo.deviceId).c_str());
            continue;
        }
        DHLOGI("Delete version ,uuid: %s", GetAnonyString(uuid).c_str());
        VersionManager::GetInstance().RemoveDHVersion(uuid);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
