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

#include "db_adapter.h"

#include "anonymous_string.h"
#include "capability_info.h"
#include "capability_info_manager.h"
#include "capability_utils.h"
#include "constants.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "event_bus.h"
#include "version_info_event.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DBAdapter"

namespace {
const std::string DATABASE_DIR = "/data/service/el1/public/database/";
}

DBAdapter::DBAdapter(const std::string &appId, const std::string &storeId,
                     const std::shared_ptr<DistributedKv::KvStoreObserver> &changeListener)
{
    this->appId_.appId = appId;
    this->storeId_.storeId = storeId;
    this->dataChangeListener_ = changeListener;
    DHLOGI("DBAdapter Constructor Success, appId: %s, storeId: %s", appId.c_str(), storeId.c_str());
}

DBAdapter::~DBAdapter()
{
    DHLOGI("DBAdapter Destruction");
}

DistributedKv::Status DBAdapter::GetKvStorePtr()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = true,
        .securityLevel = DistributedKv::SecurityLevel::S1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .area = DistributedKv::EL1,
        .baseDir = DATABASE_DIR + appId_.appId
    };
    return kvDataMgr_.GetSingleKvStore(options, appId_, storeId_, kvStoragePtr_);
}

int32_t DBAdapter::Init()
{
    DHLOGI("Init DB, storeId: %s", storeId_.storeId.c_str());
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    int32_t tryTimes = MAX_INIT_RETRY_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStorePtr();
        if (status == DistributedKv::Status::SUCCESS && kvStoragePtr_) {
            DHLOGI("Init KvStorePtr Success");
            RegisterManualSyncListener();
            RegisterChangeListener();
            RegisterKvStoreDeathListener();
            return DH_FWK_SUCCESS;
        }
        DHLOGD("CheckKvStore, left times: %d", tryTimes);
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
    UnRegisterChangeListener();
    UnRegisterManualSyncListener();
    kvStoragePtr_.reset();
}

