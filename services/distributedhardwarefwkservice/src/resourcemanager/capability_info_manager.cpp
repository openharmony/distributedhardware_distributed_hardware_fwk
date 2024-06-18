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

#include "capability_info_manager.h"

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
#define DH_LOG_TAG "CapabilityInfoManager"

CapabilityInfoManager::CapabilityInfoManager() : dbAdapterPtr_(nullptr)
{
    DHLOGI("CapabilityInfoManager construction!");
}

CapabilityInfoManager::~CapabilityInfoManager()
{
    DHLOGI("CapabilityInfoManager destruction!");
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

CapabilityInfoManager::CapabilityInfoManagerEventHandler::CapabilityInfoManagerEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner,
    std::shared_ptr<CapabilityInfoManager> capabilityInfoMgrPtr)
    : AppExecFwk::EventHandler(runner)
{
    DHLOGI("Ctor CapabilityInfoManagerEventHandler");
    capabilityInfoMgrWPtr_ = capabilityInfoMgrPtr;
}

void CapabilityInfoManager::CapabilityInfoManagerEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    auto selfPtr = capabilityInfoMgrWPtr_.lock();
    if (!selfPtr) {
        DHLOGE("Can not get strong self ptr");
        return;
    }
    switch (eventId) {
        case EVENT_CAPABILITY_INFO_DB_RECOVER:
            selfPtr->SyncRemoteCapabilityInfos();
            break;
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

std::shared_ptr<CapabilityInfoManager::CapabilityInfoManagerEventHandler> CapabilityInfoManager::GetEventHandler()
{
    return this->eventHandler_;
}

int32_t CapabilityInfoManager::Init()
{
    DHLOGI("CapabilityInfoManager instance init!");
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, GLOBAL_CAPABILITY_ID, shared_from_this());
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    if (dbAdapterPtr_->Init(false, DistributedKv::DataType::TYPE_DYNAMICAL) != DH_FWK_SUCCESS) {
        DHLOGE("Init dbAdapterPtr_ failed");
        return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<CapabilityInfoManager::CapabilityInfoManagerEventHandler>(
        runner, shared_from_this());
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
    if (dataVector.size() == 0 || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("DataVector size is invalid!");
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
    if (dataVector.size() == 0 || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("DataVector size is invalid!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<CapabilityInfo> capabilityInfo;
        if (GetCapabilityByValue<CapabilityInfo>(data, capabilityInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        const std::string &deviceId = capabilityInfo->GetDeviceId();
        const std::string &localDeviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
        if (deviceId.compare(localDeviceId) == 0) {
            DHLOGE("local device info not need sync from db");
            continue;
        }
        if (!DHContext::GetInstance().IsDeviceOnline(DHContext::GetInstance().GetUUIDByDeviceId(deviceId))) {
            DHLOGE("offline device, no need sync to memory, deviceId : %{public}s ", GetAnonyString(deviceId).c_str());
            continue;
        }
        globalCapInfoMap_[capabilityInfo->GetKey()] = capabilityInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::AddCapability(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    if (resInfos.size() == 0 || resInfos.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("ResInfos size is invalid!");
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
    std::string data;
    for (auto &resInfo : resInfos) {
        if (resInfo == nullptr) {
            continue;
        }
        key = resInfo->GetKey();
        globalCapInfoMap_[key] = resInfo;
        if (dbAdapterPtr_->GetDataByKey(key, data) == DH_FWK_SUCCESS &&
            IsCapInfoJsonEqual<CapabilityInfo>(data, resInfo->ToJsonString())) {
            DHLOGD("this record is exist, Key: %{public}s", resInfo->GetAnonymousKey().c_str());
            continue;
        }
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

int32_t CapabilityInfoManager::AddCapabilityInMem(const std::vector<std::shared_ptr<CapabilityInfo>> &resInfos)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (auto &resInfo : resInfos) {
        if (resInfo == nullptr) {
            continue;
        }
        const std::string key = resInfo->GetKey();
        DHLOGI("AddCapabilityInMem, Key: %{public}s", resInfo->GetAnonymousKey().c_str());
        globalCapInfoMap_[key] = resInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::RemoveCapabilityInfoInDB(const std::string &deviceId)
{
    if (deviceId.size() == 0 || deviceId.size() > MAX_ID_LEN) {
        DHLOGE("DeviceId is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Remove capability device info, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    // 1. Clear the cache in the memory.
    for (auto iter = globalCapInfoMap_.begin(); iter != globalCapInfoMap_.end();) {
        if (!IsCapKeyMatchDeviceId(iter->first, deviceId)) {
            iter++;
            continue;
        }
        DHLOGI("Clear globalCapInfoMap_ iter: %{public}s", GetAnonyString(iter->first).c_str());
        globalCapInfoMap_.erase(iter++);
    }
    // 2. Delete the corresponding record from the database(use UUID).
    if (dbAdapterPtr_->RemoveDeviceData(deviceId) != DH_FWK_SUCCESS) {
        DHLOGE("Remove capability Device Data failed, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t CapabilityInfoManager::RemoveCapabilityInfoByKey(const std::string &key)
{
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

int32_t CapabilityInfoManager::RemoveCapabilityInfoInMem(const std::string &deviceId)
{
    DHLOGI("remove capability device info in memory, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (auto iter = globalCapInfoMap_.begin(); iter != globalCapInfoMap_.end();) {
        if (!IsCapKeyMatchDeviceId(iter->first, deviceId)) {
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
    for (auto &info : globalCapInfoMap_) {
        bool isMatch = true;
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

void CapabilityInfoManager::OnChange(const DistributedKv::ChangeNotification &changeNotification)
{
    DHLOGI("CapabilityInfoManager: DB data OnChange");
    if (!changeNotification.GetInsertEntries().empty() &&
        changeNotification.GetInsertEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle capability data add change");
        HandleCapabilityAddChange(changeNotification.GetInsertEntries());
    }
    if (!changeNotification.GetUpdateEntries().empty() &&
        changeNotification.GetUpdateEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle capability data update change");
        HandleCapabilityUpdateChange(changeNotification.GetUpdateEntries());
    }
    if (!changeNotification.GetDeleteEntries().empty() &&
        changeNotification.GetDeleteEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle capability data delete change");
        HandleCapabilityDeleteChange(changeNotification.GetDeleteEntries());
    }
}

void CapabilityInfoManager::OnChange(const DistributedKv::DataOrigin &origin, Keys &&keys)
{
    DHLOGI("CapabilityInfoManager: Cloud data OnChange.");
    std::vector<DistributedKv::Entry> insertRecords = GetEntriesByKeys(keys[ChangeOp::OP_INSERT]);
    if (!insertRecords.empty() && insertRecords.size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle capability data add change");
        HandleCapabilityAddChange(insertRecords);
    }
    std::vector<DistributedKv::Entry> updateRecords = GetEntriesByKeys(keys[ChangeOp::OP_UPDATE]);
    if (!updateRecords.empty() && updateRecords.size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("Handle capability data update change");
        HandleCapabilityUpdateChange(updateRecords);
    }
    std::vector<std::string> delKeys = keys[ChangeOp::OP_DELETE];
    if (!delKeys.empty() && delKeys.size() <= MAX_DB_RECORD_SIZE) {
        std::vector<DistributedKv::Entry> deleteRecords;
        for (const auto &key : delKeys) {
            DistributedKv::Entry entry;
            DistributedKv::Key kvKey(key);
            entry.key = kvKey;
            deleteRecords.emplace_back(entry);
        }
        DHLOGI("Handle capability data delete change");
        HandleCapabilityDeleteChange(deleteRecords);
    }
}

void CapabilityInfoManager::HandleCapabilityAddChange(const std::vector<DistributedKv::Entry> &insertRecords)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (const auto &item : insertRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<CapabilityInfo> capPtr;
        if (GetCapabilityByValue<CapabilityInfo>(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability by value failed");
            continue;
        }
        const auto keyString = capPtr->GetKey();
        DHLOGI("Add capability key: %{public}s", capPtr->GetAnonymousKey().c_str());
        globalCapInfoMap_[keyString] = capPtr;
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(capPtr->GetDeviceId());
        if (uuid.empty()) {
            DHLOGI("Find uuid failed and never enable");
            continue;
        }
        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGI("Find network failed and never enable, uuid: %{public}s", GetAnonyString(uuid).c_str());
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
        if (GetCapabilityByValue<CapabilityInfo>(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability by value failed");
            continue;
        }
        const auto keyString = capPtr->GetKey();
        DHLOGI("Update capability key: %{public}s", capPtr->GetAnonymousKey().c_str());
        globalCapInfoMap_[keyString] = capPtr;
    }
}

void CapabilityInfoManager::HandleCapabilityDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    for (const auto &item : deleteRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<CapabilityInfo> capPtr;
        if (GetCapabilityByValue<CapabilityInfo>(value, capPtr) != DH_FWK_SUCCESS) {
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
            DHLOGI("Find network failed and never disable, uuid: %{public}s", GetAnonyString(uuid).c_str());
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
        DHLOGI("Delete capability key: %{public}s", capPtr->GetAnonymousKey().c_str());
        globalCapInfoMap_.erase(keyString);
    }
}

bool CapabilityInfoManager::IsCapabilityMatchFilter(const std::shared_ptr<CapabilityInfo> &cap,
    const CapabilityInfoFilter &filter, const std::string &value)
{
    if (cap == nullptr) {
        DHLOGE("cap is null");
        return false;
    }

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
        if (IsCapKeyMatchDeviceId(capabilityInfo.first, deviceId)) {
            resInfos.emplace_back(capabilityInfo.second);
        }
    }
}

bool CapabilityInfoManager::HasCapability(const std::string &deviceId, const std::string &dhId)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    std::string kvKey = GetCapabilityKey(deviceId, dhId);
    if (globalCapInfoMap_.find(kvKey) == globalCapInfoMap_.end()) {
        return false;
    }
    return true;
}

int32_t CapabilityInfoManager::GetCapability(const std::string &deviceId, const std::string &dhId,
    std::shared_ptr<CapabilityInfo> &capPtr)
{
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    std::string key = GetCapabilityKey(deviceId, dhId);
    if (globalCapInfoMap_.find(key) == globalCapInfoMap_.end()) {
        DHLOGE("Can not find capability In globalCapInfoMap_: %{public}s", GetAnonyString(deviceId).c_str());
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
        DHLOGE("Query capability info from db failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return GetCapabilityByValue<CapabilityInfo>(data, capInfoPtr);
}

int32_t CapabilityInfoManager::GetDataByDHType(const DHType dhType, CapabilityInfoMap &capabilityMap)
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

int32_t CapabilityInfoManager::GetDataByKeyPrefix(const std::string &keyPrefix, CapabilityInfoMap &capabilityMap)
{
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
    if (dataVector.size() == 0 || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("DataVector size is invalid!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<CapabilityInfo> capabilityInfo;
        if (GetCapabilityByValue<CapabilityInfo>(data, capabilityInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        capabilityMap[capabilityInfo->GetKey()] = capabilityInfo;
    }
    return DH_FWK_SUCCESS;
}

void CapabilityInfoManager::DumpCapabilityInfos(std::vector<CapabilityInfo> &capInfos)
{
    for (auto info : globalCapInfoMap_) {
        CapabilityInfo capInfo = *(info.second);
        capInfos.emplace_back(capInfo);
    }
}

std::vector<DistributedKv::Entry> CapabilityInfoManager::GetEntriesByKeys(const std::vector<std::string> &keys)
{
    DHLOGI("call");
    if (keys.empty()) {
        DHLOGE("keys empty.");
        return {};
    }
    std::lock_guard<std::mutex> lock(capInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return {};
    }
    return dbAdapterPtr_->GetEntriesByKeys(keys);
}
} // namespace DistributedHardware
} // namespace OHOS
