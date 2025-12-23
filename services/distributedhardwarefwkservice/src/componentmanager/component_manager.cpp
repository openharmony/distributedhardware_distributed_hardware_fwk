/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <chrono>
#include <future>
#include <pthread.h>
#include <string>
#include <thread>

#include "ffrt.h"
#include "ipc_object_stub.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "anonymous_string.h"
#include "access_listener_service.h"
#include "capability_info_manager.h"
#include "component_disable.h"
#include "component_enable.h"
#include "component_loader.h"
#include "constants.h"
#include "hdf_operate.h"
#include "device_manager.h"
#include "dh_context.h"
#include "dh_data_sync_trigger_listener.h"
#include "dh_state_listener.h"
#include "dh_utils_hitrace.h"
#include "dh_utils_hisysevent.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "enabled_comps_dump.h"
#include "local_capability_info_manager.h"
#include "low_latency.h"
#include "meta_info_manager.h"
#include "publisher.h"
#include "task_executor.h"
#include "task_factory.h"
#include "version_info_manager.h"
#include "version_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentManager"

IMPLEMENT_SINGLE_INSTANCE(ComponentManager);

namespace {
    constexpr int32_t ENABLE_RETRY_MAX_TIMES = 3;
    constexpr int32_t DISABLE_RETRY_MAX_TIMES = 3;
    constexpr int32_t ENABLE_PARAM_RETRY_TIME = 500 * 1000;
    constexpr int32_t INVALID_SA_ID = -1;
    constexpr int32_t UNINIT_COMPONENT_TIMEOUT_SECONDS = 2;
    constexpr int32_t SYNC_DATA_TIMEOUT_MS = 1000 * 9;
    constexpr const char *MIC = "mic";
    constexpr const char *CAMERA = "camera";
}

ComponentManager::ComponentManager() : compSource_({}), compSink_({}), compSrcSaId_({}),
    compMonitorPtr_(std::make_shared<ComponentMonitor>()),
    lowLatencyListener_(sptr<LowLatencyListener>(new(std::nothrow) LowLatencyListener())),
    isUnInitTimeOut_(false), dhBizStates_({}), dhStateListener_(std::make_shared<DHStateListener>()),
    dhSinkStateListener_(std::make_shared<DHSinkStateListener>()),
    dataSyncTriggerListener_(std::make_shared<DHDataSyncTriggerListener>()),
    dhCommToolPtr_(std::make_shared<DHCommTool>()), needRefreshTaskParams_({}),
    workModeParam_(-1, 0, 0, false)
{
    DHLOGI("Ctor ComponentManager");
}

ComponentManager::~ComponentManager()
{
    DHLOGD("Dtor ComponentManager");
    compMonitorPtr_.reset();
    compMonitorPtr_ = nullptr;
    lowLatencyListener_ = nullptr;
}

int32_t ComponentManager::Init()
{
    DHLOGI("start.");
    DHTraceStart(COMPONENT_INIT_START);
#ifdef DHARDWARE_LOW_LATENCY
    Publisher::GetInstance().RegisterListener(DHTopic::TOPIC_LOW_LATENCY, lowLatencyListener_);
#endif
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<ComponentManager::ComponentManagerEventHandler>(runner);
    InitDHCommTool();
    DHLOGI("Init component success");
    DHTraceEnd();
    return DH_FWK_SUCCESS;
}

void ComponentManager::InitDHCommTool()
{
    if (dhCommToolPtr_ == nullptr) {
        DHLOGE("DH communication tool ptr is null");
        return;
    }
    DHLOGI("Init DH communication tool");
    dhCommToolPtr_->Init();
}

int32_t ComponentManager::UnInit()
{
    DHLOGI("start.");
    StopPrivacy();
    UnInitDHCommTool();
#ifdef DHARDWARE_LOW_LATENCY
    Publisher::GetInstance().UnregisterListener(DHTopic::TOPIC_LOW_LATENCY, lowLatencyListener_);
    LowLatency::GetInstance().CloseLowLatency();
#endif
    DHLOGI("Release component success");
    if (isUnInitTimeOut_.load()) {
        DHLOGE("Some component stop timeout, FORCE exit!");
        _Exit(0);
    }
    return DH_FWK_SUCCESS;
}

void ComponentManager::StopPrivacy()
{
    // stop privacy
    if (cameraCompPrivacy_ != nullptr && cameraCompPrivacy_->GetPageFlag()) {
        cameraCompPrivacy_->StopPrivacePage("camera");
        cameraCompPrivacy_->SetPageFlagFalse();
    }

    if (audioCompPrivacy_ != nullptr  && audioCompPrivacy_->GetPageFlag()) {
        audioCompPrivacy_->StopPrivacePage("mic");
        audioCompPrivacy_->SetPageFlagFalse();
    }
}

void ComponentManager::UnInitDHCommTool()
{
    if (dhCommToolPtr_ == nullptr) {
        DHLOGE("DH communication tool ptr is null");
        return;
    }
    DHLOGI("UnInit DH communication tool");
    dhCommToolPtr_->UnInit();
}

int32_t ComponentManager::StartSource(DHType dhType, ActionResult &sourceResult)
{
    DHLOGI("Start Source, dhType: %{public}" PRIu32, (uint32_t)dhType);
    std::unique_lock<std::shared_mutex> lock(compSourceMutex_);
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("Component for DHType: %{public}" PRIu32 " not init source handler", (uint32_t)dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    if (compSource_[dhType] == nullptr) {
        DHLOGE("comp source ptr is null");
        return ERR_DH_FWK_SA_HANDLER_IS_NULL;
    }
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    CompVersion compVersion;
    VersionManager::GetInstance().GetCompVersion(uuid, dhType, compVersion);
    auto params = compVersion.sourceVersion;
    std::promise<int32_t> p;
    std::future<int32_t> f = p.get_future();
    std::thread([p = std::move(p), this, dhType, params] () mutable {
        p.set_value(compSource_[dhType]->InitSource(params));
    }).detach();
    sourceResult.emplace(dhType, f.share());

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::StartSink(DHType dhType, ActionResult &sinkResult)
{
    DHLOGI("Start Sink, dhType: %{public}" PRIu32, (uint32_t)dhType);
    std::unique_lock<std::shared_mutex> lock(compSinkMutex_);
    std::unordered_map<DHType, std::shared_future<int32_t>> futures;
    if (compSink_.find(dhType) == compSink_.end()) {
        DHLOGE("Component for DHType: %{public}" PRIu32 " not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    if (compSink_[dhType] == nullptr) {
        DHLOGE("comp sink ptr is null");
        return ERR_DH_FWK_SA_HANDLER_IS_NULL;
    }
    std::string uuid = DHContext::GetInstance().GetDeviceInfo().uuid;
    CompVersion compVersion;
    VersionManager::GetInstance().GetCompVersion(uuid, dhType, compVersion);
    auto params = compVersion.sinkVersion;
    std::promise<int32_t> p;
    std::future<int32_t> f = p.get_future();
    std::thread([p = std::move(p), this, dhType, params] () mutable {
        p.set_value(compSink_[dhType]->InitSink(params));
    }).detach();
    sinkResult.emplace(dhType, f.share());
    if (cameraCompPrivacy_ == nullptr && dhType == DHType::CAMERA) {
        cameraCompPrivacy_ = std::make_shared<ComponentPrivacy>();
        compSink_[dhType]->RegisterPrivacyResources(cameraCompPrivacy_);
    }
    if (audioCompPrivacy_ == nullptr && dhType == DHType::AUDIO) {
        audioCompPrivacy_ = std::make_shared<ComponentPrivacy>();
        compSink_[dhType]->RegisterPrivacyResources(audioCompPrivacy_);
    }
    return DH_FWK_SUCCESS;
}

bool ComponentManager::WaitForResult(const Action &action, ActionResult actionsResult)
{
    DHLOGD("start.");
    auto ret = true;
    for (auto &iter : actionsResult) {
        std::future_status status = iter.second.wait_for(std::chrono::seconds(UNINIT_COMPONENT_TIMEOUT_SECONDS));
        if (status == std::future_status::ready) {
            auto result = iter.second.get();
            DHLOGI("action = %{public}d, compType = %{public}#X, READY, ret = %{public}d.",
                static_cast<int32_t>(action), iter.first, result);
            if (result != DH_FWK_SUCCESS) {
                ret = false;
                DHLOGE("there is error, but want to continue.");
            }
        }

        if (status == std::future_status::timeout) {
            DHLOGI("action = %{public}d, compType = %{public}#X, TIMEOUT", static_cast<int32_t>(action), iter.first);
            if (action == Action::STOP_SOURCE || action == Action::STOP_SINK) {
                isUnInitTimeOut_ = true;
            }
        }

        if (status == std::future_status::deferred) {
            DHLOGI("action = %{public}d, compType = %{public}#X, DEFERRED", static_cast<int32_t>(action), iter.first);
        }
    }
    DHLOGD("end.");
    return ret;
}

int32_t ComponentManager::Enable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType, bool isActive)
{
    DHLOGI("start.");
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::unique_lock<std::shared_mutex> lock(compSourceMutex_);
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %{public}s.", GetAnonyString(dhId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    EnableParam param;
    auto ret = GetEnableParam(networkId, uuid, dhId, dhType, param);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetEnableParam failed, uuid = %{public}s, dhId = %{public}s, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        if (RetryGetEnableParam(networkId, uuid, dhId, dhType, param) != DH_FWK_SUCCESS) {
            return ret;
        }
    }
    if (!isActive) {
        ret = CheckSubtypeResource(param.subtype, networkId);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("CheckSubtypeResource failed, ret = %{public}d.", ret);
            return ret;
        }
    }

    auto compEnable = std::make_shared<ComponentEnable>();
    auto result = compEnable->Enable(networkId, dhId, param, (compSource_.find(dhType))->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
            if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
                DHLOGE("device is already offline, no need try enable, uuid= %{public}s", GetAnonyString(uuid).c_str());
                return result;
            }
            if (compEnable->Enable(networkId, dhId, param, (compSource_.find(dhType))->second) == DH_FWK_SUCCESS) {
                DHLOGE("enable success, retryCount = %{public}d", retryCount);
                EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("enable failed, retryCount = %{public}d", retryCount);
        }
        return result;
    }
    DHLOGI("enable result is %{public}d, uuid = %{public}s, dhId = %{public}s", result, GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    EnabledCompsDump::GetInstance().DumpEnabledComp(networkId, dhType, dhId);

    return result;
}

