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

#include "versionmanager/version_manager.h"

#include "anonymous_string.h"
#include "componentloader/component_loader.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"
#include "version_info_adapter.h"

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
    VersionInfoEvent versionInfoEvent(*this);
    DHContext::GetInstance().GetEventBus()->AddHandler<VersionInfoEvent>(versionInfoEvent.GetType(), *this);
    VersionInfoAdapter::GetInstance().Init();

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
    AddDHVersion(dhVersion.uuid, dhVersion);
    VersionInfoAdapter::GetInstance().AddVersion(dhVersion);
    return DH_FWK_SUCCESS;
}

void VersionManager::UnInit()
{
    DHLOGI("start");
    VersionInfoAdapter::GetInstance().UnInit();
    dhVersions_.clear();
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
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::RemoveDHVersion(const std::string &uuid)
{
    DHLOGI("removeDHVersion uuid: %s", GetAnonyString(uuid).c_str());
    {
        std::lock_guard<std::mutex> lock(versionMutex_);
        std::unordered_map<std::string, DHVersion>::iterator iter = dhVersions_.find(uuid);
        if (iter == dhVersions_.end()) {
            DHLOGE("there is no uuid: %s, remove fail", GetAnonyString(uuid).c_str());
            return ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST;
        }
        dhVersions_.erase(iter);
    }
    VersionInfoAdapter::GetInstance().RemoveVersionInfoInDB(GetDeviceIdByUUID(uuid));
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::GetDHVersion(const std::string &uuid, DHVersion &dhVersion)
{
    DHLOGI("uuid: %s", GetAnonyString(uuid).c_str());
    {
        std::lock_guard<std::mutex> lock(versionMutex_);
        std::unordered_map<std::string, DHVersion>::iterator iter = dhVersions_.find(uuid);
        if (iter != dhVersions_.end()) {
            dhVersion = dhVersions_[uuid];
            return DH_FWK_SUCCESS;
        }
        DHLOGE("there is no uuid: %s in cache, get version fail", GetAnonyString(uuid).c_str());
    }
    int32_t ret = SyncDHVersionFromDB(uuid, dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("there is no uuid: %s in db, get version fail", GetAnonyString(uuid).c_str());
    }
    return ret;
}

int32_t VersionManager::SyncDHVersionFromDB(const std::string &uuid, DHVersion &dhVersion)
{
    DHLOGI("uuid: %s", GetAnonyString(uuid).c_str());
    int32_t ret = VersionInfoAdapter::GetInstance().GetVersionInfoFromDB(GetDeviceIdByUUID(uuid), dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        return ret;
    }
    DHLOGI("Sync DHVersion from db success, uuid: %s", GetAnonyString(uuid).c_str());

    std::lock_guard<std::mutex> lock(versionMutex_);
    dhVersions_[uuid] = dhVersion;
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::SyncRemoteVersionInfos()
{
    std::unordered_map<std::string, DHVersion> dhVersions;
    int32_t ret = VersionInfoAdapter::GetInstance().SyncRemoteVersionInfos(dhVersions);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGI("Sync remote all DHVersions fail");
        return ret;
    }

    std::lock_guard<std::mutex> lock(versionMutex_);
    for (auto dhVersion : dhVersions) {
        dhVersions_[dhVersion.first] = dhVersion.second;
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
            DHLOGI("Find uuid failed");
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
            DHLOGI("Find uuid failed");
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
            DHLOGI("Find uuid failed");
            continue;
        }
        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGI("Find network failed and never disable, uuid: %s", GetAnonyString(uuid).c_str());
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
