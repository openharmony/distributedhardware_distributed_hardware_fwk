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

#include "component_manager.h"

#include <future>
#include <string>

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "component_disable.h"
#include "component_enable.h"
#include "component_loader.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "ipc_object_stub.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "version_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentManager"

IMPLEMENT_SINGLE_INSTANCE(ComponentManager);

namespace {
    constexpr int32_t ENABLE_RETRY_MAX_TIMES = 30;
    constexpr int32_t DISABLE_RETRY_MAX_TIMES = 30;
    constexpr int32_t ENABLE_PARAM_RETRY_TIME = 500 * 1000;
}

ComponentManager::~ComponentManager()
{
    DHLOGD("start.");
}

int32_t ComponentManager::Init()
{
    DHLOGI("start.");
    if (!InitCompSource()) {
        DHLOGE("InitCompSource failed.");
        return ERR_DH_FWK_COMPONENT_INIT_SOURCE_FAILED;
    }
    if (!InitCompSink()) {
        DHLOGE("InitCompSink failed.");
        compSource_.clear();
        return ERR_DH_FWK_COMPONENT_INIT_SINK_FAILED;
    }

    auto sourceResult = StartSource();
    auto sinkResult = StartSink();

    if (!WaitForResult(Action::START_SOURCE, sourceResult)) {
        DHLOGE("StartSource failed, some virtual components maybe cannot work, but want to continue");
    }
    if (!WaitForResult(Action::START_SINK, sinkResult)) {
        DHLOGE("StartSink failed, some virtual components maybe cannot work, but want to continue");
    }

    DHLOGI("Init component success");

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::UnInit()
{
    DHLOGI("start.");
    auto sourceResult = StopSource();
    auto sinkResult = StopSink();

    if (!WaitForResult(Action::STOP_SOURCE, sourceResult)) {
        DHLOGE("StopSource failed, but want to continue");
    }
    if (!WaitForResult(Action::STOP_SINK, sinkResult)) {
        DHLOGE("StopSource failed, but want to continue");
    }

    compSource_.clear();
    compSink_.clear();
    {
        std::lock_guard<std::mutex> lock(sinkVersionMutex_);
        sinkVersions_.clear();
    }
    DHLOGI("Release component success");
    return DH_FWK_SUCCESS;
}

ComponentManager::ActionResult ComponentManager::StartSource()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    for (const auto &item : compSource_) {
        CompVersion compversion;
        VersionManager::GetInstance().GetCompVersion(uuid, item.first, compversion);
        auto params = compversion.sourceVersion;
        auto future = std::async(std::launch::async, [item, params]() { return item.second->InitSource(params); });
        futures.emplace(item.first, future.share());
    }
    return futures;
}

ComponentManager::ActionResult ComponentManager::StartSink()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    for (const auto &item : compSink_) {
        CompVersion compversion;
        VersionManager::GetInstance().GetCompVersion(uuid, item.first, compversion);
        auto params = compversion.sinkVersion;
        auto future = std::async(std::launch::async, [item, params]() { return item.second->InitSink(params); });
        futures.emplace(item.first, future.share());
    }
    return futures;
}

ComponentManager::ActionResult ComponentManager::StopSource()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    for (const auto &item : compSource_) {
        auto future = std::async(std::launch::async, [item]() { return item.second->ReleaseSource(); });
        futures.emplace(item.first, future.share());
    }
    return futures;
}

ComponentManager::ActionResult ComponentManager::StopSink()
{
    DHLOGI("start.");
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    for (const auto &item : compSink_) {
        auto future = std::async(std::launch::async, [item]() {
            int32_t status = item.second->ReleaseSink();
            IHardwareHandler *hardwareHandler = nullptr;
            status = ComponentLoader::GetInstance().GetHardwareHandler(item.first, hardwareHandler);
            if (status != DH_FWK_SUCCESS || hardwareHandler == nullptr) {
                DHLOGE("GetHardwareHandler %#X failed", item.first);
                return status;
            }
            hardwareHandler->UnRegisterPluginListener();
            return status;
        });

        futures.emplace(item.first, future.share());
    }
    return futures;
}