int32_t ComponentManager::CheckSubtypeResource(const std::string &subtype, const std::string &networkId)
{
#ifdef DHARDWARE_CHECK_RESOURCE
    std::map<std::string, bool> resourceDesc = ComponentLoader::GetInstance().GetCompResourceDesc();
    if (resourceDesc.find(subtype) == resourceDesc.end()) {
        DHLOGE("GetCompResourceDesc failed, subtype: %{public}s", subtype.c_str());
        return ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY;
    }
    if (resourceDesc[subtype] && !IsIdenticalAccount(networkId)) {
        DHLOGE("Privacy resources must be logged in with the same account.");
        return ERR_DH_FWK_COMPONENT_ENABLE_FAILED;
    }
#endif
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::RetryGetEnableParam(const std::string &networkId, const std::string &uuid,
    const std::string &dhId, const DHType dhType, EnableParam &param)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    for (int32_t retryCount = 0; retryCount < ENABLE_RETRY_MAX_TIMES; retryCount++) {
        if (!DHContext::GetInstance().IsDeviceOnline(uuid)) {
            DHLOGE("device is already offline, no need try GetEnableParam, uuid = %{public}s",
                GetAnonyString(uuid).c_str());
            return ERR_DH_FWK_COMPONENT_ENABLE_FAILED;
        }
        if (GetEnableParam(networkId, uuid, dhId, dhType, param) == DH_FWK_SUCCESS) {
            DHLOGE("GetEnableParam success, retryCount = %{public}d", retryCount);
            break;
        }
        DHLOGE("GetEnableParam failed, retryCount = %{public}d", retryCount);
        usleep(ENABLE_PARAM_RETRY_TIME);
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::Disable(const std::string &networkId, const std::string &uuid, const std::string &dhId,
    const DHType dhType)
{
    DHLOGI("start.");
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::unique_lock<std::shared_mutex> lock(compSourceMutex_);
    auto find = compSource_.find(dhType);
    if (find == compSource_.end()) {
        DHLOGE("can not find handler for dhId = %{public}s.", GetAnonyString(dhId).c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }
    
    auto compDisable = std::make_shared<ComponentDisable>();
    auto result = compDisable->Disable(networkId, dhId, find->second);
    if (result != DH_FWK_SUCCESS) {
        for (int32_t retryCount = 0; retryCount < DISABLE_RETRY_MAX_TIMES; retryCount++) {
            if (compDisable->Disable(networkId, dhId, find->second) == DH_FWK_SUCCESS) {
                DHLOGE("disable success, retryCount = %{public}d", retryCount);
                EnabledCompsDump::GetInstance().DumpDisabledComp(networkId, dhType, dhId);
                return DH_FWK_SUCCESS;
            }
            DHLOGE("disable failed, retryCount = %{public}d", retryCount);
        }
        return result;
    }
    DHLOGI("disable result is %{public}d, uuid = %{public}s, dhId = %{public}s", result, GetAnonyString(uuid).c_str(),
        dhId.c_str());
    EnabledCompsDump::GetInstance().DumpDisabledComp(networkId, dhType, dhId);

    return result;
}

DHType ComponentManager::GetDHType(const std::string &uuid, const std::string &dhId) const
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(GetDeviceIdByUUID(uuid), dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        return capability->GetDHType();
    }
    DHLOGE("get dhType failed, uuid = %{public}s, dhId = %{public}s", GetAnonyString(uuid).c_str(),
        GetAnonyString(dhId).c_str());
    return DHType::UNKNOWN;
}

int32_t ComponentManager::GetEnableCapParam(const std::string &networkId, const std::string &uuid,
    DHType dhType, EnableParam &param, std::shared_ptr<CapabilityInfo> capability)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DeviceInfo sourceDeviceInfo = GetLocalDeviceInfo();
    std::vector<std::shared_ptr<CapabilityInfo>> sourceCapInfos;
    std::string sourceDHId;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(sourceDeviceInfo.deviceId, sourceCapInfos);
    for (const auto &capInfo : sourceCapInfos) {
        if (dhType == capInfo->GetDHType()) {
            param.sourceAttrs = capInfo->GetDHAttrs();
            sourceDHId = capInfo->GetDHId();
        }
    }
    std::string sourceVersion("");
    auto ret = GetVersion(sourceDeviceInfo.uuid, dhType, sourceVersion, false);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get source version failed.");
        return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
    }
    param.sourceVersion = sourceVersion;

    param.sinkAttrs = capability->GetDHAttrs();
    std::string sinkVersion("");
    ret = GetVersion(uuid, dhType, sinkVersion, true);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get sink version failed.");
        // If Version DB not sync, try get sink version from meta info
        std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
        ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(DHContext::GetInstance().GetUdidHashIdByUUID(uuid),
            capability->GetDHId(), metaCapPtr);
        if ((ret == DH_FWK_SUCCESS) && (metaCapPtr != nullptr)) {
            sinkVersion = metaCapPtr->GetSinkVersion();
        } else {
            return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
        }
    }
    param.sinkVersion = sinkVersion;
    param.subtype = capability->GetDHSubtype();
    DHLOGI("GetEnableCapParam success. dhType = %{public}#X, sink uuid =%{public}s,"
        "sinVersion = %{public}s, source uuid =%{public}s, source dhId = %{public}s, sourceVersion = %{public}s,"
        "subtype = %{public}s", dhType, GetAnonyString(uuid).c_str(),
        param.sinkVersion.c_str(), GetAnonyString(sourceDeviceInfo.uuid).c_str(), GetAnonyString(sourceDHId).c_str(),
        param.sourceVersion.c_str(), param.subtype.c_str());
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetEnableMetaParam(const std::string &networkId, const std::string &uuid,
    DHType dhType, EnableParam &param, std::shared_ptr<MetaCapabilityInfo> metaCapPtr)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DeviceInfo sourceDeviceInfo = GetLocalDeviceInfo();
    std::vector<std::shared_ptr<MetaCapabilityInfo>> sourceMetaInfos;
    std::string sourceDHId;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(sourceDeviceInfo.udidHash, sourceMetaInfos);
    for (const auto &metaInfo : sourceMetaInfos) {
        if (dhType == metaInfo->GetDHType()) {
            param.sourceAttrs = metaInfo->GetDHAttrs();
            sourceDHId = metaInfo->GetDHId();
        }
    }
    std::string sourceVersion("");
    auto ret = GetVersion(sourceDeviceInfo.uuid, dhType, sourceVersion, false);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get source version failed.");
        return ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED;
    }
    param.sourceVersion = sourceVersion;

    param.sinkAttrs = metaCapPtr->GetDHAttrs();
    param.sinkVersion = metaCapPtr->GetSinkVersion();
    param.subtype = metaCapPtr->GetDHSubtype();
    DHLOGI("GetEnableCapParam success. dhType = %{public}#X, sink uuid =%{public}s,"
        "sinVersion = %{public}s, source uuid =%{public}s, source dhId = %{public}s, sourceVersion = %{public}s,"
        "subtype = %{public}s", dhType, GetAnonyString(uuid).c_str(),
        param.sinkVersion.c_str(), GetAnonyString(sourceDeviceInfo.uuid).c_str(), GetAnonyString(sourceDHId).c_str(),
        param.sourceVersion.c_str(), param.subtype.c_str());
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetCapParam(const std::string &uuid, const std::string &dhId,
    std::shared_ptr<CapabilityInfo> &capability)
{
    if (!IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    std::string deviceId = GetDeviceIdByUUID(uuid);
    auto ret = CapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        DHLOGI("GetCapability success, deviceId: %{public}s, uuid: %{public}s, dhId: %{public}s, ret: %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        return ret;
    }

    ret = LocalCapabilityInfoManager::GetInstance()->GetCapability(deviceId, dhId, capability);
    if ((ret == DH_FWK_SUCCESS) && (capability != nullptr)) {
        DHLOGI("Local GetCaps success, deviceId: %{public}s, uuid: %{public}s, dhId: %{public}s, ret: %{public}d",
            GetAnonyString(deviceId).c_str(), GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        return ret;
    }

    return ret;
}

int32_t ComponentManager::GetMetaParam(const std::string &uuid, const std::string &dhId,
    std::shared_ptr<MetaCapabilityInfo> &metaCapPtr)
{
    if (!IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    auto ret = MetaInfoManager::GetInstance()->GetMetaCapInfo(DHContext::GetInstance().GetUdidHashIdByUUID(uuid),
        dhId, metaCapPtr);
    if ((ret == DH_FWK_SUCCESS) && (metaCapPtr != nullptr)) {
        DHLOGI("GetCapability success, uuid =%{public}s, dhId = %{public}s, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), ret);
        return ret;
    }
    return ret;
}

int32_t ComponentManager::GetEnableParam(const std::string &networkId, const std::string &uuid,
    const std::string &dhId, DHType dhType, EnableParam &param)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED;
    }
    DHLOGI("GetEnableParam start, networkId= %{public}s, uuid = %{public}s, dhId = %{public}s, dhType = %{public}#X,",
        GetAnonyString(networkId).c_str(), GetAnonyString(uuid).c_str(), GetAnonyString(dhId).c_str(), dhType);
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    if (GetCapParam(uuid, dhId, capability) == DH_FWK_SUCCESS) {
        auto ret = GetEnableCapParam(networkId, uuid, dhType, param, capability);
        if (ret == DH_FWK_SUCCESS) {
            return ret;
        }
        DHLOGE("GetEnableCapParam failed.");
    }

    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    if (GetMetaParam(uuid, dhId, metaCapPtr) == DH_FWK_SUCCESS) {
        auto ret = GetEnableMetaParam(networkId, uuid, dhType, param, metaCapPtr);
        if (ret == DH_FWK_SUCCESS) {
            return ret;
        }
        DHLOGE("GetEnableMetaParam failed.");
    }
    DHLOGE("GetEnableParam is failed.");
    return ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED;
}

int32_t ComponentManager::GetVersionFromVerMgr(const std::string &uuid, const DHType dhType,
    std::string &version, bool isSink)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    CompVersion compversion;
    int32_t ret = VersionManager::GetInstance().GetCompVersion(uuid, dhType, compversion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get version Manager failed, uuid =%{public}s, dhType = %{public}#X, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), dhType, ret);
        return ret;
    }
    DHLOGI("Get version mgr success, sinkVersion = %{public}s, sourceVersion = %{public}s,uuid = %{public}s, "
        "dhType = %{public}#X", compversion.sinkVersion.c_str(), compversion.sourceVersion.c_str(),
        GetAnonyString(uuid).c_str(), dhType);
    version = isSink ? compversion.sinkVersion : compversion.sourceVersion;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetVersionFromVerInfoMgr(const std::string &uuid, const DHType dhType,
    std::string &version, bool isSink)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    VersionInfo versionInfo;
    int32_t ret =  VersionInfoManager::GetInstance()->GetVersionInfoByDeviceId(GetDeviceIdByUUID(uuid), versionInfo);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get Version info Manager failed, uuid =%{public}s, dhType = %{public}#X, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), dhType, ret);
        return ret;
    }
    auto iter = versionInfo.compVersions.find(dhType);
    if (iter == versionInfo.compVersions.end()) {
        DHLOGE("can not find component version for dhType = %{public}d", dhType);
        return ERR_DH_FWK_COMPONENT_DHTYPE_NOT_FOUND;
    }
    DHLOGI("Get version info mgr success, sinkVersion = %{public}s, sourceVersion = %{public}s, uuid = %{public}s, "
        "dhType = %{public}#X", iter->second.sinkVersion.c_str(), iter->second.sourceVersion.c_str(),
        GetAnonyString(uuid).c_str(), dhType);
    UpdateVersionCache(uuid, versionInfo);
    version = isSink ? iter->second.sinkVersion : iter->second.sourceVersion;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetVersion(const std::string &uuid, DHType dhType, std::string &version, bool isSink)
{
    if (!IsIdLengthValid(uuid)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = GetVersionFromVerMgr(uuid, dhType, version, isSink);
    if ((ret == DH_FWK_SUCCESS) && (!version.empty())) {
        return DH_FWK_SUCCESS;
    }

    ret = GetVersionFromVerInfoMgr(uuid, dhType, version, isSink);
    if ((ret == DH_FWK_SUCCESS) && (!version.empty())) {
        return DH_FWK_SUCCESS;
    }

    return ret;
}

void ComponentManager::UpdateVersionCache(const std::string &uuid, const VersionInfo &versionInfo)
{
    if (!IsIdLengthValid(uuid)) {
        return;
    }
    DHVersion dhVersion;
    dhVersion.uuid = uuid;
    dhVersion.dhVersion = versionInfo.dhVersion;
    dhVersion.compVersions = versionInfo.compVersions;
    VersionManager::GetInstance().AddDHVersion(uuid, dhVersion);
}

void ComponentManager::DumpLoadedCompsource(std::set<DHType> &compSourceType)
{
    std::unique_lock<std::shared_mutex> lock(compSourceMutex_);
    for (auto compSource : compSource_) {
        compSourceType.emplace(compSource.first);
    }
}

void ComponentManager::DumpLoadedCompsink(std::set<DHType> &compSinkType)
{
    std::unique_lock<std::shared_mutex> lock(compSinkMutex_);
    for (auto compSink : compSink_) {
        compSinkType.emplace(compSink.first);
    }
}

void ComponentManager::Recover(DHType dhType)
{
    ffrt::submit([this, dhType]() { this->DoRecover(dhType); });
}

void ComponentManager::DoRecover(DHType dhType)
{
    int32_t ret = pthread_setname_np(pthread_self(), DO_RECOVER);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DoRecover setname failed.");
    }
    // reset enable status
    DHLOGI("Reset enable status for DHType %{public}" PRIu32, (uint32_t)dhType);
    ResetSinkEnableStatus(dhType);
    ResetSourceEnableStatus(dhType);
    // recover distributed hardware virtual driver
    DHLOGI("Recover distributed hardware virtual driver for DHType %{public}" PRIu32, (uint32_t)dhType);
    RecoverAutoEnableSink(dhType);
    RecoverAutoEnableSource(dhType);
    RecoverActiveEnableSink(dhType);
    RecoverActiveEnableSource(dhType);
}

std::map<DHType, IDistributedHardwareSink*> ComponentManager::GetDHSinkInstance()
{
    std::shared_lock<std::shared_mutex> lock(compSinkMutex_);
    return compSink_;
}

bool ComponentManager::IsIdenticalAccount(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return false;
    }
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DH_FWK_PKG_NAME, "", deviceList);
    if (deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE) {
        DHLOGE("DeviceList size is invalid!");
        return false;
    }
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            authForm = deviceInfo.authForm;
            break;
        }
    }
    if (authForm == DmAuthForm::IDENTICAL_ACCOUNT) {
        return true;
    }
    return false;
}

