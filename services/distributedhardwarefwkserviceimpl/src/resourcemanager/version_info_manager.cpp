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

#include "version_info_manager.h"

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
#define DH_LOG_TAG "VersionInfoManager"
IMPLEMENT_SINGLE_INSTANCE(VersionInfoManager);

int32_t VersionInfoManager::Init()
{
    DHLOGI("VersionInfoManager instance init!");
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    dbAdapterPtr_ = std::make_shared<DBAdapter>(APP_ID, GLOBAL_VERSION_ID, shared_from_this());
    if (dbAdapterPtr_->Init() != DH_FWK_SUCCESS) {
        DHLOGE("Init dbAdapterPtr_ failed");
        return ERR_DH_FWK_RESOURCE_INIT_DB_FAILED;
    }
    VersionInfoEvent versionInfoEvent(*this);
    DHContext::GetInstance().GetEventBus()->AddHandler<VersionInfoEvent>(versionInfoEvent.GetType(), *this);
    DHLOGI("VersionInfoManager instance init success");
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoManager::UnInit()
{
    DHLOGI("VersionInfoManager UnInit");
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED;
    }
    dbAdapterPtr_->UnInit();
    dbAdapterPtr_.reset();
    return DH_FWK_SUCCESS;
}

int32_t VersionInfoManager::AddVersion(const std::vector<DHVersion> &versions)
{
    std::lock_guard<std::mutex> lock(verInfoMgrMutex_);
    if (dbAdapterPtr_ == nullptr) {
        DHLOGE("dbAdapterPtr_ is null");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL;
    }
    std::vector<std::string> keys;
    std::vector<std::string> values;
    for (auto &version : versions) {
        const std::string key = version.deviceId;
        DHLOGI("AddVersion, Key: %s", GetAnonyString(version.deviceId).c_str());
        keys.push_back(key);
        // cstd::string value = version.ToJsonString();
        values.push_back(version.ToJsonString());
        // VersionManager::GetInstance().AddDHVersion(version.uuid, version);
    }
    if (dbAdapterPtr_->PutDataBatch(keys, values) != DH_FWK_SUCCESS) {
        DHLOGE("Fail to storage batch to kv");
        return ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL;
    }
    return DH_FWK_SUCCESS;
}

void VersionInfoManager::OnChange(const DistributedKv::ChangeNotification &changeNotification)
{

}

void VersionInfoManager::OnEvent(VersionInfoEvent &e)
{

}
} // namespace DistributedHardware
} // namespace OHOS