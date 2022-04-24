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

#include "capability_info_manager.h"

#include "anonymous_string.h"
#include "capability_info_event.h"
#include "capability_utils.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager.h"
#include "event_bus.h"
#include "task_executor.h"
#include "task_factory.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "CapabilityInfoManager"

CapabilityInfoManager::CapabilityInfoManager() : dbAdapterPtr_(nullptr)
{}

CapabilityInfoManager::~CapabilityInfoManager()
{
    DHLOGI("CapabilityInfoManager Destruction!");
}

std::shared_ptr<CapabilityInfoManager> CapabilityInfoManager::GetInstance()
{
    static std::shared_ptr<CapabilityInfoManager> instance(new(std::nothrow) CapabilityInfoManager);
    if (instance == nullptr) {
        DHLOGE("instance is nullptr, because applying memory fail!");
        return nullptr;
    }
    return instance;
}

int32_t CapabilityInfoManager::Init()
{
    DHLOGI("CapabilityInfoManager instance init!");
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, GLOBAL_CAPABILITY_ID, shared_from_this());
    if (dbAdapterPtr_->Init() != DH_FWK_SUCCESS) {
        DHLOGE("Init dbAdapterPtr_ failed");
        return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
    }
    CapabilityInfoEvent capabilityInfoEvent(*this);
    DHContext::GetInstance().GetEventBus()->AddHandler<CapabilityInfoEvent>(capabilityInfoEvent.GetType(), *this);
    DHLOGI("CapabilityInfoManager instance init success");
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::UnInit()
{
    DHLOGI("CapabilityInfoManager UnInit");
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED;
    }
    dbAdapterPtr_->UnInit();
    dbAdapterPtr_.reset();
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::SyncDeviceInfoFromDB(const std::string &deviceId)
{
    DHLOGI("Sync DeviceInfo from DB, deviceId: %s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix(deviceId, dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query data from DB by deviceId failed, id: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<CapabilityInfo> capabilityInfo;
        if (CapabilityUtils::GetCapabilityByValue(data, capabilityInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        globalCapInfoMap_[capabilityInfo->GetKey()] = capabilityInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::SyncRemoteCapabilityInfos()
{
    DHLOGI("Sync full remote device info from DB");
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix("", dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query all data from DB failed");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<CapabilityInfo> capabilityInfo;
        if (CapabilityUtils::GetCapabilityByValue(data, capabilityInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        const std::string &deviceId = capabilityInfo->GetDeviceId();
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
        globalCapInfoMap_[capabilityInfo->GetKey()] = capabilityInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::AddCapability(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> keys;
    std::vector<std::string> values;
    for (auto &resInfo : resInfos) {
        if (!resInfo) {
            continue;
        }
        const std::string key = resInfo->GetKey();
        DHLOGI("AddCapability, Key: %s", resInfo->GetAnonymousKey().c_str());
        keys.push_back(key);
        values.push_back(resInfo->ToJsonString());
        globalCapInfoMap_[key] = resInfo;
    }
    if (dbAdapterPtr_->PutDataBatch(keys, values) != DH_FWK_SUCCESS) {
        DHLOGE("Fail to storage batch to kv");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::AddCapabilityInMem(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (auto &resInfo : resInfos) {
        if (!resInfo) {
            continue;
        }
        const std::string key = resInfo->GetKey();
        DHLOGI("AddCapabilityInMem, Key: %s", resInfo->GetAnonymousKey().c_str());
        globalCapInfoMap_[key] = resInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::RemoveCapabilityInfoInDB(const std::string &deviceId)
{
    DHLOGI("Remove capability device info, deviceId: %s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    if (deviceId.empty()) {
        DHLOGE("RemoveCapabilityInfoInDB failed, deviceId is empty");
        return ERR_DH_FWK_PARA_INVALID;
    }
    // 1. Clear the cache in the memory.
    for (auto iter = globalCapInfoMap_.begin(); iter != globalCapInfoMap_.end();) {
        if (!CapabilityUtils::IsCapKeyMatchDeviceId(iter->first, deviceId)) {
            iter++;
            continue;
        }
        DHLOGI("Clear globalCapInfoMap_ iter: %s", GetAnonyString(iter->first).c_str());
        globalCapInfoMap_.erase(iter++);
    }
    // 2. Delete the corresponding record from the database(use UUID).
    if (dbAdapterPtr_->RemoveDeviceData(deviceId) != DH_FWK_SUCCESS) {
        DHLOGE("Remove capability Device Data failed, deviceId: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::RemoveCapabilityInfoByKey(const std::string &key)
{
    DHLOGI("Remove capability device info, key: %s", GetAnonyString(key).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    if (key.empty()) {
        DHLOGE("key is empty");
        return ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY;
    }
    // 1. Clear the cache in the memory.
    globalCapInfoMap_.erase(key);

    // 2. Delete the corresponding record from the database.(use key)
    if (dbAdapterPtr_->RemoveDataByKey(key) != DH_FWK_SUCCESS) {
        DHLOGE("Remove capability Device Data failed, key: %s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::RemoveCapabilityInfoInMem(const std::string &deviceId)
{
    DHLOGI("remove capability device info in memory, deviceId: %s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (deviceId.empty()) {
        DHLOGE("RemoveCapabilityInfoInMem failed, deviceId is empty");
        return ERR_DH_FWK_PARA_INVALID;
    }
    for (auto iter = globalCapInfoMap_.begin(); iter != globalCapInfoMap_.end();) {
        if (!CapabilityUtils::IsCapKeyMatchDeviceId(iter->first, deviceId)) {
            iter++;
            continue;
        }
        globalCapInfoMap_.erase(iter++);
    }
    return DH_FWK_SUCCESS;
}

std::map<std::string, std::shared_ptr<CapabilityInfo>> CapabilityInfoManager::QueryCapabilityByFilters(
    const std::map<CapabilityInfoFilter, std::string> &filters)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    std::map<std::string, std::shared_ptr<CapabilityInfo>> capMap;
    bool isMatch = true;
    for (auto &info : globalCapInfoMap_) {
        isMatch = true;
        for (auto &filter : filters) {
            if (!IsCapabilityMatchFilter(info.second, filter.first, filter.second)) {
                isMatch = false;
                break;
            }
        }
        if (isMatch) {
            capMap.emplace(info.first, info.second);
        }
    }
    return capMap;
}

void CapabilityInfoManager::CreateManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->CreateManualSyncCount(deviceId);
}

void CapabilityInfoManager::RemoveManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return;
    }
    dbAdapterPtr_->RemoveManualSyncCount(deviceId);
}

int32_t CapabilityInfoManager::ManualSync(const std::string &networkId)
{
    DHLOGI("ManualSync start, networkId: %s", GetAnonyString(networkId).c_str());
    std::unique_lock<std::mutex> lock(capInfoMgrMutex_);
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

void CapabilityInfoManager::OnChange(const DistributedKv::ChangeNotification &changeNotification)
{
    DHLOGI("CapabilityInfoManager: DB data OnChange");
    if (!changeNotification.GetInsertEntries().empty()) {
        DHLOGI("Handle capability data add change");
        HandleCapabilityAddChange(changeNotification.GetInsertEntries());
    }
    if (!changeNotification.GetUpdateEntries().empty()) {
        DHLOGI("Handle capability data update change");
        HandleCapabilityUpdateChange(changeNotification.GetUpdateEntries());
    }
    if (!changeNotification.GetDeleteEntries().empty()) {
        DHLOGI("Handle capability data delete change");
        HandleCapabilityDeleteChange(changeNotification.GetDeleteEntries());
    }
}

void CapabilityInfoManager::OnChange(const DistributedKv::ChangeNotification &changeNotification,
    std::shared_ptr<DistributedKv::KvStoreSnapshot> snapshot)
{}

void CapabilityInfoManager::OnEvent(CapabilityInfoEvent &ev)
{
    switch (ev.GetAction()) {
        case CapabilityInfoEvent::EventType::RECOVER:
            SyncRemoteCapabilityInfos();
            break;
        default:
            DHLOGE("Event is undefined, type is %d", ev.GetAction());
            break;
    }
}

void CapabilityInfoManager::HandleCapabilityAddChange(const std::vector<DistributedKv::Entry> &insertRecords)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (const auto &item : insertRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<CapabilityInfo> capPtr;
        if (CapabilityUtils::GetCapabilityByValue(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability by value failed");
            continue;
        }
        const auto keyString = capPtr->GetKey();
        DHLOGI("Add capability key: %s", capPtr->GetAnonymousKey().c_str());
        globalCapInfoMap_[keyString] = capPtr;
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(capPtr->GetDeviceId());
        if (uuid.empty()) {
            DHLOGI("Find uuid failed and never enable");
            continue;
        }
        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGI("Find network failed and never enable, uuid: %s", GetAnonyString(uuid).c_str());
            continue;
        }
        TaskParam taskParam = {
            .networkId = networkId,
            .uuid = uuid,
            .dhId = capPtr->GetDHId(),
            .dhType = capPtr->GetDHType()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
}

void CapabilityInfoManager::HandleCapabilityUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (const auto &item : updateRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<CapabilityInfo> capPtr;
        if (CapabilityUtils::GetCapabilityByValue(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability by value failed");
            continue;
        }
        const auto keyString = capPtr->GetKey();
        DHLOGI("Update capability key: %s", capPtr->GetAnonymousKey().c_str());
        globalCapInfoMap_[keyString] = capPtr;
    }
}

void CapabilityInfoManager::HandleCapabilityDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (const auto &item : deleteRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<CapabilityInfo> capPtr;
        if (CapabilityUtils::GetCapabilityByValue(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability by value failed");
            continue;
        }
        const auto keyString = capPtr->GetKey();
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(capPtr->GetDeviceId());
        if (uuid.empty()) {
            DHLOGI("Find uuid failed and never disable");
            continue;
        }
        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGI("Find network failed and never disable, uuid: %s", GetAnonyString(uuid).c_str());
            continue;
        }
        TaskParam taskParam = {
            .networkId = networkId,
            .uuid = uuid,
            .dhId = capPtr->GetDHId(),
            .dhType = capPtr->GetDHType()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::DISABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
        DHLOGI("Delete capability key: %s", capPtr->GetAnonymousKey().c_str());
        globalCapInfoMap_.erase(keyString);
    }
}

bool CapabilityInfoManager::IsCapabilityMatchFilter(const std::shared_ptr<CapabilityInfo> &cap,
    const CapabilityInfoFilter &filter, const std::string &value)
{
    bool isMatch = false;
    switch (filter) {
        case CapabilityInfoFilter::FILTER_DH_ID: {
            isMatch = cap->GetDHId().compare(value) == 0;
            break;
        }
        case CapabilityInfoFilter::FILTER_DEVICE_ID: {
            isMatch = cap->GetDeviceId().compare(value) == 0;
            break;
        }
        case CapabilityInfoFilter::FILTER_DEVICE_NAME: {
            isMatch = cap->GetDeviceName().compare(value) == 0;
            break;
        }
        case CapabilityInfoFilter::FILTER_DEVICE_TYPE: {
            auto devType = static_cast<uint16_t>(std::stoi(value));
            isMatch = cap->GetDeviceType() == devType;
            break;
        }
        case CapabilityInfoFilter::FILTER_DH_TYPE: {
            DHType dhType = (DHType)std::stoi(value);
            isMatch = cap->GetDHType() == dhType;
            break;
        }
        case CapabilityInfoFilter::FILTER_DH_ATTRS: {
            isMatch = cap->GetDHAttrs().compare(value) == 0;
            break;
        }
        default: {
            isMatch = false;
            break;
        }
    }
    return isMatch;
}

void CapabilityInfoManager::GetCapabilitiesByDeviceId(const std::string &deviceId,
    std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (auto &capabilityInfo : globalCapInfoMap_) {
        if (CapabilityUtils::IsCapKeyMatchDeviceId(capabilityInfo.first, deviceId)) {
            resInfos.emplace_back(capabilityInfo.second);
        }
    }
}

bool CapabilityInfoManager::HasCapability(const std::string &deviceId, const std::string &dhId)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    std::string kvKey = CapabilityUtils::GetCapabilityKey(deviceId, dhId);
    if (globalCapInfoMap_.find(kvKey) == globalCapInfoMap_.end()) {
        return false;
    }
    return true;
}

int32_t CapabilityInfoManager::GetCapability(const std::string &deviceId, const std::string &dhId,
    std::shared_ptr<CapabilityInfo> &capPtr)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    std::string key = CapabilityUtils::GetCapabilityKey(deviceId, dhId);
    if (globalCapInfoMap_.find(key) == globalCapInfoMap_.end()) {
        DHLOGE("Can not find capability In globalCapInfoMap_: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND;
    }
    capPtr = globalCapInfoMap_[key];
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::GetDataByKey(const std::string &key, std::shared_ptr<CapabilityInfo> &capInfoPtr)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGI("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::string data;
    if (dbAdapterPtr_->GetDataByKey(key, data) != DH_FWK_SUCCESS) {
        DHLOGE("Query capability info from db failed, key: %s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return CapabilityUtils::GetCapabilityByValue(data, capInfoPtr);
}

int32_t CapabilityInfoManager::GetDataByKeyPrefix(const std::string &keyPrefix, CapabilityInfoMap &capabilityMap)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGI("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix(keyPrefix, dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query capability info from db failed, key: %s", GetAnonyString(keyPrefix).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<CapabilityInfo> capabilityInfo;
        if (CapabilityUtils::GetCapabilityByValue(data, capabilityInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        if (capabilityInfo->FromJsonString(data) != DH_FWK_SUCCESS) {
            DHLOGE("Wrong data: %s", GetAnonyString(data).c_str());
            continue;
        }
        capabilityMap[capabilityInfo->GetKey()] = capabilityInfo;
    }
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
