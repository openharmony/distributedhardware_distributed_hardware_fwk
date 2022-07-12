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

#include "version_manager.h"

#include "anonymous_string.h"
#include "component_loader.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "VersionManager"

VersionManager::~VersionManager()
{
    DHLOGI("VersionManager Destruction!");
}

std::shared_ptr<VersionManager> VersionManager::GetInstance()
{
    static std::shared_ptr<VersionManager> instance(new(std::nothrow) VersionManager);
    if (instance == nullptr) {
        DHLOGE("instance is nullptr, because applying memory fail!");
        return nullptr;
    }
    return instance;
}

int32_t VersionManager::Init()
{
    DHLOGI("start");
    {
        std::lock_guard<std::mutex> lock(versionMutex_);
        dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, GLOBAL_VERSION_ID, shared_from_this());
        if (dbAdapterPtr_->Init() != DH_FWK_SUCCESS) {
            DHLOGE("Init dbAdapterPtr_ failed");
            return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
        }
    }

    VersionInfoEvent versionInfoEvent(*this);
    DHContext::GetInstance().GetEventBus()->AddHandler<VersionInfoEvent>(versionInfoEvent.GetType(), *this);
    int32_t ret = AddLocalVersion();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("AddLocalVersion fail");
        return ret;
    }

    return DH_FWK_SUCCESS;
}

void VersionManager::UnInit()
{
    DHLOGI("start");
    dhVersions_.clear();
}

int32_t VersionManager::AddLocalVersion()
{
    DHVersion dhVersion;
    int32_t ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalDHVersion fail");
        return ret;
    }
    dhVersion.dhVersion = GetLocalDeviceVersion();
    dhVersion.uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    dhVersion.deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    ShowLocalVersion(dhVersion);
    ret = AddDHVersion(dhVersion.uuid, dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("AddDHVersion fail");
        return ret;
    }
    return DH_FWK_SUCCESS;
}

void VersionManager::ShowLocalVersion(const DHVersion &dhVersion) const
{
    for (const auto &item : dhVersion.compVersions) {
        DHLOGI("LocalDHVersion = %s, DHtype = %#X, handlerVersion = %s, sourceVersion = %s, sinkVersion = %s",
            dhVersion.dhVersion.c_str(), item.first, item.second.handlerVersion.c_str(),
            item.second.sourceVersion.c_str(), item.second.sinkVersion.c_str());
    }
}

