/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "version_info_adapter.h"

#include "anonymous_string.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "event_bus.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_event.h"
#include "version_manager.h"

class DBAdapter;
namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "VersionInfoAdapter"

IMPLEMENT_SINGLE_INSTANCE(VersionInfoAdapter);

int32_t VersionInfoAdapter::Init()
{
    DHLOGI("VersionInfoAdapter instance init!");
    std::lock_guard<std::mutex> lock(verAdapterMutex_);
    dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, GLOBAL_VERSION_ID, VersionManager::GetInstance());
    if (dbAdapterPtr_->Init() != DH_FWK_SUCCESS) {
        DHLOGE("Init dbAdapterPtr_ failed");
        return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
    }

    DHLOGI("VersionInfoAdapter instance init success");
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoAdapter::UnInit()
{
    DHLOGI("VersionInfoAdapter UnInit");
    std::lock_guard<std::mutex> lock(verAdapterMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED;
    }
    dbAdapterPtr_->UnInit();
    dbAdapterPtr_.reset();
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoAdapter::AddVersion(const DHVersion &version)
{
    std::lock_guard<std::mutex> lock(verAdapterMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }

    std::string data("");
    dbAdapterPtr_->GetDataByKey(version.deviceId, data);
    if (data.compare(version.ToJsonString()) == 0) {
        DHLOGI("dhversion already stored, Key: %s", GetAnonyString(version.deviceId).c_str());
        return DH_FWK_SUCCESS;
    }

    std::string key = version.deviceId;
    std::string value = version.ToJsonString();
    DHLOGI("AddVersion, Key: %s", GetAnonyString(version.deviceId).c_str());
    if (dbAdapterPtr_->PutData(key, value) != DH_FWK_SUCCESS) {
        DHLOGE("Fail to storage to kv");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoAdapter::GetVersionInfoFromDB(const std::string &deviceId, DHVersion &dhVersion)
{
    DHLOGI("Sync VersionInfo from DB, deviceId: %s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(verAdapterMutex_);
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
    dhVersion.FromJsonString(data);
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoAdapter::SyncRemoteVersionInfos(std::unordered_map<std::string, DHVersion> &dhVersions)
{
    DHLOGI("Sync full remote version info from DB");
    std::vector<std::string> dataVector;
    {
        std::lock_guard<std::mutex> lock(verAdapterMutex_);
        if (dbAdapterPtr_ == nullptr) {
            DHLOGE("dbAdapterPtr_ is null");
            return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
        }
        
        if (dbAdapterPtr_->GetDataByKeyPrefix("", dataVector) != DH_FWK_SUCCESS) {
            DHLOGE("Query all data from DB failed");
            return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
        }
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
            DHLOGI("Find uuid failed");
            continue;
        }
        dhVersions.insert(std::pair<std::string, DHVersion>(uuid, dhVersion));
    }
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoAdapter::RemoveVersionInfoInDB(const std::string &deviceId)
{
    DHLOGI("Remove capability device info, deviceId: %s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(verAdapterMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    if (deviceId.empty()) {
        DHLOGE("RemoveCapabilityInfoInDB failed, deviceId is empty");
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (dbAdapterPtr_->RemoveDeviceData(deviceId) != DH_FWK_SUCCESS) {
        DHLOGE("Remove version Device Data failed, deviceId: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

void VersionInfoAdapter::CreateManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(verAdapterMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->CreateManualSyncCount(deviceId);
}

void VersionInfoAdapter::RemoveManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(verAdapterMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->RemoveManualSyncCount(deviceId);
}

int32_t VersionInfoAdapter::ManualSync(const std::string &networkId)
{
    DHLOGI("ManualSync start, networkId: %s", GetAnonyString(networkId).c_str());
    std::unique_lock<std::mutex> lock(verAdapterMutex_);
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
} // namespace DistributedHardware
} // namespace OHOS