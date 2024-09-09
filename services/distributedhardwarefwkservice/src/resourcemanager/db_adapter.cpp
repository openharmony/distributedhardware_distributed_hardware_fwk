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

#include "db_adapter.h"

#include <vector>

#include "anonymous_string.h"
#include "capability_info.h"
#include "capability_info_manager.h"
#include "capability_utils.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "event_handler.h"
#include "meta_info_manager.h"
#include "version_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DBAdapter"

namespace {
    constexpr int32_t MAX_INIT_RETRY_TIMES = 20;
    constexpr int32_t INIT_RETRY_SLEEP_INTERVAL = 200 * 1000; // 200ms
    constexpr int32_t DIED_CHECK_MAX_TIMES = 300;
    constexpr int32_t DIED_CHECK_INTERVAL = 100 * 1000; // 100ms
    const std::string DATABASE_DIR = "/data/service/el1/public/database/";
}

DBAdapter::DBAdapter(const std::string &appId, const std::string &storeId,
                     const std::shared_ptr<DistributedKv::KvStoreObserver> changeListener)
{
    this->appId_.appId = appId;
    this->storeId_.storeId = storeId;
    this->dataChangeListener_ = changeListener;
    DHLOGI("DBAdapter Constructor Success, appId: %{public}s, storeId: %{public}s", appId.c_str(), storeId.c_str());
}

DBAdapter::~DBAdapter()
{
    DHLOGI("DBAdapter Destruction");
}

DistributedKv::Status DBAdapter::GetKvStorePtr(bool isAutoSync, DistributedKv::DataType dataType)
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = isAutoSync,
        .isPublic = true,
        .securityLevel = DistributedKv::SecurityLevel::S1,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = DATABASE_DIR + appId_.appId,
        .dataType = dataType,
        .cloudConfig = {
            .enableCloud = true,
            .autoSync  = true,
        }
    };
    if (dataType == DistributedKv::DataType::TYPE_DYNAMICAL) {
        DHLOGI("Dynamic not go to cloud.");
        options.cloudConfig.enableCloud = false;
        options.cloudConfig.autoSync = false;
    }
    return kvDataMgr_.GetSingleKvStore(options, appId_, storeId_, kvStoragePtr_);
}

DistributedKv::Status DBAdapter::GetLocalKvStorePtr()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .securityLevel = DistributedKv::SecurityLevel::S1,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = DATABASE_DIR + appId_.appId
    };
    return kvDataMgr_.GetSingleKvStore(options, appId_, storeId_, kvStoragePtr_);
}

int32_t DBAdapter::Init(bool isAutoSync, DistributedKv::DataType dataType)
{
    this->isAutoSync_ = isAutoSync;
    this->dataType_ = dataType;
    DHLOGI("Init DB, storeId: %{public}s, dataType: %{public}d",
        storeId_.storeId.c_str(), static_cast<int32_t>(dataType));
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    int32_t tryTimes = MAX_INIT_RETRY_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStorePtr(isAutoSync, dataType);
        if (status == DistributedKv::Status::SUCCESS && kvStoragePtr_) {
            DHLOGI("Init KvStorePtr Success");
            RegisterChangeListener();
            RegisterKvStoreDeathListener();
            return DH_FWK_SUCCESS;
        }

        if (status == DistributedKv::Status::STORE_META_CHANGED) {
            DHLOGW("This db meta changed, remove and rebuild it");
            kvDataMgr_.DeleteKvStore(appId_, storeId_, DATABASE_DIR + appId_.appId);
        }

        DHLOGD("CheckKvStore, left times: %{public}d", tryTimes);
        usleep(INIT_RETRY_SLEEP_INTERVAL);
        tryTimes--;
    }
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("Init KvStorePtr failed");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::InitLocal()
{
    this->isAutoSync_ = false;
    this->dataType_ = DistributedKv::DataType::TYPE_STATICS;
    DHLOGI("Init local DB, storeId: %{public}s, dataType: %{public}d",
        storeId_.storeId.c_str(), static_cast<int32_t>(this->dataType_));
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    int32_t tryTimes = MAX_INIT_RETRY_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetLocalKvStorePtr();
        if (status == DistributedKv::Status::SUCCESS && kvStoragePtr_) {
            DHLOGI("Init KvStorePtr Success");
            RegisterKvStoreDeathListener();
            return DH_FWK_SUCCESS;
        }
        DHLOGD("CheckKvStore, left times: %{public}d", tryTimes);
        usleep(INIT_RETRY_SLEEP_INTERVAL);
        tryTimes--;
    }
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("Init KvStorePtr failed");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    return DH_FWK_SUCCESS;
}

void DBAdapter::UnInit()
{
    DHLOGI("DBAdapter UnInit");
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return;
    }
    UnRegisterKvStoreDeathListener();
    if (this->isAutoSync_) {
        UnRegisterChangeListener();
    }
    kvStoragePtr_.reset();
}