bool ComponentManager::WaitForResult(const Action &action, ActionResult actionsResult)
{
    DHLOGD("start.");
    auto ret = true;
    for (auto &iter : actionsResult) {
        auto result = iter.second.get();
        DHLOGI("action = %d, compType = %#X, ret = %d.", static_cast<int32_t>(action), iter.first, result);
        if (result != DH_FWK_SUCCESS) {
            ret = false;
            DHLOGE("there is error, but want to continue.");
        }
    }
    DHLOGD("end.");
    return ret;
}

bool ComponentManager::InitCompSource()
{
    auto compTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    for (const auto &type : compTypes) {
        IDistributedHardwareSource *sourcePtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSource(type, sourcePtr);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGW("GetSource failed, compType = %#X, ret = %d.", type, ret);
            continue;
        }
        if (sourcePtr == nullptr) {
            DHLOGW("sourcePtr is null, compType = %#X.", type);
            continue;
        }
        compSource_.insert(std::make_pair(type, sourcePtr));
    }
    return !compSource_.empty();
}

bool ComponentManager::InitCompSink()
{
    auto compTypes = ComponentLoader::GetInstance().GetAllCompTypes();
    for (const auto &type : compTypes) {
        IDistributedHardwareSink *sinkPtr = nullptr;
        auto ret = ComponentLoader::GetInstance().GetSink(type, sinkPtr);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGW("GetSink failed, compType = %#X, ret = %d.", type, ret);
            continue;
        }
        if (sinkPtr == nullptr) {
            DHLOGW("sinkPtr is null, compType = %#X.", type);
            continue;
        }
        compSink_.insert(std::make_pair(type, sinkPtr));
    }
    return !compSink_.empty();
}

int32_t ComponentManager::Enable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType)
{
    DHLOGI("start.");
    auto find = compSource_.find(dhType);
    if (find == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %s.", dhId.c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    EnableParam param;
    auto ret = GetEnableParam(networkId, uuid, dhId, dhType, param);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetEnableParam failed, uuid = %s, dhId = %s, errCode = %d", GetAnonyString(uuid).c_str(),
            dhId.c_str(), ret);
        for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
            if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already offline, no need try GetEnableParam, uuid = %s",
                    GetAnonyString(uuid).c_str());
                return ret;
            }
            if (GetEnableParam(networkId, uuid, dhId, dhType, param) == DH_FWK_SUCCESS) {
                DHLOGE("GetEnableParam success, retryCount = %d", retryCount);
                break;
            }
            DHLOGE("GetEnableParam failed, retryCount = %d", retryCount);
            usleep(ENABLE_PARAM_RETRY_TIME);
        }
    }
    auto compEnable = std::make_shared<ComponentEnable>();
    auto result = compEnable->Enable(networkId, dhId, param, find->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
            if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already offline, no need try enable, uuid = %s", GetAnonyString(uuid).c_str());
                return result;
            }
            if (compEnable->Enable(networkId, dhId, param, find->second) == DH_FWK_SUCCESS) {
                DHLOGE("enable success, retryCount = %d", retryCount);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("enable failed, retryCount = %d", retryCount);
        }
        return result;
    }
    DHLOGI("enable result is %d, uuid = %s, dhId = %s", result, GetAnonyString(uuid).c_str(), dhId.c_str());
    return result;
}

int32_t ComponentManager::Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType)
{
    auto find = compSource_.find(dhType);
    if (find == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %s.", dhId.c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    auto compDisable = std::make_shared<ComponentDisable>();
    auto result = compDisable->Disable(networkId, dhId, find->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < DISABLE_RETRY_MAX_TIMES; retryCount++) {
            if (DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already online, no need try disable, uuid = %s", GetAnonyString(uuid).c_str());
                return result;
            }
            if (compDisable->Disable(networkId, dhId, find->second) == DH_FWK_SUCCESS) {
                DHLOGE("disable success, retryCount = %d", retryCount);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("disable failed, retryCount = %d", retryCount);
        }
        return result;
    }
    DHLOGI("disable result is %d, uuid = %s, dhId = %s", result, GetAnonyString(uuid).c_str(), dhId.c_str());
    return result;
}

DHType ComponentManager::GetDHType(const std::string &uuid, const std::string &dhId) const
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(GetDeviceIdByUUID(uuid), dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        return capability->GetDHType();
    }
    DHLOGE("get dhType failed, uuid = %s, dhId = %s", GetAnonyString(uuid).c_str(), dhId.c_str());
    return DHType::UNKNOWN;
}

