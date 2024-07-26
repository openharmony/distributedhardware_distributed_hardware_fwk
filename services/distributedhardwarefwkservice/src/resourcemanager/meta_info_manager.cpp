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

#include "meta_info_manager.h"

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
#include "task_board.h"

namespace OHOS {
namespace DistributedHardware {

#undef DH_LOG_TAG
#define DH_LOG_TAG "MetaInfoManager"

MetaInfoManager::MetaInfoManager() : dbAdapterPtr_(nullptr)
{
    DHLOGI("MetaInfoManager construction!");
}

MetaInfoManager::~MetaInfoManager()
{
    DHLOGI("MetaInfoManager destruction!");
}

std::shared_ptr<MetaInfoManager> MetaInfoManager::GetInstance()
{
    static std::shared_ptr<MetaInfoManager> instance(new(std::nothrow) MetaInfoManager);
    if (instance == nullptr) {
        DHLOGE("instance is nullptr, because applying memory fail!");
        return nullptr;
    }
    return instance;
}

MetaInfoManager::MetaInfoManagerEventHandler::MetaInfoManagerEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, std::shared_ptr<MetaInfoManager> metaInfoMgrPtr)
    : AppExecFwk::EventHandler(runner), metaInfoMgrWPtr_(metaInfoMgrPtr)
{
    DHLOGI("Ctor MetaInfoManagerEventHandler");
}

void MetaInfoManager::MetaInfoManagerEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    auto selfPtr = metaInfoMgrWPtr_.lock();
    if (!selfPtr) {
        DHLOGE("Can not get strong self ptr");
        return;
    }
    switch (eventId) {
        case EVENT_META_INFO_DB_RECOVER:
            selfPtr->SyncRemoteMetaInfos();
            break;
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

std::shared_ptr<MetaInfoManager::MetaInfoManagerEventHandler> MetaInfoManager::GetEventHandler()
{
    return this->eventHandler_;
}

int32_t MetaInfoManager::Init()
{
    DHLOGI("MetaInfoManager instance init!");
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, GLOBAL_META_INFO, shared_from_this());
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    if (dbAdapterPtr_->Init(false, DistributedKv::DataType::TYPE_STATICS) != DH_FWK_SUCCESS) {
        DHLOGE("Init dbAdapterPtr_ failed");
        return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<MetaInfoManager::MetaInfoManagerEventHandler>(runner, shared_from_this());
    DHLOGI("MetaInfoManager instance init success");
    return DH_FWK_SUCCESS;
}

int32_t MetaInfoManager::UnInit()
{
    DHLOGI("MetaInfoManager UnInit");
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED;
    }
    dbAdapterPtr_->UnInit();
    dbAdapterPtr_.reset();
    return DH_FWK_SUCCESS;
}

int32_t MetaInfoManager::AddMetaCapInfos(const std::vector<std::shared_ptr<MetaCapabilityInfo>> &metaCapInfos)
{
    if (metaCapInfos.size() == 0 || metaCapInfos.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("metaCapInfos size is invalid!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> keys;
    std::vector<std::string> values;
    std::string key;
    std::string data;
    for (auto &metaCapInfo : metaCapInfos) {
        if (metaCapInfo == nullptr) {
            continue;
        }
        key = metaCapInfo->GetKey();
        globalMetaInfoMap_[key] = metaCapInfo;
        if (dbAdapterPtr_->GetDataByKey(key, data) == DH_FWK_SUCCESS && data == metaCapInfo->ToJsonString()) {
            DHLOGI("this record is exist, Key: %{public}s", metaCapInfo->GetAnonymousKey().c_str());
            continue;
        }
        DHLOGI("AddMetaCapability, Key: %{public}s", metaCapInfo->GetAnonymousKey().c_str());
        keys.push_back(key);
        values.push_back(metaCapInfo->ToJsonString());
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

int32_t MetaInfoManager::SyncMetaInfoFromDB(const std::string &udidHash)
{
    DHLOGI("Sync MetaInfo from DB, udidHash: %{public}s", GetAnonyString(udidHash).c_str());
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix(udidHash, dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query Metadata from DB by udidHash failed, udidHash: %{public}s", GetAnonyString(udidHash).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    if (dataVector.size() == 0 || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("DataVector size is invalid!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<MetaCapabilityInfo> metaCapInfo;
        if (GetMetaCapByValue(data, metaCapInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get capability ptr by value failed");
            continue;
        }
        globalMetaInfoMap_[metaCapInfo->GetKey()] = metaCapInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t MetaInfoManager::SyncRemoteMetaInfos()
{
    DHLOGI("Sync full remote device Metainfo from DB");
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix("", dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query all Metadata from DB failed");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    if (dataVector.size() == 0 || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("DataVector size is invalid!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<MetaCapabilityInfo> metaCapInfo;
        if (GetMetaCapByValue(data, metaCapInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get Metainfo ptr by value failed");
            continue;
        }
        const std::string &udidHash = metaCapInfo->GetUdidHash();
        const std::string &localUdidHash = DHContext::GetInstance().GetDeviceInfo().udidHash;
        if (udidHash.compare(localUdidHash) == 0) {
            DHLOGE("device MetaInfo not need sync from db");
            continue;
        }
        if (!DHContext::GetInstance().IsDeviceOnline(
            DHContext::GetInstance().GetUUIDByDeviceId(metaCapInfo->GetDeviceId()))) {
            DHLOGE("offline device, no need sync to memory, udidHash : %{public}s",
                GetAnonyString(metaCapInfo->GetUdidHash()).c_str());
            continue;
        }
        globalMetaInfoMap_[metaCapInfo->GetKey()] = metaCapInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t MetaInfoManager::GetDataByKeyPrefix(const std::string &keyPrefix, MetaCapInfoMap &metaCapMap)
{
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> dataVector;
    if (dbAdapterPtr_->GetDataByKeyPrefix(keyPrefix, dataVector) != DH_FWK_SUCCESS) {
        DHLOGE("Query metaInfo from db failed, keyPrefix: %{public}s", GetAnonyString(keyPrefix).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    if (dataVector.size() == 0 || dataVector.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("DataVector size is invalid!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto &data : dataVector) {
        std::shared_ptr<MetaCapabilityInfo> metaCapInfo;
        if (GetMetaCapByValue(data, metaCapInfo) != DH_FWK_SUCCESS) {
            DHLOGE("Get Metainfo ptr by value failed");
            continue;
        }
        metaCapMap[metaCapInfo->GetKey()] = metaCapInfo;
    }
    return DH_FWK_SUCCESS;
}

int32_t MetaInfoManager::RemoveMetaInfoByKey(const std::string &key)
{
    DHLOGI("Remove device metaInfo, key: %{public}s", GetAnonyString(key).c_str());
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }

    globalMetaInfoMap_.erase(key);
    if (dbAdapterPtr_->RemoveDataByKey(key) != DH_FWK_SUCCESS) {
        DHLOGE("Remove device metaData failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t MetaInfoManager::GetMetaCapInfo(const std::string &udidHash,
    const std::string &dhId, std::shared_ptr<MetaCapabilityInfo> &metaCapPtr)
{
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    std::string key = GetCapabilityKey(udidHash, dhId);
    if (globalMetaInfoMap_.find(key) == globalMetaInfoMap_.end()) {
        DHLOGE("Can not find capability In globalMetaInfoMap_: %{public}s", GetAnonyString(udidHash).c_str());
        return ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND;
    }
    metaCapPtr = globalMetaInfoMap_[key];
    return DH_FWK_SUCCESS;
}

void MetaInfoManager::GetMetaCapInfosByUdidHash(const std::string &udidHash,
    std::vector<std::shared_ptr<MetaCapabilityInfo>> &metaCapInfos)
{
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    for (auto &metaCapInfo : globalMetaInfoMap_) {
        if (IsCapKeyMatchDeviceId(metaCapInfo.first, udidHash)) {
            metaCapInfos.emplace_back(metaCapInfo.second);
        }
    }
}

int32_t MetaInfoManager::GetMetaCapByValue(const std::string &value, std::shared_ptr<MetaCapabilityInfo> &metaCapPtr)
{
    if (metaCapPtr == nullptr) {
        metaCapPtr = std::make_shared<MetaCapabilityInfo>();
    }
    return metaCapPtr->FromJsonString(value);
}

void MetaInfoManager::OnChange(const DistributedKv::ChangeNotification &changeNotification)
{
    DHLOGI("MetaInfoManager: DB data OnChange");
    if (!changeNotification.GetInsertEntries().empty() &&
        changeNotification.GetInsertEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("MetaInfoManager Handle capability data add change");
        HandleMetaCapabilityAddChange(changeNotification.GetInsertEntries());
    }
    if (!changeNotification.GetUpdateEntries().empty() &&
        changeNotification.GetUpdateEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("MetaInfoManager Handle capability data update change");
        HandleMetaCapabilityUpdateChange(changeNotification.GetUpdateEntries());
    }
    if (!changeNotification.GetDeleteEntries().empty() &&
        changeNotification.GetDeleteEntries().size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("MetaInfoManager Handle capability data delete change");
        HandleMetaCapabilityDeleteChange(changeNotification.GetDeleteEntries());
    }
}

void MetaInfoManager::OnChange(const DistributedKv::DataOrigin &origin, Keys &&keys)
{
    DHLOGI("MetaInfoManager: Cloud data OnChange.");
    std::vector<DistributedKv::Entry> insertRecords = GetEntriesByKeys(keys[ChangeOp::OP_INSERT]);
    if (!insertRecords.empty() && insertRecords.size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("MetaInfoManager Handle capability data add change");
        HandleMetaCapabilityAddChange(insertRecords);
    }
    std::vector<DistributedKv::Entry> updateRecords = GetEntriesByKeys(keys[ChangeOp::OP_UPDATE]);
    if (!updateRecords.empty() && updateRecords.size() <= MAX_DB_RECORD_SIZE) {
        DHLOGI("MetaInfoManager Handle capability data update change");
        HandleMetaCapabilityUpdateChange(updateRecords);
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
        DHLOGI("MetaInfoManager Handle capability data delete change");
        HandleMetaCapabilityDeleteChange(deleteRecords);
    }
}

void MetaInfoManager::HandleMetaCapabilityAddChange(const std::vector<DistributedKv::Entry> &insertRecords)
{
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    for (const auto &item : insertRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<MetaCapabilityInfo> capPtr;
        if (GetCapabilityByValue<MetaCapabilityInfo>(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get Meta capability by value failed");
            continue;
        }
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(capPtr->GetDeviceId());
        if (uuid.empty()) {
            DHLOGE("Find uuid failed and never enable, deviceId: %{public}s",
                GetAnonyString(capPtr->GetDeviceId()).c_str());
            continue;
        }
        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGE("Find network failed and never enable, uuid: %{public}s", GetAnonyString(uuid).c_str());
            continue;
        }

        const auto keyString = capPtr->GetKey();
        DHLOGI("Add MetaCapability key: %{public}s", capPtr->GetAnonymousKey().c_str());
        globalMetaInfoMap_[keyString] = capPtr;
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

void MetaInfoManager::HandleMetaCapabilityUpdateChange(const std::vector<DistributedKv::Entry> &updateRecords)
{
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    for (const auto &item : updateRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<MetaCapabilityInfo> capPtr;
        if (GetCapabilityByValue<MetaCapabilityInfo>(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get Meta capability by value failed");
            continue;
        }
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(capPtr->GetDeviceId());
        if (uuid.empty()) {
            DHLOGE("Find uuid failed and never enable, deviceId: %{public}s",
                GetAnonyString(capPtr->GetDeviceId()).c_str());
            continue;
        }
        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGE("Find network failed and never enable, uuid: %{public}s", GetAnonyString(uuid).c_str());
            continue;
        }
        std::string enabledDeviceKey = GetCapabilityKey(capPtr->GetDeviceId(), capPtr->GetDHId());
        if (TaskBoard::GetInstance().IsEnabledDevice(enabledDeviceKey)) {
            DHLOGI("The deviceKey: %{public}s is enabled.", GetAnonyString(enabledDeviceKey).c_str());
            continue;
        }
        const auto keyString = capPtr->GetKey();
        DHLOGI("Update MetaCapability key: %{public}s", capPtr->GetAnonymousKey().c_str());
        globalMetaInfoMap_[keyString] = capPtr;
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

void MetaInfoManager::HandleMetaCapabilityDeleteChange(const std::vector<DistributedKv::Entry> &deleteRecords)
{
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    for (const auto &item : deleteRecords) {
        const std::string value = item.value.ToString();
        std::shared_ptr<MetaCapabilityInfo> capPtr;
        if (GetCapabilityByValue<MetaCapabilityInfo>(value, capPtr) != DH_FWK_SUCCESS) {
            DHLOGE("Get Meta capability by value failed");
            continue;
        }
        const auto keyString = capPtr->GetKey();
        DHLOGI("Delete MetaCapability key: %{public}s", capPtr->GetAnonymousKey().c_str());
        globalMetaInfoMap_.erase(keyString);
    }
}

std::vector<DistributedKv::Entry> MetaInfoManager::GetEntriesByKeys(const std::vector<std::string> &keys)
{
    DHLOGI("call");
    if (keys.empty()) {
        DHLOGE("keys empty.");
        return {};
    }
    std::lock_guard<std::mutex> lock(metaInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return {};
    }
    return dbAdapterPtr_->GetEntriesByKeys(keys);
}
}
}