int32_t DBAdapter::ReInit(bool isAutoSync)
{
    DHLOGI("ReInit DB, storeId: %{public}s", storeId_.storeId.c_str());
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    kvStoragePtr_.reset();
    DistributedKv::Status status = this->isAutoSync_ ?
        GetKvStorePtr(isAutoSync, this->dataType_) : GetLocalKvStorePtr();
    if (status != DistributedKv::Status::SUCCESS || !kvStoragePtr_) {
        DHLOGW("Get kvStoragePtr_ failed, status: %{public}d", status);
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    RegisterKvStoreDeathListener();
    return DH_FWK_SUCCESS;
}

std::string DBAdapter::GetNetworkIdByKey(const std::string &key)
{
    if (!IsIdLengthValid(key)) {
        return "";
    }
    DHLOGI("Get networkId by key: %{public}s", GetAnonyString(key).c_str());
    std::string deviceId = DHContext::GetInstance().GetDeviceIdByDBGetPrefix(key);
    if (deviceId.empty()) {
        DHLOGW("Get deviceId empty, key: %{public}s", GetAnonyString(key).c_str());
        return "";
    }

    if (deviceId == DHContext::GetInstance().GetDeviceInfo().deviceId) {
        DHLOGW("Query local db info, no need sync");
        return "";
    }

    std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(deviceId);
    if (uuid.empty()) {
        DHLOGW("Get uuid empty, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
        return "";
    }
    if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
        DHLOGW("The device not online, no need sync, uuid: %{public}s, deviceId: %{public}s",
            GetAnonyString(uuid).c_str(), GetAnonyString(deviceId).c_str());
        return "";
    }
    return DHContext::GetInstance().GetNetworkIdByUUID(uuid);
}

void DBAdapter::SyncByNotFound(const std::string &key)
{
    if (!IsIdLengthValid(key)) {
        return;
    }
    std::string networkId = GetNetworkIdByKey(key);
    if (networkId.empty()) {
        DHLOGW("The networkId emtpy.");
        return;
    }
    DHLOGI("Try sync data by key: %{public}s, storeId: %{public}s", GetAnonyString(key).c_str(),
        storeId_.storeId.c_str());
    std::vector<std::string> networkIdVec;
    networkIdVec.push_back(networkId);
    kvStoragePtr_->Sync(networkIdVec, DistributedKv::SyncMode::PUSH_PULL);
    return;
}

int32_t DBAdapter::GetDataByKey(const std::string &key, std::string &data)
{
    if (!IsIdLengthValid(key)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Get data by key: %{public}s, storeId: %{public}s, dataType: %{public}d",
        GetAnonyString(key).c_str(), storeId_.storeId.c_str(), static_cast<int32_t>(this->dataType_));
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Key kvKey(key);
    DistributedKv::Value kvValue;
    DistributedKv::Status status = kvStoragePtr_->Get(kvKey, kvValue);
    if (status == DistributedKv::Status::NOT_FOUND) {
        if (this->dataType_ == DistributedKv::DataType::TYPE_DYNAMICAL) {
            SyncByNotFound(key);
        }
#ifdef DHARDWARE_OPEN_SOURCE
        if (this->dataType_ == DistributedKv::DataType::TYPE_STATICS && this->storeId_.storeId == GLOBAL_META_INFO) {
            SyncByNotFound(key);
        }
#endif
    }
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Query from db failed, key: %{public}s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    data = kvValue.ToString();
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::GetDataByKeyPrefix(const std::string &keyPrefix, std::vector<std::string> &values)
{
    DHLOGI("Get data by key prefix: %{public}s, storeId: %{public}s, dataType: %{public}d",
        GetAnonyString(keyPrefix).c_str(), storeId_.storeId.c_str(), static_cast<int32_t>(this->dataType_));
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    // if prefix is empty, get all entries.
    DistributedKv::Key allEntryKeyPrefix(keyPrefix);
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStoragePtr_->GetEntries(allEntryKeyPrefix, allEntries);
    if (status == DistributedKv::Status::SUCCESS && allEntries.size() == 0) {
        if (this->dataType_ == DistributedKv::DataType::TYPE_DYNAMICAL) {
            SyncByNotFound(keyPrefix);
        }
#ifdef DHARDWARE_OPEN_SOURCE
        if (this->dataType_ == DistributedKv::DataType::TYPE_STATICS && this->storeId_.storeId == GLOBAL_META_INFO) {
            SyncByNotFound(keyPrefix);
        }
#endif
    }
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Query data by keyPrefix failed, prefix: %{public}s",
            GetAnonyString(keyPrefix).c_str());
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    if (allEntries.empty() || allEntries.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("AllEntries is empty or too large!");
        return ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID;
    }
    for (const auto& item : allEntries) {
        values.push_back(item.value.ToString());
    }
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::PutData(const std::string &key, const std::string &value)
{
    if (!IsIdLengthValid(key) || !IsMessageLengthValid(value)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Key kvKey(key);
    DistributedKv::Value kvValue(value);
    DistributedKv::Status status = kvStoragePtr_->Put(kvKey, kvValue);
    if (status == DistributedKv::Status::IPC_ERROR) {
        DHLOGE("Put kv to db failed, ret: %{public}d", status);
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::PutDataBatch(const std::vector<std::string> &keys, const std::vector<std::string> &values)
{
    if (!IsArrayLengthValid(keys) || !IsArrayLengthValid(values)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    if (keys.size() != values.size() || keys.empty() || values.empty()) {
        DHLOGE("Param is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::vector<DistributedKv::Entry> entries;
    for (unsigned long i = 0; i < keys.size(); i++) {
        DistributedKv::Entry entry;
        entry.key = keys[i];
        entry.value = values[i];
        entries.push_back(entry);
    }
    DistributedKv::Status status = kvStoragePtr_->PutBatch(entries);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Put kv batch to db failed, ret: %{public}d", status);
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    DHLOGI("Put kv batch to db success");
    return DH_FWK_SUCCESS;
}

void DBAdapter::SyncDBForRecover()
{
    DHLOGI("Sync store id: %{public}s after db recover", storeId_.storeId.c_str());
    if (storeId_.storeId == GLOBAL_CAPABILITY_ID) {
        AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(EVENT_CAPABILITY_INFO_DB_RECOVER);
        CapabilityInfoManager::GetInstance()->GetEventHandler()->SendEvent(msgEvent,
            0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }

    if (storeId_.storeId == GLOBAL_VERSION_ID) {
        AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(EVENT_VERSION_INFO_DB_RECOVER);
        VersionInfoManager::GetInstance()->GetEventHandler()->SendEvent(msgEvent,
            0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }

    if (storeId_.storeId == GLOBAL_META_INFO) {
        AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(EVENT_META_INFO_DB_RECOVER);
        MetaInfoManager::GetInstance()->GetEventHandler()->SendEvent(msgEvent,
            0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
}

int32_t DBAdapter::RegisterChangeListener()
{
    DHLOGI("Register db data change listener");
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Status status = kvStoragePtr_->SubscribeKvStore(DistributedKv::SubscribeType::SUBSCRIBE_TYPE_REMOTE,
        dataChangeListener_);
    if (status == DistributedKv::Status::IPC_ERROR) {
        DHLOGE("Register db data change listener failed, ret: %{public}d", status);
        return ERR_DH_FWK_RESOURCE_REGISTER_DB_FAILED;
    }
    status = kvStoragePtr_->SubscribeKvStore(DistributedKv::SubscribeType::SUBSCRIBE_TYPE_CLOUD,
        dataChangeListener_);
    if (status == DistributedKv::Status::IPC_ERROR) {
        DHLOGE("Register db cloud data change listener failed, ret: %{public}d", status);
        return ERR_DH_FWK_RESOURCE_REGISTER_DB_FAILED;
    }
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::UnRegisterChangeListener()
{
    DHLOGI("UnRegister db data change listener");
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Status status = kvStoragePtr_->UnSubscribeKvStore(
        DistributedKv::SubscribeType::SUBSCRIBE_TYPE_REMOTE, dataChangeListener_);
    if (status == DistributedKv::Status::IPC_ERROR) {
        DHLOGE("UnRegister db data change listener failed, ret: %{public}d", status);
        return ERR_DH_FWK_RESOURCE_UNREGISTER_DB_FAILED;
    }
    status = kvStoragePtr_->UnSubscribeKvStore(
        DistributedKv::SubscribeType::SUBSCRIBE_TYPE_CLOUD, dataChangeListener_);
    if (status == DistributedKv::Status::IPC_ERROR) {
        DHLOGE("UnRegister db cloud data change listener failed, ret: %{public}d", status);
        return ERR_DH_FWK_RESOURCE_UNREGISTER_DB_FAILED;
    }
    return DH_FWK_SUCCESS;
}

void DBAdapter::RegisterKvStoreDeathListener()
{
    DHLOGI("Register kvStore death listener");
    kvDataMgr_.RegisterKvStoreServiceDeathRecipient(shared_from_this());
}

void DBAdapter::UnRegisterKvStoreDeathListener()
{
    DHLOGI("UnRegister kvStore death listener");
    kvDataMgr_.UnRegisterKvStoreServiceDeathRecipient(shared_from_this());
}

void DBAdapter::OnRemoteDied()
{
    DHLOGI("OnRemoteDied, recover db begin");
    auto reInitTask = [this] {
        int32_t times = 0;
        while (times < DIED_CHECK_MAX_TIMES) {
            if (DBDiedOpt(times)) {
                DHLOGI("ReInit DB success");
                break;
            }
        }
    };
    DHContext::GetInstance().GetEventHandler()->PostTask(reInitTask, "reInitTask", 0);
    DHLOGI("OnRemoteDied, recover db end");
}

bool DBAdapter::DBDiedOpt(int32_t &times)
{
    // init kvStore.
    if (this->ReInit(this->isAutoSync_) == DH_FWK_SUCCESS) {
        // register data change listener again.
        if (this->isAutoSync_) {
            this->RegisterChangeListener();
        }
        this->SyncDBForRecover();
        DHLOGE("Current times is %{public}d", times);
        return true;
    }
    times++;
    usleep(DIED_CHECK_INTERVAL);
    return false;
}

void DBAdapter::DeleteKvStore()
{
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    DistributedKv::Status status = kvDataMgr_.DeleteKvStore(appId_, storeId_);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("DeleteKvStore error, appId: %{public}s, storeId: %{public}s, status: %{public}d",
            appId_.appId.c_str(), storeId_.storeId.c_str(), status);
        return;
    }
    DHLOGI("DeleteKvStore success appId: %{public}s", appId_.appId.c_str());
}

int32_t DBAdapter::RemoveDeviceData(const std::string &deviceId)
{
    if (!IsIdLengthValid(deviceId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Status status = kvStoragePtr_->RemoveDeviceData(deviceId);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Remove device data failed, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    DHLOGD("Remove device data success, deviceId: %{public}s", GetAnonyString(deviceId).c_str());
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::RemoveDataByKey(const std::string &key)
{
    if (!IsIdLengthValid(key)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Key kvKey(key);
    DistributedKv::Status status = kvStoragePtr_->Delete(kvKey);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Remove data by key failed");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    DHLOGD("Remove data by key success");
    return DH_FWK_SUCCESS;
}

std::vector<DistributedKv::Entry> DBAdapter::GetEntriesByKeys(const std::vector<std::string> &keys)
{
    if (!IsArrayLengthValid(keys)) {
        return {};
    }
    DHLOGI("call");
    std::vector<DistributedKv::Entry> entries;
    {
        std::lock_guard<std::mutex> lock(dbAdapterMutex_);
        if (kvStoragePtr_ == nullptr) {
            DHLOGE("kvStoragePtr_ is nullptr!");
            return entries;
        }
        for (const auto &key : keys) {
            DistributedKv::Key kvKey(key);
            DistributedKv::Value kvValue;
            if (kvStoragePtr_->Get(kvKey, kvValue) != DistributedKv::Status::SUCCESS) {
                continue;
            }
            DistributedKv::Entry entry;
            entry.key = kvKey;
            entry.value = kvValue;
            entries.emplace_back(entry);
        }
    }
    return entries;
}

bool DBAdapter::SyncDataByNetworkId(const std::string &networkId)
{
    DHLOGI("Try initiative sync data by networId: %{public}s", GetAnonyString(networkId).c_str());
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is nullptr!");
        return false;
    }
    std::vector<std::string> networkIdVec;
    networkIdVec.push_back(networkId);
    DistributedKv::Status status = kvStoragePtr_->Sync(networkIdVec, DistributedKv::SyncMode::PUSH_PULL);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("initiative sync data failed");
        return false;
    }
    return true;
}

bool DBAdapter::ClearDataWhenPeerLogout(const std::string &peerudid, const std::string &peeruuid)
{
    DHLOGI("Clear cloudData start.");
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is nullptr!");
        return false;
    }
    std::string udIdHash = Sha256(peerudid);
    DistributedKv::Key allEntryKeyPrefix(udIdHash);
    std::vector<DistributedKv::Entry> peerEntries;
    DistributedKv::Status status = kvStoragePtr_->GetEntries(allEntryKeyPrefix, peerEntries);
    if (status != DistributedKv::Status::SUCCESS || peerEntries.size() == 0) {
        DHLOGE("GetEntries error: %{public}d, or peerEntries is empty", status);
        return false;
    }
    std::vector<DistributedKv::Key> peerkeys;
    for (const auto &entry : peerEntries) {
        peerkeys.push_back(entry.key);
    }

    if (kvStoragePtr_->DeleteBatch(peerkeys) != DistributedKv::Status::SUCCESS) {
        DHLOGE("DeleteBatch failed, error: %{public}d", status);
        return false;
    }

    if (kvStoragePtr_->RemoveDeviceData(peeruuid) != DistributedKv::Status::SUCCESS) {
        DHLOGE("RemoveDeviceData failed, peeruuid=%{public}s", GetAnonyString(peeruuid).c_str());
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