int32_t ComponentManager::GetEnableParam(const std::string &networkId, const std::string &uuid,
    const std::string &dhId, DHType dhType, EnableParam &param)
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(GetDeviceIdByUUID(uuid), dhId, capability);
    if ((ret != DH_FWK_SUCCESS) || (capability == nullptr)) {
        DHLOGE("GetCapability failed, uuid =%s, dhId = %s, errCode = %d", GetAnonyString(uuid).c_str(), dhId.c_str(),
            ret);
        return ret;
    }

    param.attrs = capability->GetDHAttrs();
    param.version = GetSinkVersion(networkId, uuid, dhType);
    if (param.version.empty()) {
        DHLOGI("Get Sink Version failed, uuid = %s, dhId = %s", GetAnonyString(uuid).c_str(), dhId.c_str());
        return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
    }

    DHLOGI("success. uuid =%s, dhId = %s, version = %s", GetAnonyString(uuid).c_str(), dhId.c_str(),
        param.version.c_str());

    return DH_FWK_SUCCESS;
}

std::string ComponentManager::GetSinkVersion(const std::string &networkId, const std::string &uuid, DHType dhType)
{
    DHLOGI("networkId = %s ", GetAnonyString(networkId).c_str());
    auto sinkVersion = GetVersionFromCache(uuid, dhType);
    if (!sinkVersion.empty()) {
        DHLOGI("GetVersionFromCache success, sinkVersion = %s, uuid = %s, dhType = %#X", sinkVersion.c_str(),
            GetAnonyString(uuid).c_str(), dhType);
        return sinkVersion;
    }

    auto updateResult = UpdateVersionCache(networkId, uuid);
    if (updateResult != DH_FWK_SUCCESS) {
        DHLOGE("UpdateVersionCache failed, uuid = %s, errCode = %d", GetAnonyString(uuid).c_str(), updateResult);
        return "";
    }

    sinkVersion = GetVersionFromCache(uuid, dhType);
    return sinkVersion;
}

std::string ComponentManager::GetVersionFromCache(const std::string &uuid, DHType dhType)
{
    std::lock_guard<std::mutex> lock(sinkVersionMutex_);
    auto iter = sinkVersions_.find(uuid);
    if (iter == sinkVersions_.end()) {
        DHLOGE("can not find component version for uuid = %s", GetAnonyString(uuid).c_str());
        return "";
    }

    auto find = iter->second.find(dhType);
    if (find == iter->second.end()) {
        DHLOGE("can not find component version for uuid = %s, dhType = %#X", uuid.c_str(), dhType);
        return "";
    }
    return find->second;
}

int32_t ComponentManager::UpdateVersionCache(const std::string &networkId, const std::string &uuid)
{
    sptr<IDistributedHardware> dhms = GetRemoteDHMS(networkId);
    if (dhms == nullptr) {
        DHLOGI("GetRemoteDHMS failed, networkId = %s", GetAnonyString(networkId).c_str());
        return ERR_DH_FWK_COMPONENT_GET_REMOTE_SA_FAILED;
    }

    std::unordered_map<DHType, std::string> versions;
    auto ret = dhms->QuerySinkVersion(versions);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("QuerySinkVersion failed, errCode = %d", ret);
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(sinkVersionMutex_);
        sinkVersions_.emplace(uuid, versions);
    }
    DHLOGI("QuerySinkVersion success");
    return DH_FWK_SUCCESS;
}

sptr<IDistributedHardware> ComponentManager::GetRemoteDHMS(const std::string &networkId) const
{
    DHLOGI("start, networkId = %s", GetAnonyString(networkId).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return nullptr;
    }
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DHLOGE("GetSystemAbilityManager failed");
        return nullptr;
    }
    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, networkId);
    if (object == nullptr) {
        DHLOGE("CheckSystemAbility failed");
        return nullptr;
    }
    return iface_cast<IDistributedHardware>(object);
}
} // namespace DistributedHardware
} // namespace OHOS