int32_t ComponentManager::InitAVSyncSharedMemory()
{
    uint32_t memLen = sizeof(SyncShareData);
    std::string memName = "AVSyncSharedMemory";
    syncSharedMem_ = OHOS::Ashmem::CreateAshmem(memName.c_str(), memLen);
    if (syncSharedMem_ == nullptr) {
        DHLOGE("create ashmem failed");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    if (!syncSharedMem_->MapReadAndWriteAshmem()) {
        DHLOGE("mmap ashmem failed");
        return ERR_DH_FWK_PARA_INVALID;
    }
    {
        std::lock_guard<std::mutex> lock(workModeParamMtx_);
        workModeParam_.isAVsync = true;
        workModeParam_.sharedMemLen = sizeof(SyncShareData);
        workModeParam_.fd = syncSharedMem_->GetAshmemFd();
    }

    syncShareData_ = std::make_shared<SyncShareData>();
    syncShareData_->lock = 1;
    syncShareData_->audio_current_pts = 0;
    syncShareData_->audio_update_clock= 0;
    syncShareData_->audio_speed = 1.0;
    syncShareData_->video_current_pts= 0;
    syncShareData_->video_update_clock= 0;
    syncShareData_->video_speed = 1.0;
    syncShareData_->sync_strategy= 1;
    syncShareData_->reset = false;

    bool ret = syncSharedMem_->WriteToAshmem(static_cast<void *>(syncShareData_.get()),
        sizeof(SyncShareData), 0);
    if (!ret) {
        DHLOGE("init sync info failed");
        return ERR_DH_FWK_BAD_OPERATION;
    }
    return DH_FWK_SUCCESS;
}

void ComponentManager::DeinitAVSyncSharedMemory()
{
    if (syncSharedMem_ != nullptr) {
        syncSharedMem_->UnmapAshmem();
        syncSharedMem_->CloseAshmem();
        syncSharedMem_ = nullptr;
    }
}

int32_t ComponentManager::GetDHIdByDHSubtype(const DHSubtype dhSubtype, std::string &networkId, std::string &dhId)
{
    std::lock_guard<std::mutex> lock(bizStateMtx_);
    for (const auto &iter : dhBizStates_) {
        const auto devInfo = iter.first;
        DHSubtype subType = DHSubtype::UNKNOWN;
        GetDHSubtypeByDHId(subType, devInfo.first, devInfo.second);
        if (subType == dhSubtype) {
            networkId = devInfo.first;
            dhId = devInfo.second;
            return DH_FWK_SUCCESS;
        }
    }
    DHLOGE("unable to obtain dhId that matches dhSubtype.");
    return ERR_DH_FWK_BAD_OPERATION;
}

int32_t ComponentManager::GetDHSubtypeByDHId(DHSubtype &dhSubtype, const std::string &networkId,
    const std::string &dhId)
{
    std::string deviceId = "";
    std::string dhSubtypeStr = "";
    if (networkId == GetLocalNetworkId()) {
        deviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
        if (deviceId.empty()) {
            DHLOGE("Get local deviceId failed.");
            return ERR_DH_FWK_BAD_OPERATION;
        }
        dhSubtypeStr = CapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
        if (dhSubtypeStr.empty()) {
            DHLOGE("Get local dhSubtype failed.");
            return ERR_DH_FWK_BAD_OPERATION;
        }
    } else {
        deviceId = DHContext::GetInstance().GetDeviceIdByNetworkId(networkId);
        dhSubtypeStr = LocalCapabilityInfoManager::GetInstance()->GetDhSubtype(deviceId, dhId);
        if (dhSubtypeStr.empty()) {
            DHLOGW("Can not get remote dhSubtype from LocalCapability, try use meta info.");
            std::string udid = DHContext::GetInstance().GetUDIDByNetworkId(networkId);
            std::string udidHash = Sha256(udid);
            dhSubtypeStr = MetaInfoManager::GetInstance()->GetDhSubtypeByUdidHash(udidHash, dhId);
            if (dhSubtypeStr.empty()) {
                DHLOGE("Get remote dhSubtype failed.");
                return ERR_DH_FWK_BAD_OPERATION;
            }
        }
    }

    if (dhSubtypeStr == MIC) {
        dhSubtype = DHSubtype::AUDIO_MIC;
    } else if (dhSubtypeStr == CAMERA) {
        dhSubtype = DHSubtype::CAMERA;
    } else {
        DHLOGE("unable to obtain dhSubtype that matches dhId.");
        return ERR_DH_FWK_BAD_OPERATION;
    }
    return DH_FWK_SUCCESS;
}

void ComponentManager::HandleIdleStateChange(const std::string &networkId, const std::string &dhId, const DHType dhType)
{
    if (dhId.empty() || networkId.empty()) {
        DHLOGE("targetDHId or targetNetworkId is empty");
        DeinitAVSyncSharedMemory();
        return;
    }
    auto targetHandler = GetDHSourceInstance(dhType);
    if (targetHandler == nullptr) {
        DHLOGE("targetHandler is nullptr");
        DeinitAVSyncSharedMemory();
        return;
    }
    {
        std::lock_guard<std::mutex> lock(workModeParamMtx_);
        workModeParam_.isAVsync = false;
    }
    targetHandler->UpdateDistributedHardwareWorkMode(networkId, dhId, workModeParam_);
    DeinitAVSyncSharedMemory();
    DHLOGI("handle idle state change success.");
}

void ComponentManager::HandleBusinessStateChange(const std::string &networkId, const std::string &dhId,
    const DHSubtype dhSubType, const BusinessState state)
{
    DHType sourceType = (dhSubType == DHSubtype::AUDIO_MIC) ? DHType::AUDIO : DHType::CAMERA;
    DHType targetType = (dhSubType == DHSubtype::AUDIO_MIC) ? DHType::CAMERA : DHType::AUDIO;
    DHSubtype targetSubType = (dhSubType == DHSubtype::AUDIO_MIC) ? DHSubtype::CAMERA : DHSubtype::AUDIO_MIC;
    std::string targetDHId = "";
    std::string targetNetworkId = "";
    GetDHIdByDHSubtype(targetSubType, targetNetworkId, targetDHId);
    DHLOGI("targetNetworkId: %{public}s, targetDHId: %{public}s, targetSubType: %{public}" PRIu32,
        GetAnonyString(targetNetworkId).c_str(), GetAnonyString(targetDHId).c_str(), (uint32_t)targetSubType);
    if (state == BusinessState::IDLE) {
        HandleIdleStateChange(targetNetworkId, targetDHId, targetType);
    } else if (state == BusinessState::RUNNING) {
        if (targetDHId.empty() || targetNetworkId.empty()) {
            DHLOGE("target dhardware is not online");
            return;
        }
        BusinessState targetState = QueryBusinessState(targetNetworkId, targetDHId);
        if (targetState != BusinessState::RUNNING) {
            DHLOGE("target dhardware is not ready");
            return;
        }
        auto targetHandler = GetDHSourceInstance(targetType);
        auto sourceHandler = GetDHSourceInstance(sourceType);
        if (targetHandler == nullptr || sourceHandler == nullptr) {
            DHLOGE("can not find cameraHandler or audioHandler");
            return;
        }
        
        int32_t ret = InitAVSyncSharedMemory();
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("InitAVSyncSharedMemory failed");
            DeinitAVSyncSharedMemory();
            return;
        }
        ret = targetHandler->UpdateDistributedHardwareWorkMode(targetNetworkId, targetDHId, workModeParam_);
        if (ret != DH_FWK_SUCCESS) {
            DeinitAVSyncSharedMemory();
            return;
        }
        ret = sourceHandler->UpdateDistributedHardwareWorkMode(networkId, dhId, workModeParam_);
        if (ret != DH_FWK_SUCCESS) {
            DeinitAVSyncSharedMemory();
            HandleIdleStateChange(targetNetworkId, targetDHId, targetType);
            return;
        }
    }
}

void ComponentManager::NotifyBusinessStateChange(const DHSubtype dhSubType, const BusinessState state)
{
    DHLOGI("NotifyBusinessStateChange, dhSubType:%{public}" PRIu32", state:%{public}" PRIu32,
        static_cast<uint32_t>(dhSubType), static_cast<uint32_t>(state));
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        DHLOGE("cJSON_CreateObject failed");
        return;
    }
    if (cJSON_AddNumberToObject(root, DH_SUBTYPE, static_cast<uint32_t>(dhSubType)) == nullptr) {
        DHLOGE("failed to add dhSubType to json");
        cJSON_Delete(root);
        return;
    }
    if (cJSON_AddNumberToObject(root, BUSINESS_STATE, static_cast<uint32_t>(state)) == nullptr) {
        DHLOGE("failed to add state to json");
        cJSON_Delete(root);
        return;
    }
    char *json = cJSON_Print(root);
    if (json == nullptr) {
        DHLOGE("failed to print json");
        cJSON_Delete(root);
        return;
    }
    std::lock_guard<std::mutex> lock(dhTopicMtx_);
    Publisher::GetInstance().PublishMessage(dhTopic_, json);
    cJSON_Delete(root);
    free(json);
    return;
}

void ComponentManager::UpdateSinkBusinessState(const std::string &networkId, const std::string &dhId,
    BusinessSinkState state)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(dhId)) {
        return;
    }
    DHLOGI("UpdateSinkBusinessState, networkId: %{public}s, dhId: %{public}s, state: %{public}" PRIu32,
        GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str(), (uint32_t)state);
    DHSubtype dhSubtype;
    GetDHSubtypeByDHId(dhSubtype, networkId, dhId);
    DHLOGI("UpdateSinkBusinessState::get dhSubtype:%{public}" PRIu32, static_cast<uint32_t>(dhSubtype));
    if (state == BusinessSinkState::IDLE) {
        NotifyBusinessStateChange(dhSubtype, BusinessState::IDLE);
    } else if (state == BusinessSinkState::RUNNING) {
        NotifyBusinessStateChange(dhSubtype, BusinessState::RUNNING);
    } else {
        DHLOGI("unsupported sink business state");
    }
}

void ComponentManager::SetAVSyncScene(const DHTopic topic)
{
    DHLOGI("set AVSync scene : %{public}" PRIu32, topic);
    std::lock_guard<std::mutex> lock(dhTopicMtx_);
    switch (topic) {
        case DHTopic::TOPIC_AV_LOW_LATENCY: {
            dhTopic_ = topic;
            std::lock_guard<std::mutex> lock(workModeParamMtx_);
            workModeParam_.scene = static_cast<uint32_t>(AVscene::VIDEOCALL);
            break;
        }
        case DHTopic::TOPIC_AV_FLUENCY: {
            dhTopic_ = topic;
            std::lock_guard<std::mutex> lock(workModeParamMtx_);
            workModeParam_.scene = static_cast<uint32_t>(AVscene::BROADCAST);
            break;
        }
        default: {
            DHLOGW("not AVSync related topic");
        }
    }
}

