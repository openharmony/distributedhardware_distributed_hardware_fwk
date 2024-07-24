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
IMPLEMENT_SINGLE_INSTANCE(VersionManager);

int32_t VersionManager::Init()
{
    DHLOGI("start");
    DHVersion dhVersion;
    int32_t ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalDHVersion fail");
        return ret;
    }
    dhVersion.dhVersion = GetLocalDeviceVersion();
    ShowLocalVersion(dhVersion);
    std::string strUUID = DHContext::GetInstance().GetDeviceInfo().uuid;
    AddDHVersion(strUUID, dhVersion);
    return DH_FWK_SUCCESS;
}

void VersionManager::UnInit()
{
    DHLOGI("start");
    dhVersions_.clear();
}

void VersionManager::ShowLocalVersion(const DHVersion &dhVersion) const
{
    for (const auto &item : dhVersion.compVersions) {
        DHLOGI("LocalDHVersion = %{public}s, DHtype = %{public}#X, handlerVersion = %{public}s, "
            "sourceVersion = %{public}s, sinkVersion = %{public}s",
            dhVersion.dhVersion.c_str(), item.first, item.second.handlerVersion.c_str(),
            item.second.sourceVersion.c_str(), item.second.sinkVersion.c_str());
    }
}

int32_t VersionManager::AddDHVersion(const std::string &uuid, const DHVersion &dhVersion)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("addDHVersion uuid: %{public}s", GetAnonyString(uuid).c_str());
    std::lock_guard<std::mutex> lock(versionMutex_);
    dhVersions_[uuid] = dhVersion;
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::RemoveDHVersion(const std::string &uuid)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("uuid: %{public}s", GetAnonyString(uuid).c_str());
    std::lock_guard<std::mutex> lock(versionMutex_);
    std::unordered_map<std::string, DHVersion>::iterator iter = dhVersions_.find(uuid);
    if (iter == dhVersions_.end()) {
        DHLOGE("there is no uuid: %{public}s, remove fail", GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST;
    }
    dhVersions_.erase(iter);
    return DH_FWK_SUCCESS;
}

int32_t VersionManager::GetDHVersion(const std::string &uuid, DHVersion &dhVersion)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("uuid: %{public}s", GetAnonyString(uuid).c_str());
    std::lock_guard<std::mutex> lock(versionMutex_);
    std::unordered_map<std::string, DHVersion>::iterator iter = dhVersions_.find(uuid);
    if (iter == dhVersions_.end()) {
        DHLOGE("there is no uuid: %{public}s, get version fail", GetAnonyString(uuid).c_str());
        return ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST;
    } else {
        dhVersion = dhVersions_[uuid];
        return DH_FWK_SUCCESS;
    }
}

int32_t VersionManager::GetCompVersion(const std::string &uuid, const DHType dhType, CompVersion &compVersion)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHVersion dhVersion;
    int32_t ret = GetDHVersion(uuid, dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetDHVersion fail, uuid: %{public}s", GetAnonyString(uuid).c_str());
        return ret;
    }
    if (dhVersion.compVersions.find(dhType) == dhVersion.compVersions.end()) {
        DHLOGE("not find dhType: %{public}#X", dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    DHLOGI("GetCompVersion success, uuid: %{public}s, dhType: %{public}#X", GetAnonyString(uuid).c_str(), dhType);
    compVersion = dhVersion.compVersions[dhType];
    return DH_FWK_SUCCESS;
}

std::string VersionManager::GetLocalDeviceVersion()
{
    return DH_LOCAL_VERSION;
}
} // namespace DistributedHardware
} // namespace OHOS
