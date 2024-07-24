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

#include "local_capability_info_manager.h"

#include "anonymous_string.h"
#include "capability_utils.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "LocalCapabilityInfoManager"

LocalCapabilityInfoManager::LocalCapabilityInfoManager() : dbAdapterPtr_(nullptr)
{
    DHLOGI("LocalCapabilityInfoManager construction!");
}

LocalCapabilityInfoManager::~LocalCapabilityInfoManager()
{
    DHLOGI("LocalCapabilityInfoManager destruction!");
}

std::shared_ptr<LocalCapabilityInfoManager> LocalCapabilityInfoManager::GetInstance()
{
    static std::shared_ptr<LocalCapabilityInfoManager> instance(new(std::nothrow) LocalCapabilityInfoManager);
    if (instance == nullptr) {
        DHLOGE("instance is nullptr, because applying memory fail!");
        return nullptr;
    }
    return instance;
}

int32_t LocalCapabilityInfoManager::Init()
{
    DHLOGI("LocalCapabilityInfoManager instance init!");
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, LOCAL_CAPABILITY_ID, shared_from_this());
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    if (dbAdapterPtr_->InitLocal() != DH_FWK_SUCCESS) {
        DHLOGE("Init dbAdapterPtr_ failed");
        return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
    }
    DHLOGI("LocalCapabilityInfoManager instance init success");
    return DH_FWK_SUCCESS;
}

int32_t LocalCapabilityInfoManager::UnInit()
{
    DHLOGI("LocalCapabilityInfoManager UnInit");
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED;
    }
    dbAdapterPtr_->UnInit();
    dbAdapterPtr_.reset();
    return DH_FWK_SUCCESS;
}

int32_t LocalCapabilityInfoManager::SyncDeviceInfoFromDB(const std::string &deviceId)
{
    if (!IsIdLengthValid(deviceId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Sync DeviceInfo from DB, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix(deviceId, dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query data from DB by deviceId failed, id: %{public}s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    if (dataVector.empty() || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("On dataVector error, maybe empty or too large.");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<CapabilityInfo> capabilityInfo;
        if (GetCapabilityByValue<CapabilityInfo>(data, capabilityInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        globalCapInfoMap_[capabilityInfo->GetKey()] = capabilityInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t LocalCapabilityInfoManager::AddCapability(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    if (resInfos.empty() || resInfos.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("resInfo is empty or too large!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> keys;
    std::vector<std::string> values;
    std::string key;
    for (auto &resInfo : resInfos) {
        if (resInfo == nullptr) {
            continue;
        }
        key = resInfo->GetKey();
        globalCapInfoMap_[key] = resInfo;
        DHLOGI("AddCapability, Key: %{public}s", resInfo->GetAnonymousKey().c_str());
        keys.push_back(key);
        values.push_back(resInfo->ToJsonString());
    }
    if (keys.empty() || values.empty()) {
        DHLOGD("Records are empty, No need add data to db!");
        return DH_FWK_SUCCESS;
    }
    if (dbAdapterPtr_->PutDataBatch(keys, values) != DH_FWK_SUCCESS) {
        DHLOGE("Fail to storage batch to kv");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t LocalCapabilityInfoManager::RemoveCapabilityInfoByKey(const std::string &key)
{
    if (!IsKeySizeValid(key)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Remove capability device info, key: %{public}s", GetAnonyString(key).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    // 1. Clear the cache in the memory.
    globalCapInfoMap_.erase(key);

    // 2. Delete the corresponding record from the database.(use key)
    if (dbAdapterPtr_->RemoveDataByKey(key) != DH_FWK_SUCCESS) {
        DHLOGE("Remove capability Device Data failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

void LocalCapabilityInfoManager::GetCapabilitiesByDeviceId(const std::string &deviceId,
    std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    if (!IsIdLengthValid(deviceId) || resInfos.empty() || resInfos.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("On parameter error, maybe empty or too large!");
        return;
    }
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (auto &capabilityInfo : globalCapInfoMap_) {
        if (IsCapKeyMatchDeviceId(capabilityInfo.first, deviceId)) {
            resInfos.emplace_back(capabilityInfo.second);
        }
    }
}

int32_t LocalCapabilityInfoManager::GetCapability(const std::string &deviceId, const std::string &dhId,
    std::shared_ptr<CapabilityInfo> &capPtr)
{
    if (!IsIdLengthValid(deviceId) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    std::string key = GetCapabilityKey(deviceId, dhId);
    if (globalCapInfoMap_.find(key) == globalCapInfoMap_.end()) {
        DHLOGE("Can not find capability In globalCapInfoMap_: %{public}s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND;
    }
    capPtr = globalCapInfoMap_[key];
    return DH_FWK_SUCCESS;
}

int32_t LocalCapabilityInfoManager::GetDataByKey(const std::string &key, std::shared_ptr<CapabilityInfo> &capInfoPtr)
{
    if (!IsIdLengthValid(key)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGI("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::string data;
    if (dbAdapterPtr_->GetDataByKey(key, data) != DH_FWK_SUCCESS) {
        DHLOGE("Query capability info from db failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return GetCapabilityByValue<CapabilityInfo>(data, capInfoPtr);
}

int32_t LocalCapabilityInfoManager::GetDataByDHType(const DHType dhType, CapabilityInfoMap &capabilityMap)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (const auto &capInfo : globalCapInfoMap_) {
        if (capInfo.second->GetDHType() != dhType) {
            continue;
        }
        capabilityMap[capInfo.first] = capInfo.second;
    }
    return DH_FWK_SUCCESS;
}

int32_t LocalCapabilityInfoManager::GetDataByKeyPrefix(const std::string &keyPrefix, CapabilityInfoMap &capabilityMap)
{
    if (!IsKeySizeValid(keyPrefix)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix(keyPrefix, dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query capability info from db failed, key: %{public}s", GetAnonyString(keyPrefix).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    if (dataVector.empty() || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("On dataVector error, maybe empty or too large!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<CapabilityInfo> capabilityInfo;
        if (GetCapabilityByValue<CapabilityInfo>(data, capabilityInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        if (capabilityInfo->FromJsonString(data) != DH_FWK_SUCCESS) {
            DHLOGE("Wrong data: %{public}s", GetAnonyString(data).c_str());
            continue;
        }
        capabilityMap[capabilityInfo->GetKey()] = capabilityInfo;
    }
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