void ComponentManager::UpdateBusinessState(const std::string &networkId, const std::string &dhId, BusinessState state)
{
    if (!IsIdLengthValid(networkId) || !IsIdLengthValid(dhId)) {
        return;
    }
    DHLOGI("UpdateBusinessState, networkId: %{public}s, dhId: %{public}s, state: %{public}" PRIu32,
        GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str(), (uint32_t)state);
    {
        std::lock_guard<std::mutex> lock(bizStateMtx_);
        dhBizStates_[{networkId, dhId}] = state;
    }
    DHSubtype dhSubtype;
    GetDHSubtypeByDHId(dhSubtype, networkId, dhId);
    DHLOGI("UpdateBusinessState::get dhSubtype:%{public}" PRIu32, static_cast<uint32_t>(dhSubtype));
    if (dhSubtype == DHSubtype::CAMERA || dhSubtype == DHSubtype::AUDIO_MIC) {
        NotifyBusinessStateChange(dhSubtype, state);
        HandleBusinessStateChange(networkId, dhId, dhSubtype, state);
    }
    if (state == BusinessState::IDLE) {
        TaskParam taskParam;
        if (!FetchNeedRefreshTask({networkId, dhId}, taskParam)) {
            return;
        }
        DHLOGI("The dh need refresh, networkId: %{public}s, dhId: %{public}s",
            GetAnonyString(networkId).c_str(), GetAnonyString(dhId).c_str());
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
}

IDistributedHardwareSource* ComponentManager::GetDHSourceInstance(DHType dhType)
{
    std::unique_lock<std::shared_mutex> lock(compSourceMutex_);
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("can not find handler for dhType = %{public}d.", dhType);
        return nullptr;
    }
    return compSource_[dhType];
}

BusinessState ComponentManager::QueryBusinessState(const std::string &uuid, const std::string &dhId)
{
    if (!IsIdLengthValid(uuid) || !IsIdLengthValid(dhId)) {
        return BusinessState::UNKNOWN;
    }
    std::lock_guard<std::mutex> lock(bizStateMtx_);
    std::pair<std::string, std::string> key = {uuid, dhId};
    if (dhBizStates_.find(key) == dhBizStates_.end()) {
        return BusinessState::UNKNOWN;
    }

    return dhBizStates_.at(key);
}

void ComponentManager::TriggerFullCapsSync(const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return;
    }
    if (dhCommToolPtr_ == nullptr) {
        DHLOGE("DH communication tool ptr is null");
        return;
    }
    dhCommToolPtr_->TriggerReqFullDHCaps(networkId);
}

void ComponentManager::SaveNeedRefreshTask(const TaskParam &taskParam)
{
    std::lock_guard<std::mutex> lock(needRefreshTaskParamsMtx_);
    needRefreshTaskParams_[{taskParam.networkId, taskParam.dhId}] = taskParam;
}

bool ComponentManager::FetchNeedRefreshTask(const std::pair<std::string, std::string> &taskKey, TaskParam &taskParam)
{
    std::lock_guard<std::mutex> lock(needRefreshTaskParamsMtx_);
    if (needRefreshTaskParams_.find(taskKey) == needRefreshTaskParams_.end()) {
        return false;
    }

    taskParam = needRefreshTaskParams_.at(taskKey);
    needRefreshTaskParams_.erase(taskKey);
    return true;
}

ComponentManager::ComponentManagerEventHandler::ComponentManagerEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> runner) : AppExecFwk::EventHandler(runner)
{
    DHLOGI("Ctor ComponentManagerEventHandler");
}

void ComponentManager::ComponentManagerEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        DHLOGE("event is nullptr");
        return;
    }
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case EVENT_DATA_SYNC_MANUAL: {
            // do muanul sync with remote
            auto sharedObjPtr = event->GetSharedObject<std::string>();
            if (sharedObjPtr == nullptr) {
                DHLOGE("The data sync param invalid!");
                break;
            }
            std::string networkId = *sharedObjPtr;
            DHLOGI("Try receive full capabiliy info from networkId: %{public}s", GetAnonyString(networkId).c_str());
            if (networkId.empty()) {
                DHLOGE("Can not get device uuid by networkId: %{public}s", GetAnonyString(networkId).c_str());
                break;
            }
            ComponentManager::GetInstance().TriggerFullCapsSync(networkId);
            break;
        }
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

std::shared_ptr<ComponentManager::ComponentManagerEventHandler> ComponentManager::GetEventHandler()
{
    return this->eventHandler_;
}