int32_t DBAdapter::ReInit()
{
    DHLOGI("ReInit DB, storeId: %s", storeId_.storeId.c_str());
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    UnRegisterManualSyncListener();
    kvStoragePtr_.reset();
    DistributedKv::Status status = GetKvStorePtr();
    if (status != DistributedKv::Status::SUCCESS || !kvStoragePtr_) {
        DHLOGW("Get kvStoragePtr_ failed, status: %d", status);
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    RegisterManualSyncListener();
    RegisterKvStoreDeathListener();
    return DH_FWK_SUCCESS;
}

void DBAdapter::SyncCompleted(const std::map<std::string, DistributedKv::Status> &results)
{
    DHLOGI("DBAdapter SyncCompleted start");
    if (results.size() == 0 || results.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("Results size is invalid!");
        return;
    }
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    for (const auto &result : results) {
        std::string deviceId = result.first;
        DHLOGI("SyncCompleted, deviceId: %s", GetAnonyString(deviceId).c_str());
        if (manualSyncCountMap_.count(deviceId) == 0) {
            DHLOGE("SyncCompleted, error, ManualSyncCount is removed");
            return;
        }
        DHLOGI("ManualSyncCallback::SyncCompleted, retryCount: %d", manualSyncCountMap_[deviceId]);
        if (result.second == DistributedKv::Status::SUCCESS) {
            manualSyncCountMap_[deviceId] = 0;
        } else {
            manualSyncCountMap_[deviceId]++;
            if (manualSyncCountMap_[deviceId] >= MANUAL_SYNC_TIMES) {
                manualSyncCountMap_[deviceId] = 0;
            } else {
                auto retryTask = [this, deviceId] {
                    this->ManualSync(deviceId);
                    usleep(MANUAL_SYNC_INTERVAL);
                };
                DHContext::GetInstance().GetEventBus()->PostTask(retryTask, "retryTask", 0);
            }
        }
    }
}

int32_t DBAdapter::GetDataByKey(const std::string &key, std::string &data)
{
    DHLOGI("Get data by key: %s", GetAnonyString(key).c_str());
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Key kvKey(key);
    DistributedKv::Value kvValue;
    DistributedKv::Status status = kvStoragePtr_->Get(kvKey, kvValue);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Query from db failed, key: %s", GetAnonyString(key).c_str());
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    data = kvValue.ToString();
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::GetDataByKeyPrefix(const std::string &keyPrefix, std::vector<std::string> &values)
{
    DHLOGI("Get data by key prefix: %s", GetAnonyString(keyPrefix).c_str());
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }

    // if prefix is empty, get all entries.
    DistributedKv::Key allEntryKeyPrefix(keyPrefix);
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Status status = kvStoragePtr_->GetEntries(allEntryKeyPrefix, allEntries);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Query data by keyPrefix failed, prefix: %s",
            GetAnonyString(keyPrefix).c_str());
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    if (allEntries.size() == 0 || allEntries.size() > MAX_DB_RECORD_SIZE) {
        DHLOGE("AllEntries size is invalid!");
        return ERR_DH_FWK_PARA_INVALID;
    }
    for (const auto& item : allEntries) {
        values.push_back(item.value.ToString());
    }
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::PutData(const std::string &key, const std::string &value)
{
    if (key.empty() || key.size() > MAX_MESSAGE_LEN || value.empty() || value.size() > MAX_MESSAGE_LEN) {
        DHLOGI("Param is invalid!");
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
        DHLOGE("Put kv to db failed, ret: %d", status);
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::PutDataBatch(const std::vector<std::string> &keys, const std::vector<std::string> &values)
{
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
        DHLOGE("Put kv batch to db failed, ret: %d", status);
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    DHLOGI("Put kv batch to db success");
    return DH_FWK_SUCCESS;
}

void DBAdapter::CreateManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    manualSyncCountMap_[deviceId] = 0;
}

void DBAdapter::RemoveManualSyncCount(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    manualSyncCountMap_.erase(deviceId);
}

int32_t DBAdapter::ManualSync(const std::string &networkId)
{
    DHLOGI("Manual sync between networkId: %s", GetAnonyString(networkId).c_str());
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    std::vector<std::string> devList = { networkId };
    DistributedKv::Status status = kvStoragePtr_->Sync(devList, DistributedKv::SyncMode::PULL);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("ManualSync Data failed, networkId: %s", GetAnonyString(networkId).c_str());
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

void DBAdapter::SyncDBForRecover()
{
    DHLOGI("Sync store id: %s after db recover", storeId_.storeId.c_str());
    if (storeId_.storeId == GLOBAL_CAPABILITY_ID) {
        CapabilityInfoEvent recoverEvent(*this, CapabilityInfoEvent::EventType::RECOVER);
        DHContext::GetInstance().GetEventBus()->PostEvent<CapabilityInfoEvent>(recoverEvent);
    }

    if (storeId_.storeId == GLOBAL_VERSION_ID) {
        VersionInfoEvent recoverEvent(*this, VersionInfoEvent::EventType::RECOVER);
        DHContext::GetInstance().GetEventBus()->PostEvent<VersionInfoEvent>(recoverEvent);
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
        DHLOGE("Register db data change listener failed, ret: %d", status);
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
        DHLOGE("UnRegister db data change listener failed, ret: %d", status);
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

void DBAdapter::RegisterManualSyncListener()
{
    DHLOGI("Register manualSyncCallback");
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return;
    }
    kvStoragePtr_->RegisterSyncCallback(shared_from_this());
}

void DBAdapter::UnRegisterManualSyncListener()
{
    DHLOGI("UnRegister manualSyncCallback");
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return;
    }
    kvStoragePtr_->UnRegisterSyncCallback();
}

void DBAdapter::OnRemoteDied()
{
    DHLOGI("OnRemoteDied, recover db begin");
    auto ReInitTask = [this] {
        int32_t times = 0;
        while (times < DIED_CHECK_MAX_TIMES) {
            // init kvStore.
            if (this->ReInit() == DH_FWK_SUCCESS) {
                // register data change listener again.
                this->RegisterChangeListener();
                this->SyncDBForRecover();
                DHLOGE("Current times is %d", times);
                break;
            }
            times++;
            usleep(DIED_CHECK_INTERVAL);
        }
    };
    DHContext::GetInstance().GetEventBus()->PostTask(ReInitTask, "ReInitTask", 0);
    DHLOGI("OnRemoteDied, recover db end");
}

void DBAdapter::DeleteKvStore()
{
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    DistributedKv::Status status = kvDataMgr_.DeleteKvStore(appId_, storeId_);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("DeleteKvStore error, appId: %s, storeId: %s, status: %d",
            appId_.appId.c_str(), storeId_.storeId.c_str(), status);
        return;
    }
    DHLOGI("DeleteKvStore success appId: %s", appId_.appId.c_str());
}

int32_t DBAdapter::RemoveDeviceData(const std::string &deviceId)
{
    std::lock_guard<std::mutex> lock(dbAdapterMutex_);
    if (kvStoragePtr_ == nullptr) {
        DHLOGE("kvStoragePtr_ is null");
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL;
    }
    DistributedKv::Status status = kvStoragePtr_->RemoveDeviceData(deviceId);
    if (status != DistributedKv::Status::SUCCESS) {
        DHLOGE("Remove device data failed, deviceId: %s", GetAnonyString(deviceId).c_str());
        return ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL;
    }
    DHLOGD("Remove device data success, deviceId: %s", GetAnonyString(deviceId).c_str());
    return DH_FWK_SUCCESS;
}

int32_t DBAdapter::RemoveDataByKey(const std::string &key)
{
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
} // namespace DistributedHardware
} // namespace OHOS