int32_t VersionManager::AddDHVersion(const std::string &uuid, const DHVersion &dhVersion)
{
    DHLOGI("addDHVersion uuid: %s", GetAnonyString(uuid).c_str());
    std::lock_guard<std::mutex> lock(versionMutex_);
    dhVersions_[uuid] = dhVersion;

    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }

    std::string data("");
    dbAdapterPtr_->GetDataByKey(dhVersion.deviceId, data);
    if (data.compare(dhVersion.ToJsonString()) == 0) {
        DHLOGI("dhversion already stored, Key: %s", GetAnonyString(dhVersion.deviceId).c_str());
        return DH_FWK_SUCCESS;
    }

    std::string key = dhVersion.deviceId;
    std::string value = dhVersion.ToJsonString();
    DHLOGI("add version to db, Key: %s", GetAnonyString(dhVersion.deviceId).c_str());
    if (dbAdapterPtr_->PutData(key, value) != DH_FWK_SUCCESS) {
        DHLOGE("Fail to storage to kv");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::RemoveDHVersion(const std::string &uuid)
{
    DHLOGI("removeDHVersion uuid: %s", GetAnonyString(uuid).c_str());

    std::lock_guard<std::mutex> lock(versionMutex_);
    std::unordered_map<std::string, DHVersion>::iterator iter = dhVersions_.find(uuid);
    if (iter == dhVersions_.end()) {
        DHLOGE("there is no uuid: %s, remove in cache fail", GetAnonyString(uuid).c_str());
    } else {
        dhVersions_.erase(iter);
    }

    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }

    if (dbAdapterPtr_->RemoveDeviceData(GetDeviceIdByUUID(uuid)) != DH_FWK_SUCCESS) {
        DHLOGE("Remove version Device Data failed, deviceId: %s",
            GetAnonyString(GetDeviceIdByUUID(uuid)).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::GetDHVersion(const std::string &uuid, DHVersion &dhVersion)
{
    DHLOGI("uuid: %s", GetAnonyString(uuid).c_str());
    std::lock_guard<std::mutex> lock(versionMutex_);
    std::unordered_map<std::string, DHVersion>::iterator iter = dhVersions_.find(uuid);
    if (iter != dhVersions_.end()) {
        dhVersion = dhVersions_[uuid];
        return DH_FWK_SUCCESS;
    }
    DHLOGI("there is no uuid: %s in cache, get version in db", GetAnonyString(uuid).c_str());

    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::string data("");
    if (dbAdapterPtr_->GetDataByKey(GetDeviceIdByUUID(uuid), data) != DH_FWK_SUCCESS) {
        DHLOGE("Query data from DB by deviceId failed, deviceId: %s",
            GetAnonyString(GetDeviceIdByUUID(uuid)).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }

    DHLOGI("Query data from DB by deviceId success, deviceId: %s",
        GetAnonyString(GetDeviceIdByUUID(uuid)).c_str());
    dhVersion.FromJsonString(data);
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::SyncDHVersionFromDB(const std::string &uuid)
{
    DHLOGI("uuid: %s", GetAnonyString(uuid).c_str());
    std::lock_guard<std::mutex> lock(versionMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }

    std::string data("");
    if (dbAdapterPtr_->GetDataByKey(GetDeviceIdByUUID(uuid), data) != DH_FWK_SUCCESS) {
        DHLOGE("Query data from DB by deviceId failed, deviceId: %s",
            GetAnonyString(GetDeviceIdByUUID(uuid)).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }

    DHLOGI("Query data from DB by deviceId success, deviceId: %s",
        GetAnonyString(GetDeviceIdByUUID(uuid)).c_str());
    DHVersion dhVersion;
    dhVersion.FromJsonString(data);
    dhVersions_[uuid] = dhVersion;
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::SyncRemoteVersionInfos()
{
    std::unordered_map<std::string, DHVersion> dhVersions;
    std::vector<std::string> dataVector;
    std::lock_guard<std::mutex> lock(versionMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    
    if (dbAdapterPtr_->GetDataByKeyPrefix("", dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query all data from DB failed");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }

    for (const auto &data : dataVector) {
        DHVersion dhVersion;
        dhVersion.FromJsonString(data);
        const std::string &deviceId = dhVersion.deviceId;
        const std::string &localDeviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
        if (deviceId.compare(localDeviceId) == 0) {
            DHLOGE("local device info not need sync from db");
            continue;
        }
        if (!DHContext::GetInstance().IsDeviceOnline(deviceId)) {
            DHLOGE("offline device, no need sync to memory, deviceId : %s ",
                GetAnonyString(deviceId).c_str());
            continue;
        }
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
        if (uuid.empty()) {
            DHLOGI("Find uuid failed, deviceId: %s", GetAnonyString(deviceId).c_str());
            continue;
        }
        dhVersions_[uuid] = dhVersion;
    }

    return DH_FWK_SUCCESS;
}

int32_t VersionManager::GetCompVersion(const std::string &uuid, const DHType dhType, CompVersion &compVersion)
{
    DHVersion dhVersion;
    int32_t ret = GetDHVersion(uuid, dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetDHVersion fail, uuid: %s", GetAnonyString(uuid).c_str());
        return ret;
    }
    if (dhVersion.compVersions.find(dhType) == dhVersion.compVersions.end()) {
        DHLOGE("not find dhType: %#X", dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    DHLOGI("GetCompVersion success, uuid: %s, dhType: %#X", GetAnonyString(uuid).c_str(), dhType);
    compVersion = dhVersion.compVersions[dhType];
    return DH_FWK_SUCCESS;
}

std::string VersionManager::GetLocalDeviceVersion()
{
    return DH_LOCAL_VERSION;
}

void VersionManager::CreateManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(versionMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->CreateManualSyncCount(deviceId);
}

void VersionManager::RemoveManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(versionMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->RemoveManualSyncCount(deviceId);
}

int32_t VersionManager::ManualSync(const std::string &networkId)
{
    DHLOGI("ManualSync start, networkId: %s", GetAnonyString(networkId).c_str());
    std::unique_lock<std::mutex> lock(versionMutex_);
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

void VersionManager::OnChange(const DistributedKv::ChangeNotification &changeNotification)
{
    DHLOGI("VersionManager: DB data OnChange");
    if (!changeNotification.GetInsertEntries().empty()) {
        DHLOGI("Handle version data add change");
        HandleVersionAddChange(changeNotification.GetInsertEntries());
    }
    if (!changeNotification.GetUpdateEntries().empty()) {
        DHLOGI("Handle version data update change");
        HandleVersionUpdateChange(changeNotification.GetUpdateEntries());
    }
    if (!changeNotification.GetDeleteEntries().empty()) {
        DHLOGI("Handle capability data delete change");
        HandleVersionDeleteChange(changeNotification.GetDeleteEntries());
    }
}

void VersionManager::HandleVersionAddChange(const std::vector<DistributedKv::Entry> &insertRecords)
{
    DHLOGI("VersionManager: Version add change");
    for (const auto &item : insertRecords) {
        const std::string value = item.value.ToString();
        DHVersion dhVersion;
        dhVersion.FromJsonString(value);
        const std::string &deviceId = dhVersion.deviceId;
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
        if (uuid.empty()) {
            DHLOGI("Find uuid failed, deviceId: %s", GetAnonyString(deviceId).c_str());
            continue;
        }
        DHLOGI("Add Version ,uuid: %s", GetAnonyString(uuid).c_str());
        AddDHVersion(uuid, dhVersion);
    }
}

void VersionManager::HandleVersionUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords)
{
    DHLOGI("VersionManager: Version update change");
    for (const auto &item : updateRecords) {
        const std::string value = item.value.ToString();
        DHVersion dhVersion;
        dhVersion.FromJsonString(value);
        const std::string &deviceId = dhVersion.deviceId;
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
        if (uuid.empty()) {
            DHLOGI("Find uuid failed, deviceId: %s", GetAnonyString(deviceId).c_str());
            continue;
        }
        DHLOGI("Update Version ,uuid: %s", GetAnonyString(uuid).c_str());
        AddDHVersion(uuid, dhVersion);
    }
}

void VersionManager::HandleVersionDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords)
{
    DHLOGI("VersionManager: Version delete change");
    for (const auto &item : deleteRecords) {
        const std::string value = item.value.ToString();
        DHVersion dhVersion;
        dhVersion.FromJsonString(value);
        const std::string &deviceId = dhVersion.deviceId;
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
        if (uuid.empty()) {
            DHLOGI("Find uuid failed, deviceId: %s", GetAnonyString(deviceId).c_str());
            continue;
        }

        DHLOGI("Delete Version ,uuid: %s", GetAnonyString(uuid).c_str());
        RemoveDHVersion(uuid);
    }
}

void VersionManager::OnEvent(VersionInfoEvent &ev)
{
    switch (ev.GetAction()) {
        case VersionInfoEvent::EventType::RECOVER:
            SyncRemoteVersionInfos();
            break;
        default:
            DHLOGE("Event is undefined, type is %d", ev.GetAction());
            break;
    }
}
} // namespace DistributedHardware
} // namespace OHOS