int32_t ComponentManager::CheckSinkConfigStart(const DHType dhType, bool &enableSink)
{
    DHLOGI("CheckSinkConfigStart the dhType: %{public}#X configuration start.", dhType);
    DHVersion localDhVersion;
    auto ret = ComponentLoader::GetInstance().GetLocalDHVersion(localDhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalDHVersion fail.");
        return ret;
    }
    auto iterLocal = localDhVersion.compVersions.find(dhType);
    if (iterLocal == localDhVersion.compVersions.end()) {
        DHLOGE("Not find dhType: %{public}#X in local!", dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    if (!iterLocal->second.haveFeature) {
        enableSink = true;
        DHLOGI("Local DhType: %{public}#X is old configuration, need enable sink.", dhType);
        return DH_FWK_SUCCESS;
    }

    if (iterLocal->second.sinkSupportedFeatures.size() > 0) {
        enableSink = true;
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::CheckDemandStart(const std::string &uuid, const DHType dhType, bool &enableSource)
{
    DHLOGI("CheckDemandStart the dhType: %{public}#X configuration start.", dhType);
    enableSource = false;

    CompVersion compVersion;
    auto ret = GetRemoteVerInfo(compVersion, uuid, dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetRemoteVerInfo fail.");
        return ret;
    }

    DHVersion dhVersion;
    ret = ComponentLoader::GetInstance().GetLocalDHVersion(dhVersion);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetLocalDHVersion fail.");
        return ret;
    }

    auto iterLocal = dhVersion.compVersions.find(dhType);
    if (iterLocal == dhVersion.compVersions.end()) {
        DHLOGE("Not find dhType in local: %{public}#X!", dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    if (!iterLocal->second.haveFeature) {
        DHLOGI("Local dhType: %{public}#X is old configuration, need enable source", dhType);
        enableSource = true;
        return DH_FWK_SUCCESS;
    }
    if (iterLocal->second.sourceFeatureFilters.size() == 0) {
        return DH_FWK_SUCCESS;
    }

    enableSource = IsFeatureMatched(iterLocal->second.sourceFeatureFilters, compVersion.sinkSupportedFeatures);
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::RegisterDHStatusListener(
    sptr<IHDSinkStatusListener> listener, int32_t callingUid, int32_t callingPid)
{
    std::lock_guard<std::mutex> lock(dhSinkStatusMtx_);
    std::vector<DHType> compTypes;
    ComponentLoader::GetInstance().GetAllCompTypes(compTypes);
    for (const auto &type : compTypes) {
        auto &status = dhSinkStatus_[type];
        DHStatusCtrlKey ctrlKey {
            .uid = callingUid,
            .pid = callingPid
        };
        auto &listeners = status.listeners;
        if (listeners.find(ctrlKey) != listeners.end()) {
            DHLOGE("Repeat call RegisterDHStatusListener, uid = %{public}d, pid = %{public}d.",
                ctrlKey.uid, ctrlKey.pid);
            return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
        }
        listeners[ctrlKey] = listener;
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::UnregisterDHStatusListener(
    sptr<IHDSinkStatusListener> listener, int32_t callingUid, int32_t callingPid)
{
    std::lock_guard<std::mutex> lock(dhSinkStatusMtx_);
    std::vector<DHType> compTypes;
    ComponentLoader::GetInstance().GetAllCompTypes(compTypes);
    for (const auto &type : compTypes) {
        auto &status = dhSinkStatus_[type];
        DHStatusCtrlKey ctrlKey {
            .uid = callingUid,
            .pid = callingPid
        };
        auto &listeners = status.listeners;
        auto it = listeners.find(ctrlKey);
        if (it == listeners.end()) {
            DHLOGE("Repeat call UnregisterDHStatusListener, uid = %{public}d, pid = %{public}d.",
                ctrlKey.uid, ctrlKey.pid);
            return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
        }
        listeners.erase(it);
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::RegisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener, int32_t callingUid, int32_t callingPid)
{
    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);
    std::vector<DHType> compTypes;
    ComponentLoader::GetInstance().GetAllCompTypes(compTypes);
    for (const auto &type : compTypes) {
        auto &status = dhSourceStatus_[type];
        DHStatusCtrlKey ctrlKey {
            .uid = callingUid,
            .pid = callingPid
        };
        auto &listeners = status.listeners;
        if (listeners.find(ctrlKey) != listeners.end()) {
            DHLOGE("Repeat call RegisterDHStatusListener, uid = %{public}d, pid = %{public}d.",
                ctrlKey.uid, ctrlKey.pid);
            return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
        }
        listeners[ctrlKey] = listener;
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::UnregisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener, int32_t callingUid, int32_t callingPid)
{
    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);
    std::vector<DHType> compTypes;
    ComponentLoader::GetInstance().GetAllCompTypes(compTypes);
    for (const auto &type : compTypes) {
        auto &status = dhSourceStatus_[type];
        DHStatusCtrlKey ctrlKey {
            .uid = callingUid,
            .pid = callingPid
        };
        auto &listeners = status.listeners;
        auto it = listeners.find(ctrlKey);
        if (it == listeners.end()) {
            DHLOGE("Repeat call UnregisterDHStatusListener, uid = %{public}d, pid = %{public}d.",
                ctrlKey.uid, ctrlKey.pid);
            return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
        }
        listeners.erase(it);
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::EnableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    DHLOGI("Start enablesink, dhType: %{public}#X, dhid: %{public}s", dhDescriptor.dhType,
        GetAnonyString(dhDescriptor.id).c_str());
    sptr<IHDSinkStatusListener> listener;
    int32_t ret = EnableSinkInternal(dhDescriptor, callingUid, callingPid, listener);
    if (ret == DH_FWK_SUCCESS) {
        if (listener) {
            listener->OnEnable(dhDescriptor);
            DHLOGI("Callback business sink OnEnable.");
        }
    }
    return ret;
}

int32_t ComponentManager::DisableSink(const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    DHLOGI("Start disableSink, dhType: %{public}#X, dhid: %{public}s", dhDescriptor.dhType,
        GetAnonyString(dhDescriptor.id).c_str());
    sptr<IHDSinkStatusListener> listener;
    int32_t ret = DisableSinkInternal(dhDescriptor, callingUid, callingPid, listener);
    if (ret == DH_FWK_SUCCESS) {
        if (listener) {
            listener->OnDisable(dhDescriptor);
            DHLOGI("Callback business sink OnDisable.");
        }
    }
    return ret;
}

int32_t ComponentManager::EnableSource(const std::string &networkId,
    const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    DHLOGI("Start enablesource dhType: %{public}#X, dhid: %{public}s", dhDescriptor.dhType,
        GetAnonyString(dhDescriptor.id).c_str());
    sptr<IHDSourceStatusListener> listener;
    int32_t ret = EnableSourceInternal(networkId, dhDescriptor, callingUid, callingPid, listener);
    if (ret == DH_FWK_SUCCESS) {
        if (listener) {
            listener->OnEnable(networkId, dhDescriptor);
            DHLOGI("Callback business source OnEnable.");
        }
    }
    return ret;
}

int32_t ComponentManager::DisableSource(const std::string &networkId,
    const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid)
{
    DHLOGI("Start disableSource, dhType: %{public}#X, dhid: %{public}s", dhDescriptor.dhType,
        GetAnonyString(dhDescriptor.id).c_str());
    sptr<IHDSourceStatusListener> listener;
    int32_t ret = DisableSourceInternal(networkId, dhDescriptor, callingUid, callingPid, listener);
    if (ret == DH_FWK_SUCCESS) {
        if (listener) {
            listener->OnDisable(networkId, dhDescriptor);
            DHLOGI("Callback business source OnDisable.");
        }
    }
    return ret;
}

int32_t ComponentManager::ForceDisableSink(const DHDescriptor &dhDescriptor)
{
    std::vector<sptr<IHDSinkStatusListener>> listeners;
    int32_t ret = ForceDisableSinkInternal(dhDescriptor, listeners);
    if (ret == DH_FWK_SUCCESS) {
        for (auto listener : listeners) {
            listener->OnDisable(dhDescriptor);
            DHLOGI("Callback business sink OnDisable.");
        }
    }
    return ret;
}

int32_t ComponentManager::ForceDisableSource(const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    if (!ComponentLoader::GetInstance().IsDHTypeSupport(dhDescriptor.dhType)) {
        DHLOGE("Not support dhType: %{public}#X!", dhDescriptor.dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    std::vector<sptr<IHDSourceStatusListener>> listeners;
    int32_t ret = ForceDisableSourceInternal(networkId, dhDescriptor, listeners);
    if (ret == DH_FWK_SUCCESS) {
        for (auto listener : listeners) {
            listener->OnDisable(networkId, dhDescriptor);
            DHLOGI("Callback business source OnDisable.");
        }
    }
    return ret;
}

int32_t ComponentManager::CheckIdenticalAccount(const std::string &networkId,
    const std::string &uuid, const DHDescriptor &dhDescriptor)
{
    EnableParam param;
    auto ret = GetEnableParam(networkId, uuid, dhDescriptor.id, dhDescriptor.dhType, param);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetEnableParam failed, uuid = %{public}s, dhId = %{public}s, errCode = %{public}d",
            GetAnonyString(uuid).c_str(), GetAnonyString(dhDescriptor.id).c_str(), ret);
        if (ComponentManager::GetInstance().RetryGetEnableParam(
            networkId, uuid, dhDescriptor.id, dhDescriptor.dhType, param) != DH_FWK_SUCCESS) {
            return ret;
        }
    }
    ret = CheckSubtypeResource(param.subtype, networkId);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("CheckSubtypeResource failed, ret = %{public}d.", ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::GetRemoteVerInfo(CompVersion &compVersion, const std::string &uuid, DHType dhType)
{
    MetaCapInfoMap metaInfoMap;
    MetaInfoManager::GetInstance()->GetMetaDataByDHType(dhType, metaInfoMap);
    for (const auto &metaInfo : metaInfoMap) {
        if (DHContext::GetInstance().GetUUIDByDeviceId(metaInfo.second->GetDeviceId()) == uuid) {
            compVersion = metaInfo.second->GetCompVersion();
            return DH_FWK_SUCCESS;
        }
    }
    DHLOGE("The metaInfo corresponding to uuid was not found, uuid =%{public}s, dhType = %{public}#X.",
        GetAnonyString(uuid).c_str(), dhType);
    return ERR_DH_FWK_COMPONENT_COMPVERSION_NOT_FOUND;
}

bool ComponentManager::IsFeatureMatched(const std::vector<std::string> &sourceFeatureFilters,
    const std::vector<std::string> &sinkSupportedFeatures)
{
    for (const auto &filter : sourceFeatureFilters) {
        for (const auto &feature : sinkSupportedFeatures) {
            if (feature == filter) {
                return true;
            }
        }
    }
    DHLOGE("The sourcefeature and the sinkfeature do not match.");
    return false;
}

int32_t ComponentManager::EnableSinkInternal(const DHDescriptor &dhDescriptor,
    int32_t callingUid, int32_t callingPid, sptr<IHDSinkStatusListener> &listener)
{
    DHLOGI("Start EnableSinkInternal, dhType: %{public}#X", dhDescriptor.dhType);
    std::lock_guard<std::mutex> lock(dhSinkStatusMtx_);

    // Check if the input parameters and device type support it
    if (!ComponentLoader::GetInstance().IsDHTypeSupport(dhDescriptor.dhType)) {
        DHLOGE("Not support dhType: %{public}#X!", dhDescriptor.dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    auto &status = dhSinkStatus_[dhDescriptor.dhType];
    auto &enableInfo = status.enableInfos[dhDescriptor.id];

    // Check if the business is being called repeatedly
    DHStatusCtrlKey ctrlKey { .uid = callingUid, .pid = callingPid };

    // Get business enable status listener
    auto itrListener = status.listeners.find(ctrlKey);
    if (itrListener != status.listeners.end()) {
        listener = itrListener->second;
    }

    auto &statusCtrl = enableInfo.dhStatusCtrl[ctrlKey];
    if (statusCtrl.enableState == EnableState::ENABLED) {
        DHLOGI("Repeat call EnableSink, uid = %{public}d, pid = %{public}d.", ctrlKey.uid, ctrlKey.pid);
        return DH_FWK_SUCCESS;
    }

    // Check reference count
    if (enableInfo.refEnable || status.refLoad) {
        // Change status, we won't call back directly here because there is a lock
        DHLOGI("Add reference count, dhType: %{public}#X", dhDescriptor.dhType);
        statusCtrl.enableState = EnableState::ENABLED;
        enableInfo.refEnable++;
        status.refLoad++;
        return DH_FWK_SUCCESS;
    }

    // Start enabling hardware sink
    auto ret = InitCompSink(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("InitCompSink failed, ret = %{public}d.", ret);
        return ret;
    }
    std::unordered_map<DHType, std::shared_future<int32_t>> sinkResult;
    ret = StartSink(dhDescriptor.dhType, sinkResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StartSink failed");
        return ret;
    }
    if (!WaitForResult(Action::START_SINK, sinkResult)) {
        DHLOGE("StartSink failed, some virtual components maybe cannot work, but want to continue!");
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk start sink failed.");
        UninitCompSink(dhDescriptor.dhType);
        return ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT;
    }
    // Change status, we won't call back directly here because there is a lock
    statusCtrl.enableState = EnableState::ENABLED;
    enableInfo.refEnable = 1;
    status.refLoad = 1;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::DisableSinkInternal(const DHDescriptor &dhDescriptor,
    int32_t callingUid, int32_t callingPid, sptr<IHDSinkStatusListener> &listener)
{
    DHLOGI("Start DisableSinkInternal, dhType: %{public}#X", dhDescriptor.dhType);
    std::lock_guard<std::mutex> lock(dhSinkStatusMtx_);
    // Check if the input parameters and device type support it
    if (!ComponentLoader::GetInstance().IsDHTypeSupport(dhDescriptor.dhType)) {
        DHLOGE("Not support dhType: %{public}#X!", dhDescriptor.dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    auto &status = dhSinkStatus_[dhDescriptor.dhType];
    auto &enableInfo = status.enableInfos[dhDescriptor.id];

    // Check if the business is being called repeatedly
    DHStatusCtrlKey ctrlKey { .uid = callingUid, .pid = callingPid };

    // Get business enable status listener
    auto itrListener = status.listeners.find(ctrlKey);
    if (itrListener != status.listeners.end()) {
        listener = itrListener->second;
    }

    auto &statusCtrl = enableInfo.dhStatusCtrl[ctrlKey];
    if (statusCtrl.enableState == EnableState::DISABLED) {
        DHLOGE("Repeat call DisableSink, uid = %{public}d, pid = %{public}d.", ctrlKey.uid, ctrlKey.pid);
        return DH_FWK_SUCCESS;
    }

    // Check reference count
    if (enableInfo.refEnable > 1 || status.refLoad > 1) {
        // Change status, we won't call back directly here because there is a lock
        DHLOGI("Delete reference count, dhType: %{public}#X", dhDescriptor.dhType);
        statusCtrl.enableState = EnableState::DISABLED;
        enableInfo.refEnable--;
        status.refLoad--;
        return DH_FWK_SUCCESS;
    }

    // Start disabling hardware sink
    std::unordered_map<DHType, std::shared_future<int32_t>> sinkResult;
    auto ret = StopSink(dhDescriptor.dhType, sinkResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StopSink failed!");
        return ret;
    }
    if (!WaitForResult(Action::STOP_SINK, sinkResult)) {
        DHLOGE("StopSink failed, but want to continue!");
        return ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT;
    }
    ret = UninitCompSink(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UninitCompSink failed, ret = %{public}d.", ret);
        return ret;
    }
    // Change status, we won't call back directly here because there is a lock
    statusCtrl.enableState = EnableState::DISABLED;
    enableInfo.refEnable = 0;
    status.refLoad = 0;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::EnableSourceInternal(const std::string &networkId,
    const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid, sptr<IHDSourceStatusListener> &listener)
{
    DHLOGI("Start EnableSourceInternal, dhType: %{public}#X", dhDescriptor.dhType);
    // Check if the input parameters and device type support it
    if (!ComponentLoader::GetInstance().IsDHTypeSupport(dhDescriptor.dhType)) {
        DHLOGE("Not support dhType: %{public}#X!", dhDescriptor.dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    DHStatusSourceEnableInfoKey enableInfoKey { .networkId = networkId, .dhId = dhDescriptor.id };
    DHStatusCtrlKey ctrlKey { .uid = callingUid, .pid = callingPid };
    auto uuid = DHContext::GetInstance().GetUUIDByNetworkId(networkId);

    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);

    auto &status = dhSourceStatus_[dhDescriptor.dhType];
    auto &enableInfo = status.enableInfos[enableInfoKey];

    // Get business enable status listener
    auto itrListener = status.listeners.find(ctrlKey);
    if (itrListener != status.listeners.end()) {
        listener = itrListener->second;
    }

    // Check if the business is being called repeatedly
    auto &statusCtrl = enableInfo.dhStatusCtrl[ctrlKey];
    if (statusCtrl.enableState == EnableState::ENABLED) {
        DHLOGE("Repeat call EnableSource, uid = %{public}d, pid = %{public}d.", ctrlKey.uid, ctrlKey.pid);
        return DH_FWK_SUCCESS;
    }

    // Check enable reference count
    if (enableInfo.refEnable) {
        // Change status, we won't call back directly here because there is a lock
        DHLOGI("Add reference count, dhType: %{public}#X", dhDescriptor.dhType);
        statusCtrl.enableState = EnableState::ENABLED;
        enableInfo.refEnable++;
        status.refLoad++;
        return DH_FWK_SUCCESS;
    }

    // Check load reference count
    if (status.refLoad) {
        auto ret = Enable(networkId, uuid, dhDescriptor.id, dhDescriptor.dhType,
            (callingUid != 0) || (callingPid != 0));
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("Enable failed, ret = %{public}d.", ret);
            return ret;
        }
        // Change status, we won't call back directly here because there is a lock
        statusCtrl.enableState = EnableState::ENABLED;
        enableInfo.refEnable++;
        status.refLoad++;
        return DH_FWK_SUCCESS;
    }

    auto ret = RealEnableSource(networkId, uuid, dhDescriptor, statusCtrl, enableInfo, status,
        (callingUid != 0) || (callingPid != 0));
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RealEnableSource failed, ret = %{public}d.", ret);
        return ret;
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::DisableSourceInternal(const std::string &networkId,
    const DHDescriptor &dhDescriptor, int32_t callingUid, int32_t callingPid, sptr<IHDSourceStatusListener> &listener)
{
    DHLOGI("Start DisableSourceInternal, dhType: %{public}#X", dhDescriptor.dhType);
    // Check if the input parameters and device type support it
    if (!ComponentLoader::GetInstance().IsDHTypeSupport(dhDescriptor.dhType)) {
        DHLOGE("Not support dhType: %{public}#X!", dhDescriptor.dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    DHStatusSourceEnableInfoKey enableInfoKey { .networkId = networkId, .dhId = dhDescriptor.id };
    DHStatusCtrlKey ctrlKey { .uid = callingUid, .pid = callingPid };
    auto uuid = DHContext::GetInstance().GetUUIDByNetworkId(networkId);

    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);

    auto &status = dhSourceStatus_[dhDescriptor.dhType];
    auto &enableInfo = status.enableInfos[enableInfoKey];

    // Get business enable status listener
    auto itrListener = status.listeners.find(ctrlKey);
    if (itrListener != status.listeners.end()) {
        listener = itrListener->second;
    }

    // Check if the business is being called repeatedly
    auto &statusCtrl = enableInfo.dhStatusCtrl[ctrlKey];
    if (statusCtrl.enableState == EnableState::DISABLED) {
        DHLOGE("Repeat call DisableSource, uid = %{public}d, pid = %{public}d.", ctrlKey.uid, ctrlKey.pid);
        return DH_FWK_SUCCESS;
    }

    // Check enable reference count
    if (enableInfo.refEnable > 1) {
        // Change status, we won't call back directly here because there is a lock
        DHLOGI("Delete reference count, dhType: %{public}#X", dhDescriptor.dhType);
        statusCtrl.enableState = EnableState::DISABLED;
        enableInfo.refEnable--;
        status.refLoad--;
        return DH_FWK_SUCCESS;
    }

    // Check load reference count
    if (status.refLoad > 1) {
        auto ret = Disable(networkId, uuid, dhDescriptor.id, dhDescriptor.dhType);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("Disable failed, ret = %{public}d.", ret);
            return ret;
        }
        // Change status, we won't call back directly here because there is a lock
        statusCtrl.enableState = EnableState::DISABLED;
        enableInfo.refEnable--;
        status.refLoad--;
        return DH_FWK_SUCCESS;
    }

    auto ret = RealDisableSource(networkId, uuid, dhDescriptor, statusCtrl, enableInfo, status);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RealDisableSource failed, ret = %{public}d.", ret);
        return ret;
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::ForceDisableSinkInternal(
    const DHDescriptor &dhDescriptor, std::vector<sptr<IHDSinkStatusListener>> &listeners)
{
    std::lock_guard<std::mutex> lock(dhSinkStatusMtx_);

    // Check if the input parameters and device type support it
    if (!ComponentLoader::GetInstance().IsDHTypeSupport(dhDescriptor.dhType)) {
        DHLOGE("Not support dhType: %{public}#X!", dhDescriptor.dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }

    auto &status = dhSinkStatus_[dhDescriptor.dhType];
    auto itEnableInfo = status.enableInfos.find(dhDescriptor.id);
    if (itEnableInfo == status.enableInfos.end()) {
        DHLOGE("Repeat call ForceDisableSink, dhType = %{public}u, id = %{public}s.",
            dhDescriptor.dhType, GetAnonyString(dhDescriptor.id).c_str());
        return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
    }
    auto &enableInfo = itEnableInfo->second;

    // Collect listeners and reduce the load count
    for (auto &item : enableInfo.dhStatusCtrl) {
        if (item.second.enableState != EnableState::DISABLED) {
            auto it = status.listeners.find(item.first);
            if (it != status.listeners.end()) {
                auto listener = it->second;
                listeners.push_back(listener);
            }
        }
    }
    status.refLoad -= enableInfo.refEnable;
    status.enableInfos.erase(itEnableInfo);
    if (status.refLoad > 0) {
        return DH_FWK_SUCCESS;
    }

    // Unload component
    std::unordered_map<DHType, std::shared_future<int32_t>> sinkResult;
    auto ret = StopSink(dhDescriptor.dhType, sinkResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StopSink failed!");
        return ret;
    }
    if (!WaitForResult(Action::STOP_SINK, sinkResult)) {
        DHLOGE("StopSink failed, but want to continue!");
        return ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT;
    }
    ret = UninitCompSink(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UninitCompSink failed, ret = %{public}d.", ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::ForceDisableSourceInternal(const std::string &networkId,
    const DHDescriptor &dhDescriptor, std::vector<sptr<IHDSourceStatusListener>> &listeners)
{
    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);
    DHStatusSourceEnableInfoKey enableInfoKey { networkId, dhDescriptor.id };
    auto &status = dhSourceStatus_[dhDescriptor.dhType];
    auto itEnableInfo = status.enableInfos.find(enableInfoKey);
    if (itEnableInfo == status.enableInfos.end()) {
        DHLOGE("Repeat call ForceDisableSource, networkId = %{public}s, dhType = %{public}u, id = %{public}s.",
            GetAnonyString(networkId).c_str(), dhDescriptor.dhType, GetAnonyString(dhDescriptor.id).c_str());
        return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
    }
    auto &enableInfo = itEnableInfo->second;

    // First, disable the hardware
    auto uuid = DHContext::GetInstance().GetUUIDByNetworkId(networkId);
    auto ret = Disable(networkId, uuid, dhDescriptor.id, dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Disable failed, ret = %{public}d.", ret);
        return ret;
    }

    // Then collect listeners and reduce the load count
    for (auto &item : enableInfo.dhStatusCtrl) {
        if (item.second.enableState != EnableState::DISABLED) {
            auto it = status.listeners.find(item.first);
            if (it != status.listeners.end()) {
                listeners.push_back(it->second);
            }
        }
    }
    status.refLoad -= enableInfo.refEnable;
    status.enableInfos.erase(itEnableInfo);
    if (status.refLoad > 0) {
        return DH_FWK_SUCCESS;
    }

    // Unload component
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    ret = StopSource(dhDescriptor.dhType, sourceResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StopSource failed");
        return ret;
    }
    if (!WaitForResult(Action::STOP_SOURCE, sourceResult)) {
        DHLOGE("StopSource timeout!");
        return ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT;
    }
    ret = UninitCompSource(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UninitCompSource failed, ret = %{public}d.", ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::RealEnableSource(const std::string &networkId, const std::string &uuid,
    const DHDescriptor &dhDescriptor, DHStatusCtrl &statusCtrl,
    DHStatusEnableInfo &enableInfo, DHSourceStatus &status, bool isActive)
{
    auto ret = InitCompSource(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("InitCompSource failed, ret = %{public}d.", ret);
        return ret;
    }
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    ret = StartSource(dhDescriptor.dhType, sourceResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StartSource failed");
        return ret;
    }
    if (!WaitForResult(Action::START_SOURCE, sourceResult)) {
        DHLOGE("StartSource failed, some virtual components maybe cannot work, but want to continue!");
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk start source failed.");
        UninitCompSource(dhDescriptor.dhType);
        return ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT;
    }
    ret = Enable(networkId, uuid, dhDescriptor.id, dhDescriptor.dhType, isActive);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Enable failed, ret = %{public}d.", ret);
        std::unordered_map<DHType, std::shared_future<int32_t>> futureResult;
        StopSource(dhDescriptor.dhType, futureResult);
        UninitCompSource(dhDescriptor.dhType);
        return ret;
    }
    // Change status, we won't call back directly here because there is a lock
    statusCtrl.enableState = EnableState::ENABLED;
    enableInfo.refEnable = 1;
    status.refLoad = 1;
    return ret;
}

int32_t ComponentManager::RealDisableSource(const std::string &networkId, const std::string &uuid,
    const DHDescriptor &dhDescriptor, DHStatusCtrl &statusCtrl,
    DHStatusEnableInfo &enableInfo, DHSourceStatus &status)
{
    auto ret = Disable(networkId, uuid, dhDescriptor.id, dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Disable failed, ret = %{public}d.", ret);
        return ret;
    }
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    ret = StopSource(dhDescriptor.dhType, sourceResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StopSource failed");
        return ret;
    }
    if (!WaitForResult(Action::STOP_SOURCE, sourceResult)) {
        DHLOGE("StopSource timeout!");
        return ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT;
    }
    ret = UninitCompSource(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UninitCompSource failed, ret = %{public}d.", ret);
        return ret;
    }
    // Change status, we won't call back directly here because there is a lock
    statusCtrl.enableState = EnableState::DISABLED;
    enableInfo.refEnable = 0;
    status.refLoad = 0;
    return DH_FWK_SUCCESS;
}

void ComponentManager::ResetSinkEnableStatus(DHType dhType)
{
    DHLOGI("ResetSinkEnableStatus begin, dhType = %{public}#X.", dhType);
    std::lock_guard<std::mutex> lock(dhSinkStatusMtx_);
    auto ret = UninitCompSink(dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UninitCompSink failed, ret = %{public}d, but want to continue!", ret);
    }
    auto itSinkStatus = dhSinkStatus_.find(dhType);
    if (itSinkStatus == dhSinkStatus_.end()) {
        DHLOGI("No need to reset the auto-enable-sink of this type, dhType: %{public}#X.", dhType);
        return;
    }
    auto &sinkStatus = itSinkStatus->second;
    sinkStatus.refLoad = 0;
    for (auto &itEnableInfo : sinkStatus.enableInfos) {
        auto &enableInfoKey = itEnableInfo.first;
        auto &enableInfo = itEnableInfo.second;
        enableInfo.refEnable = 0;
        for (auto &itStatusCtrl : enableInfo.dhStatusCtrl) {
            auto &statusCtrlKey = itStatusCtrl.first;
            auto &statusCtrl = itStatusCtrl.second;
            if (statusCtrlKey.uid == 0 && statusCtrlKey.pid == 0) {
                statusCtrl.enableState = EnableState::DISABLED;
                DHLOGI("reset the auto-enable-sink, dhId = %{public}s, dhType: %{public}#X.",
                    GetAnonyString(enableInfoKey).c_str(), dhType);
            }
        }
    }
    DHLOGI("ResetSinkEnableStatus end, dhType = %{public}#X.", dhType);
}

void ComponentManager::ResetSourceEnableStatus(DHType dhType)
{
    DHLOGI("ResetSourceEnableStatus begin, dhType = %{public}#X.", dhType);
    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);
    auto ret = UninitCompSource(dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UninitCompSource failed, ret = %{public}d, but want to continue!", ret);
    }
    auto itSourceStatus = dhSourceStatus_.find(dhType);
    if (itSourceStatus == dhSourceStatus_.end()) {
        DHLOGI("No need to reset the auto-enable-source of this type, dhType: %{public}#X.", dhType);
        return;
    }
    auto &sourcekStatus = itSourceStatus->second;
    sourcekStatus.refLoad = 0;
    for (auto &itEnableInfo : sourcekStatus.enableInfos) {
        auto &enableInfoKey = itEnableInfo.first;
        auto &enableInfo = itEnableInfo.second;
        enableInfo.refEnable = 0;
        for (auto &itStatusCtrl : enableInfo.dhStatusCtrl) {
            auto &statusCtrlKey = itStatusCtrl.first;
            auto &statusCtrl = itStatusCtrl.second;
            if (statusCtrlKey.uid == 0 && statusCtrlKey.pid == 0) {
                statusCtrl.enableState = EnableState::DISABLED;
                DHLOGI("reset the auto-enable-source, networkId = %{public}s, dhId = %{public}s, dhType: %{public}#X.",
                    GetAnonyString(enableInfoKey.networkId).c_str(),
                    GetAnonyString(enableInfoKey.dhId).c_str(), dhType);
            }
        }
    }
    DHLOGI("ResetSourceEnableStatus end, dhType = %{public}#X.", dhType);
}

void ComponentManager::RecoverAutoEnableSink(DHType dhType)
{
    DHLOGI("RecoverAutoEnableSink begin, dhType = %{public}#X.", dhType);
    DeviceInfo localDeviceInfo = GetLocalDeviceInfo();
    std::vector<std::pair<std::string, DHType>> localMetaInfos;
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCapInfos;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(localDeviceInfo.udidHash, metaCapInfos);
    std::for_each(metaCapInfos.begin(), metaCapInfos.end(), [&](std::shared_ptr<MetaCapabilityInfo> localMetaInfo) {
        if (localMetaInfo != nullptr && localMetaInfo->GetDHType() == dhType) {
            localMetaInfos.push_back({localMetaInfo->GetDHId(), localMetaInfo->GetDHType()});
        }
    });
    if (localMetaInfos.empty()) {
        DHLOGE("No need to recover auto enable sink, dhType = %{public}#X.", dhType);
        return;
    }
    for (const auto &localInfo : localMetaInfos) {
        TaskParam taskParam = {
            .networkId = localDeviceInfo.networkId,
            .uuid = localDeviceInfo.uuid,
            .udid = localDeviceInfo.udid,
            .dhId = localInfo.first,
            .dhType = localInfo.second
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
    DHLOGI("RecoverAutoEnableSink end, dhType = %{public}#X.", dhType);
}

void ComponentManager::RecoverAutoEnableSource(DHType dhType)
{
    DHLOGI("RecoverAutoEnableSource begin, dhType = %{public}#X.", dhType);
    MetaCapInfoMap metaInfoMap;
    MetaInfoManager::GetInstance()->GetMetaDataByDHType(dhType, metaInfoMap);
    for (const auto &metaInfo : metaInfoMap) {
        std::string uuid = DHContext::GetInstance().GetUUIDByDeviceId(metaInfo.second->GetDeviceId());
        if (uuid.empty()) {
            DHLOGE("Can not find uuid by capability deviceId: %{public}s",
                GetAnonyString(metaInfo.second->GetDeviceId()).c_str());
            continue;
        }

        std::string networkId = DHContext::GetInstance().GetNetworkIdByUUID(uuid);
        if (networkId.empty()) {
            DHLOGI("Can not find network id by uuid: %{public}s", GetAnonyString(uuid).c_str());
            continue;
        }

        TaskParam taskParam = {
            .networkId = networkId,
            .uuid = uuid,
            .dhId = metaInfo.second->GetDHId(),
            .dhType = metaInfo.second->GetDHType()
        };
        auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
        TaskExecutor::GetInstance().PushTask(task);
    }
    DHLOGI("RecoverAutoEnableSource end, dhType = %{public}#X.", dhType);
}

void ComponentManager::RecoverActiveEnableSink(DHType dhType)
{
    DHLOGI("RecoverActiveEnableSink begin, dhType = %{public}#X.", dhType);
    std::lock_guard<std::mutex> lock(dhSinkStatusMtx_);
    auto itSinkStatus = dhSinkStatus_.find(dhType);
    if (itSinkStatus == dhSinkStatus_.end()) {
        DHLOGE("No need to recover active enable sink, dhType = %{public}#X.", dhType);
        return;
    }
    auto &sinkStatus = itSinkStatus->second;
    for (auto &itEnableInfo : sinkStatus.enableInfos) {
        auto &enableInfoKey = itEnableInfo.first;
        auto &enableInfo = itEnableInfo.second;
        for (auto &itStatusCtrl : enableInfo.dhStatusCtrl) {
            auto &statusCtrlKey = itStatusCtrl.first;
            auto &statusCtrl = itStatusCtrl.second;
            if ((statusCtrlKey.uid != 0 || statusCtrlKey.pid != 0)
                && statusCtrl.enableState == EnableState::ENABLED) {
                statusCtrl.enableState = EnableState::DISABLED;
                TaskParam taskParam = {
                    .dhId = enableInfoKey,
                    .dhType = dhType,
                    .effectSink = true,
                    .effectSource = false,
                    .callingUid = statusCtrlKey.uid,
                    .callingPid = statusCtrlKey.pid
                };
                auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
                TaskExecutor::GetInstance().PushTask(task);
                DHLOGI("Create enable task for recover active-enable-sink, dhId = %{public}s, dhType = %{public}#X.",
                    GetAnonyString(enableInfoKey).c_str(), dhType);
            }
        }
    }
    DHLOGI("RecoverActiveEnableSink end, dhType = %{public}#X.", dhType);
}

void ComponentManager::RecoverActiveEnableSource(DHType dhType)
{
    DHLOGI("RecoverActiveEnableSource begin, dhType = %{public}#X.", dhType);
    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);
    auto itSourceStatus = dhSourceStatus_.find(dhType);
    if (itSourceStatus == dhSourceStatus_.end()) {
        DHLOGE("No need to recover active enable source, dhType = %{public}#X.", dhType);
        return;
    }
    auto &sourcekStatus = itSourceStatus->second;
    for (auto &itEnableInfo : sourcekStatus.enableInfos) {
        auto &enableInfoKey = itEnableInfo.first;
        auto &enableInfo = itEnableInfo.second;
        for (auto &itStatusCtrl : enableInfo.dhStatusCtrl) {
            auto &statusCtrlKey = itStatusCtrl.first;
            auto &statusCtrl = itStatusCtrl.second;
            if ((statusCtrlKey.uid != 0 || statusCtrlKey.pid != 0)
                && statusCtrl.enableState == EnableState::ENABLED) {
                statusCtrl.enableState = EnableState::DISABLED;
                TaskParam taskParam = {
                    .networkId = enableInfoKey.networkId,
                    .dhId = enableInfoKey.dhId,
                    .dhType = dhType,
                    .effectSink = false,
                    .effectSource = true,
                    .callingUid = statusCtrlKey.uid,
                    .callingPid = statusCtrlKey.pid
                };
                auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
                TaskExecutor::GetInstance().PushTask(task);
                DHLOGI("Create enable task for recover active-enable-sink, "
                    "networkId = %{public}s, dhId = %{public}s, dhType = %{public}#X.",
                    GetAnonyString(enableInfoKey.networkId).c_str(),
                    GetAnonyString(enableInfoKey.dhId).c_str(), dhType);
            }
        }
    }
    DHLOGI("RecoverActiveEnableSource end, dhType = %{public}#X.", dhType);
}

int32_t ComponentManager::InitCompSource(DHType dhType)
{
    std::unique_lock<std::shared_mutex> lock(compSourceMutex_);
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(dhType, sourcePtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetSource failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
        return ret;
    }
    if (sourcePtr == nullptr) {
        DHLOGE("sourcePtr is null, compType = %{public}#X.", dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    compSource_.insert(std::make_pair(dhType, sourcePtr));
    auto saId = ComponentLoader::GetInstance().GetSourceSaId(dhType);
    if (saId != INVALID_SA_ID) {
        compSrcSaId_.insert(std::make_pair(dhType, saId));
        if (compMonitorPtr_ == nullptr) {
            DHLOGE("compMonitorPtr_ is null.");
            return ERR_DH_FWK_COMPONENT_MONITOR_NULL;
        }
        compMonitorPtr_->AddSAMonitor(saId);
    } else {
        DHLOGE("GetSourceSaId return INVALID_SA_ID, compType = %{public}#X.", dhType);
    }
    sourcePtr->RegisterDistributedHardwareStateListener(dhStateListener_);
    sourcePtr->RegisterDataSyncTriggerListener(dataSyncTriggerListener_);
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::UninitCompSource(DHType dhType)
{
    std::unique_lock<std::shared_mutex> lock(compSourceMutex_);
    IDistributedHardwareSource *sourcePtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSource(dhType, sourcePtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetSource failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
        return ret;
    }
    if (sourcePtr == nullptr) {
        DHLOGE("sourcePtr is null, compType = %{public}#X.", dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sourcePtr->UnregisterDataSyncTriggerListener();
    sourcePtr->UnregisterDistributedHardwareStateListener();
    auto it = compSrcSaId_.find(dhType);
    if (it != compSrcSaId_.end()) {
        if (compMonitorPtr_ == nullptr) {
            DHLOGE("compMonitorPtr_ is null.");
            return ERR_DH_FWK_COMPONENT_MONITOR_NULL;
        }
        compMonitorPtr_->RemoveSAMonitor(it->second);
        compSrcSaId_.erase(it);
    }
    ret = ComponentLoader::GetInstance().ReleaseSource(dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetSource failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
        return ret;
    }
    compSource_.erase(dhType);
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::InitCompSink(DHType dhType)
{
    std::unique_lock<std::shared_mutex> lock(compSinkMutex_);
    IDistributedHardwareSink *sinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(dhType, sinkPtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetSink failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
        return ret;
    }
    if (sinkPtr == nullptr) {
        DHLOGE("sinkPtr is null, compType = %{public}#X.", dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sinkPtr->RegisterDistributedHardwareSinkStateListener(dhSinkStateListener_);
    compSink_.insert(std::make_pair(dhType, sinkPtr));
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::UninitCompSink(DHType dhType)
{
    std::unique_lock<std::shared_mutex> lock(compSinkMutex_);
    IDistributedHardwareSink *sinkPtr = nullptr;
    auto ret = ComponentLoader::GetInstance().GetSink(dhType, sinkPtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("GetSinkPtr failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
        return ret;
    }
    if (sinkPtr == nullptr) {
        DHLOGE("sinkPtr is null, compType = %{public}#X.", dhType);
        return ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
    }
    sinkPtr->UnregisterDistributedHardwareSinkStateListener();
    ret = ComponentLoader::GetInstance().ReleaseSink(dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("ReleaseSink failed, compType = %{public}#X, ret = %{public}d.", dhType, ret);
        return ret;
    }
    compSink_.erase(dhType);
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::StopSource(DHType dhType, ActionResult &sourceResult)
{
    DHLOGI("StopSource, dhType: %{public}#X", dhType);
    std::shared_lock<std::shared_mutex> lock(compSourceMutex_);
    if (compSource_.find(dhType) == compSource_.end()) {
        DHLOGE("Component for DHType: %{public}" PRIu32 " not init source handler.", (uint32_t)dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    auto sourcePtr = compSource_[dhType];
    if (sourcePtr == nullptr) {
        DHLOGE("comp source ptr is null.");
        return ERR_DH_FWK_SA_HANDLER_IS_NULL;
    }
    std::promise<int32_t> p;
    std::future<int32_t> f = p.get_future();
    std::thread([p = std::move(p), sourcePtr] () mutable {
        p.set_value(sourcePtr->ReleaseSource());
    }).detach();
    sourceResult.emplace(dhType, f.share());
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::StopSink(DHType dhType, ActionResult &sinkResult)
{
    DHLOGI("StopSink, dhType: %{public}#X", dhType);
    std::shared_lock<std::shared_mutex> lock(compSinkMutex_);
    if (compSink_.find(dhType) == compSink_.end()) {
        DHLOGE("Component for DHType: %{public}" PRIu32 " not init sink handler.", (uint32_t)dhType);
        return ERR_DH_FWK_TYPE_NOT_EXIST;
    }
    auto sinkPtr = compSink_[dhType];
    if (sinkPtr == nullptr) {
        DHLOGE("comp sink ptr is null.");
        return ERR_DH_FWK_SA_HANDLER_IS_NULL;
    }
    std::promise<int32_t> p;
    std::future<int32_t> f = p.get_future();
    std::thread([p = std::move(p), sinkPtr, dhType] () mutable {
        p.set_value(sinkPtr->ReleaseSink());
        IHardwareHandler *hardwareHandler = nullptr;
        int32_t status = ComponentLoader::GetInstance().GetHardwareHandler(dhType, hardwareHandler);
        if (status != DH_FWK_SUCCESS || hardwareHandler == nullptr) {
            DHLOGE("GetHardwareHandler %{public}#X failed.", dhType);
            return status;
        }
        hardwareHandler->UnRegisterPluginListener();
        return status;
    }).detach();
    sinkResult.emplace(dhType, f.share());
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::DisableMetaSource(const std::string &networkId, const DHDescriptor &dhDescriptor,
    std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr)
{
    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);
    DHStatusSourceEnableInfoKey enableInfoKey {
        .networkId = networkId,
        .dhId = dhDescriptor.id
    };
    DHStatusCtrlKey ctrlKey {
        .uid = 0,
        .pid = 0
    };

    auto &status = dhSourceStatus_[dhDescriptor.dhType];
    auto &enableInfo = status.enableInfos[enableInfoKey];

    // Check if the business is being called repeatedly
    auto &statusCtrl = enableInfo.dhStatusCtrl[ctrlKey];
    if (statusCtrl.enableState == EnableState::DISABLED) {
        DHLOGE("Repeat call DisableSource, DhType = %{public}#X.", dhDescriptor.dhType);
        return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
    }

    // Check enable reference count
    if (enableInfo.refEnable > 1) {
        // Change status, we won't call back directly here because there is a lock
        statusCtrl.enableState = EnableState::DISABLED;
        enableInfo.refEnable--;
        status.refLoad--;
        return DH_FWK_SUCCESS;
    }

    // Check load reference count
    if (status.refLoad > 1) {
        DHLOGI("Meta disable, networkId = %{public}s", GetAnonyString(networkId).c_str());
        if (dhModemExt->Disable(networkId, sourcePtr) != DH_FWK_SUCCESS) {
            DHLOGE("Meta disable failed, networkId = %{public}s.", GetAnonyString(networkId).c_str());
        }
        // Change status, we won't call back directly here because there is a lock
        statusCtrl.enableState = EnableState::DISABLED;
        enableInfo.refEnable--;
        status.refLoad--;
        return DH_FWK_SUCCESS;
    }

    auto ret = DisableMetaSourceInternal(networkId, dhDescriptor, statusCtrl, enableInfo, status,
        dhModemExt, sourcePtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DisableMetaSource failed, ret = %{public}d.", ret);
        return ret;
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::DisableMetaSourceInternal(const std::string &networkId, const DHDescriptor &dhDescriptor,
    DHStatusCtrl &statusCtrl, DHStatusEnableInfo &enableInfo, DHSourceStatus &status,
    std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr)
{
    auto ret = dhModemExt->Disable(networkId, sourcePtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Meta disable source failed, ret = %{public}d.", ret);
    }
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    ret = StopSource(dhDescriptor.dhType, sourceResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StopSource failed");
        return ret;
    }
    if (!WaitForResult(Action::STOP_SOURCE, sourceResult)) {
        DHLOGE("StopSource timeout!");
        return ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT;
    }
    ret = UninitCompSource(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UninitCompSource failed, ret = %{public}d.", ret);
        return ret;
    }
    // Change status, we won't call back directly here because there is a lock
    statusCtrl.enableState = EnableState::DISABLED;
    enableInfo.refEnable = 0;
    status.refLoad = 0;
    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::EnableMetaSource(const std::string &networkId, const DHDescriptor &dhDescriptor,
    std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr)
{
    DHLOGI("EnableMetaSource enter");
    std::lock_guard<std::mutex> lock(dhSourceStatusMtx_);
    DHStatusSourceEnableInfoKey enableInfoKey { .networkId = networkId, .dhId = dhDescriptor.id };
    DHStatusCtrlKey ctrlKey { .uid = 0, .pid = 0 };
    auto &status = dhSourceStatus_[dhDescriptor.dhType];
    auto &enableInfo = status.enableInfos[enableInfoKey];

    // Check if the business is being called repeatedly
    auto &statusCtrl = enableInfo.dhStatusCtrl[ctrlKey];
    if (statusCtrl.enableState == EnableState::ENABLED) {
        DHLOGE("Repeat call EnableMetaSource, DhType = %{public}#X.", dhDescriptor.dhType);
        return ERR_DH_FWK_COMPONENT_REPEAT_CALL;
    }

    // Check enable reference count
    if (enableInfo.refEnable > 0) {
        // Change status, we won't call back directly here because there is a lock
        statusCtrl.enableState = EnableState::ENABLED;
        enableInfo.refEnable++;
        status.refLoad++;
        return DH_FWK_SUCCESS;
    }

    // Check load reference count
    if (status.refLoad > 0) {
        DHLOGI("Meta enable, networkId = %{public}s", GetAnonyString(networkId).c_str());
        if (dhModemExt->Enable(networkId, sourcePtr) != DH_FWK_SUCCESS) {
            DHLOGW("Meta enable failed, networkId = %{public}s.", GetAnonyString(networkId).c_str());
            return ERR_DH_FWK_PARA_INVALID;
        }
        // Change status, we won't call back directly here because there is a lock
        statusCtrl.enableState = EnableState::ENABLED;
        enableInfo.refEnable++;
        status.refLoad++;
        return DH_FWK_SUCCESS;
    }

    auto ret = EnableMetaSourceInternal(networkId, dhDescriptor, statusCtrl, enableInfo, status, dhModemExt, sourcePtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("EnableMetaSource failed, ret = %{public}d.", ret);
        return ret;
    }

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::EnableMetaSourceInternal(const std::string &networkId, const DHDescriptor &dhDescriptor,
    DHStatusCtrl &statusCtrl, DHStatusEnableInfo &enableInfo, DHSourceStatus &status,
    std::shared_ptr<IDistributedModemExt> dhModemExt, IDistributedHardwareSource *&sourcePtr)
{
    auto ret = InitCompSource(dhDescriptor.dhType);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("InitCompSource failed, ret = %{public}d.", ret);
        return ret;
    }
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    ret = StartSource(dhDescriptor.dhType, sourceResult);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StartSource failed");
        return ret;
    }
    if (!WaitForResult(Action::START_SOURCE, sourceResult)) {
        DHLOGE("StartSource failed, some virtual components maybe cannot work, but want to continue!");
        HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            "dhfwk start source failed.");
        UninitCompSource(dhDescriptor.dhType);
        return ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT;
    }
    ret = dhModemExt->Enable(networkId, sourcePtr);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("EnableMeta failed, ret = %{public}d.", ret);
        std::unordered_map<DHType, std::shared_future<int32_t>> futureResult;
        StopSource(dhDescriptor.dhType, futureResult);
        UninitCompSource(dhDescriptor.dhType);
        return ret;
    }
    statusCtrl.enableState = EnableState::ENABLED;
    enableInfo.refEnable = 1;
    status.refLoad = 1;
    return ret;
}

void ComponentManager::SyncRemoteDeviceInfoBySoftbus(const std::string &realNetworkId, EnableStep enableStep,
    const sptr<IGetDhDescriptorsCallback> callback)
{
    if (callback == nullptr) {
        DHLOGE("Param callback is null.");
        return;
    }
    std::lock_guard<std::mutex> lock(syncDeviceInfoMapMutex_);
    syncDeviceInfoMap_[realNetworkId] = {enableStep, callback};
    std::shared_ptr<std::string> networkIdPtr = std::make_shared<std::string>(realNetworkId);
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(EVENT_DATA_SYNC_MANUAL, networkIdPtr);
    if (GetEventHandler() == nullptr) {
        DHLOGE("Can not get eventHandler");
        callback->OnError(realNetworkId, ERR_DH_FWK_POINTER_IS_NULL);
        syncDeviceInfoMap_.erase(realNetworkId);
        return;
    }
    GetEventHandler()->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);

    std::thread([this]() {
        this->OnGetDescriptorsError();
    }).detach();
}

void ComponentManager::OnGetDescriptors(const std::string &realNetworkId, const std::vector<DHDescriptor> &descriptors)
{
    DHLOGI("OnGetDescriptors enter, networkId = %{public}s, descriptors.size = %{public}zu",
        GetAnonyString(realNetworkId).c_str(), descriptors.size());
    if (descriptors.size() == 0) {
        DHLOGE("Get dh descriptor faild");
        return;
    }
    std::lock_guard<std::mutex> lock(syncDeviceInfoMapMutex_);
    auto iter = syncDeviceInfoMap_.find(realNetworkId);
    if (iter != syncDeviceInfoMap_.end()) {
        if (iter->second.second != nullptr) {
            iter->second.second->OnSuccess(realNetworkId, descriptors, iter->second.first);
            syncDeviceInfoMap_.erase(iter);
            DHLOGI("Notify get dh descriptor success.");
        }
    }
}

void ComponentManager::OnGetDescriptorsError()
{
    DHLOGI("OnGetDescriptorsError enter");
    std::this_thread::sleep_for(std::chrono::milliseconds(SYNC_DATA_TIMEOUT_MS));
    std::lock_guard<std::mutex> lock(syncDeviceInfoMapMutex_);
    for (auto iter = syncDeviceInfoMap_.begin(); iter != syncDeviceInfoMap_.end();) {
        if (iter->second.second != nullptr) {
            DHLOGI("OnGetDescriptorsError networkId = %{public}s", GetAnonyString(iter->first).c_str());
            iter->second.second->OnError(iter->first, ERR_DH_FWK_GETDISTRIBUTEDHARDWARE_TIMEOUT);
            iter = syncDeviceInfoMap_.erase(iter);
        } else {
            ++iter;
        }
    }
}

int32_t ComponentManager::AddAccessListener(const DHType dhType, int32_t &timeOut, const std::string &pkgName,
    const sptr<IAuthorizationResultCallback> &callback)
{
    DHLOGI("AddAccessListener dhType=%{public}u, pkgName=%{public}s", (uint32_t)dhType, pkgName.c_str());

    if (callback == nullptr) {
        DHLOGE("callback is nullptr");
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (pkgName.empty()) {
        DHLOGE("pkgName is empty");
        return ERR_DH_FWK_PARA_INVALID;
    }

    std::map<DHType, IDistributedHardwareSink*> dhSinkMap = GetDHSinkInstance();
    if (dhSinkMap.empty()) {
        DHLOGE("Get DH sink instance map is empty");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    if (dhSinkMap.find(dhType) == dhSinkMap.end()) {
        DHLOGE("SetAccessListener for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (dhSinkMap[dhType] == nullptr) {
        DHLOGE("Sinkhandler ptr is null");
        return ERR_DH_FWK_PARA_INVALID;
    }

    sptr<IAccessListener> listener(new AccessListenerService());
    int32_t ret = dhSinkMap[dhType]->SetAccessListener(listener, timeOut, pkgName);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("SetAccessListener failed for DHType=%{public}u, ret=%{public}d",
            (uint32_t)dhType, ret);
        return ret;
    }

    std::lock_guard<std::mutex> lock(accessListenerMutex_);
    auto key = std::make_pair(dhType, pkgName);
    if (accessListenerMap_.find(key) != accessListenerMap_.end()) {
        DHLOGW("DHType=%{public}u, pkgName=%{public}s already registered, will update callback",
            (uint32_t)dhType, pkgName.c_str());
    }

    accessListenerMap_[key] = callback;
    DHLOGI("Save callback for DHType=%{public}u, pkgName=%{public}s, current map size=%{public}zu",
        (uint32_t)dhType, pkgName.c_str(), accessListenerMap_.size());

    return DH_FWK_SUCCESS;
}

int32_t ComponentManager::RemoveAccessListener(const DHType dhType, const std::string &pkgName)
{
    DHLOGI("RemoveAccessListener dhType=%{public}u, pkgName=%{public}s", (uint32_t)dhType, pkgName.c_str());

    if (pkgName.empty()) {
        DHLOGE("pkgName is empty");
        return ERR_DH_FWK_PARA_INVALID;
    }

    std::map<DHType, IDistributedHardwareSink*> dhSinkMap = GetDHSinkInstance();
    if (dhSinkMap.empty()) {
        DHLOGE("Get DH sink instance map is empty");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    if (dhSinkMap.find(dhType) == dhSinkMap.end()) {
        DHLOGE("RemoveAccessListener for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (dhSinkMap[dhType] == nullptr) {
        DHLOGE("Sinkhandler ptr is null");
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = dhSinkMap[dhType]->RemoveAccessListener(pkgName);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RemoveAccessListener failed for DHType=%{public}u, ret=%{public}d",
            (uint32_t)dhType, ret);
        return ret;
    }

    std::lock_guard<std::mutex> lock(accessListenerMutex_);
    auto key = std::make_pair(dhType, pkgName);
    auto it = accessListenerMap_.find(key);
    if (it == accessListenerMap_.end()) {
        DHLOGW("DHType=%{public}u, pkgName=%{public}s not found in map", (uint32_t)dhType, pkgName.c_str());
        return ERR_DH_FWK_PARA_INVALID;
    }

    accessListenerMap_.erase(it);
    DHLOGI("Remove callback for DHType=%{public}u, pkgName=%{public}s, remaining size=%{public}zu",
        (uint32_t)dhType, pkgName.c_str(), accessListenerMap_.size());

    return DH_FWK_SUCCESS;
}

sptr<IAuthorizationResultCallback> ComponentManager::GetAccessListener(const DHType dhType, const std::string &pkgName)
{
    DHLOGI("GetAccessListener dhType=%{public}u, pkgName=%{public}s", (uint32_t)dhType, pkgName.c_str());

    if (pkgName.empty()) {
        DHLOGE("pkgName is empty");
        return nullptr;
    }

    std::lock_guard<std::mutex> lock(accessListenerMutex_);

    auto key = std::make_pair(dhType, pkgName);
    auto it = accessListenerMap_.find(key);
    if (it == accessListenerMap_.end()) {
        DHLOGW("DHType=%{public}u, pkgName=%{public}s not found in map", (uint32_t)dhType, pkgName.c_str());
        return nullptr;
    }

    return it->second;
}
} // namespace DistributedHardware
} // namespace OHOS
