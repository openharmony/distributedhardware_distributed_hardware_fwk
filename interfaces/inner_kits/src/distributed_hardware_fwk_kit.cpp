/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "distributed_hardware_fwk_kit.h"

#include <cinttypes>

#include "anonymous_string.h"
#include "constants.h"
#include "dh_utils_tool.h"
#include "dhfwk_sa_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "dh_utils_hisysevent.h"
#include "idistributed_hardware.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t WAIT_TIME_MILL = 3000;
}

class DFWKLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject);
    void OnLoadSystemAbilityFail(int32_t systemAbilityId);
    bool WaitLoadComplete();
private:
    bool isNotified_ = false;
    bool isLoadSuccess_ = false;
    std::mutex waitLoadCompleteMutex_;
    std::condition_variable waitLoadCompleteCondVar_;
};

void DFWKLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    std::unique_lock<std::mutex> locker(waitLoadCompleteMutex_);
    isNotified_ = true;
    isLoadSuccess_ = true;
    waitLoadCompleteCondVar_.notify_all();
}

void DFWKLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    std::unique_lock<std::mutex> locker(waitLoadCompleteMutex_);
    isNotified_ = true;
    isLoadSuccess_ = false;
    waitLoadCompleteCondVar_.notify_all();
}

bool DFWKLoadCallback::WaitLoadComplete()
{
    std::unique_lock<std::mutex> locker(waitLoadCompleteMutex_);
    auto waitStatus = waitLoadCompleteCondVar_.wait_for(locker, std::chrono::milliseconds(WAIT_TIME_MILL),
        [this]() { return isNotified_; });
    if (!waitStatus) {
        DHLOGE("Load distributed hardware SA timeout.");
        return false;
    }
    return isLoadSuccess_;
}

std::string DumpDescriptors(const std::vector<DHDescriptor> &descriptors)
{
    std::string descriptorsInfo = "[";
    for (auto& descriptor : descriptors) {
        descriptorsInfo += "[";
        descriptorsInfo += "dhType:";
        descriptorsInfo += std::to_string((uint32_t)descriptor.dhType);
        descriptorsInfo += ",";
        descriptorsInfo += "id:";
        descriptorsInfo += descriptor.id;
        descriptorsInfo += "]";
    }
    descriptorsInfo += "]";
    return descriptorsInfo;
}

DistributedHardwareFwkKit::DistributedHardwareFwkKit()
{
    DHLOGI("Ctor DistributedHardwareFwkKit");
    DHFWKSAManager::GetInstance().RegisterAbilityListener();
}

DistributedHardwareFwkKit::~DistributedHardwareFwkKit()
{
    DHLOGI("Dtor DistributedHardwareFwkKit");
}

int32_t DistributedHardwareFwkKit::RegisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener)
{
    DHLOGI("Register publisher listener, topic: %{public}" PRIu32 "", (uint32_t)topic);
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = DH_FWK_SUCCESS;
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() != nullptr) {
        ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterPublisherListener(topic, listener);
        DHLOGI("Register publisher listener to DHFWK, ret: %{public}" PRId32, ret);
        if (ret == DH_FWK_SUCCESS) {
            return DHFWKSAManager::GetInstance().AddPublisherListenerToCache(topic, listener);
        }
    } else {
        DHLOGI("DHFWK not online, or get proxy failed, save listener temporary");
        return DHFWKSAManager::GetInstance().AddPublisherListenerToCache(topic, listener);
    }

    return ret;
}

int32_t DistributedHardwareFwkKit::UnregisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener)
{
    DHLOGI("Unregister publisher listener, topic: %{public}" PRIu32 "", (uint32_t)topic);
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }

    int32_t ret = DH_FWK_SUCCESS;
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() != nullptr) {
        ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->UnregisterPublisherListener(topic, listener);
        DHLOGI("Unregister publisher listener to DHFWK, ret: %{public}" PRId32, ret);
    }

    DHFWKSAManager::GetInstance().RemovePublisherListenerFromCache(topic, listener);
    return ret;
}

int32_t DistributedHardwareFwkKit::PublishMessage(const DHTopic topic, const std::string &message)
{
    DHLOGI("Publish message, topic: %{public}" PRIu32, (uint32_t)topic);
    if (!IsDHTopicValid(topic)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)topic);
        return ERR_DH_FWK_PARA_INVALID;
    }
    if (!IsMessageLengthValid(message)) {
        return ERR_DH_FWK_PARA_INVALID;
    }

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online, can not publish message");
        return ERR_DH_FWK_PUBLISH_MSG_FAILED;
    }

    int32_t ret = DHFWKSAManager::GetInstance().GetDHFWKProxy()->PublishMessage(topic, message);
    DHLOGI("Publish message to DHFWK, ret: %{public}" PRId32, ret);

    return ret;
}

bool DistributedHardwareFwkKit::IsDHTopicValid(DHTopic topic)
{
    return topic > DHTopic::TOPIC_MIN && topic < DHTopic::TOPIC_MAX;
}

bool DistributedHardwareFwkKit::IsQueryLocalSysSpecTypeValid(QueryLocalSysSpecType spec)
{
    return spec > QueryLocalSysSpecType::MIN && spec < QueryLocalSysSpecType::MAX;
}

int32_t DistributedHardwareFwkKit::LoadDistributedHardwareSA()
{
    DHLOGI("Load distributed hardware SA begin.");
    std::unique_lock<std::mutex> locker(dfwkLoadServiceMutex_);
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        DHLOGE("Failed to get system ability mgr.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    sptr<IRemoteObject> remote = saMgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_SA_ID);
    if (remote != nullptr) {
        DHLOGI("DHFWK service has already been loaded!");
        return DH_FWK_SUCCESS;
    }
    sptr<DFWKLoadCallback> dfwkLoadCallback(new DFWKLoadCallback);
    if (dfwkLoadCallback == nullptr) {
        DHLOGE("Failed to create DFWK load callback.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t ret = saMgr->LoadSystemAbility(DISTRIBUTED_HARDWARE_SA_ID, dfwkLoadCallback);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Failed to load DFWK service, ret: %{public}d", ret);
        return ret;
    }
    if (!dfwkLoadCallback->WaitLoadComplete()) {
        DHLOGE("Load DFWK service callback failed");
        return ERR_DH_FWK_LOAD_CALLBACK_FAIL;
    }
    DHLOGI("Load distributed hardware SA end!");
    return DH_FWK_SUCCESS;
}

std::string DistributedHardwareFwkKit::QueryLocalSysSpec(enum QueryLocalSysSpecType spec)
{
    DHLOGI("Query Local Sys Spec, %{public}d", (uint32_t)spec);
    if (!IsQueryLocalSysSpecTypeValid(spec)) {
        DHLOGE("Topic invalid, topic: %{public}" PRIu32, (uint32_t)spec);
        return "";
    }

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online, can not publish message");
        return "";
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->QueryLocalSysSpec(spec);
}

int32_t DistributedHardwareFwkKit::InitializeAVCenter(const TransRole &transRole, int32_t &engineId)
{
    DHLOGI("Initialize av control center, transRole: %{public}" PRIu32, (uint32_t)transRole);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not initializeA av control center");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->InitializeAVCenter(transRole, engineId);
}

int32_t DistributedHardwareFwkKit::ReleaseAVCenter(int32_t engineId)
{
    DHLOGI("Release av control center, engineId: %{public}" PRId32, engineId);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not release av control center");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    DHFWKSAManager::GetInstance().RemoveAVTransControlCenterCbFromCache(engineId);
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->ReleaseAVCenter(engineId);
}

int32_t DistributedHardwareFwkKit::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    DHLOGI("Create av control center channel, engineId: %{public}" PRId32 ", peerDevId=%{public}s.", engineId,
        GetAnonyString(peerDevId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not create av control center channel");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->CreateControlChannel(engineId, peerDevId);
}

int32_t DistributedHardwareFwkKit::NotifyAVCenter(int32_t engineId, const AVTransEvent &event)
{
    DHLOGI("Notify av control center, engineId: %{public}" PRId32 ", event type=%{public}" PRId32, engineId,
        event.type);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not notity av control center event.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }

    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->NotifyAVCenter(engineId, event);
}

int32_t DistributedHardwareFwkKit::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAvTransControlCenterCallback> callback)
{
    DHLOGI("Register av control center callback. engineId: %{public}" PRId32, engineId);

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not register av control center callback.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    DHFWKSAManager::GetInstance().AddAVTransControlCenterCbToCache(engineId, callback);
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterCtlCenterCallback(engineId, callback);
}

int32_t DistributedHardwareFwkKit::PauseDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Pause distributed hardware dhType %{public}u, networkId %{public}s", (uint32_t)dhType,
        GetAnonyString(networkId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not pause distributed hardware.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    HiSysEventWriteMsg(DHFWK_INIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "user pause sink ui.");
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->PauseDistributedHardware(dhType, networkId);
}

int32_t DistributedHardwareFwkKit::ResumeDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Resume distributed hardware dhType %{public}u, networkId %{public}s", (uint32_t)dhType,
        GetAnonyString(networkId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not resume distributed hardware.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    HiSysEventWriteMsg(DHFWK_INIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "user resume sink ui.");
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->ResumeDistributedHardware(dhType, networkId);
}

int32_t DistributedHardwareFwkKit::StopDistributedHardware(DHType dhType, const std::string &networkId)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Stop distributed hardware dhType %{public}u, networkId %{public}s", (uint32_t)dhType,
        GetAnonyString(networkId).c_str());

    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not stop distributed hardware.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    HiSysEventWriteMsg(DHFWK_EXIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "user stop sink ui.");
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->StopDistributedHardware(dhType, networkId);
}

int32_t DistributedHardwareFwkKit::GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
    const sptr<IGetDhDescriptorsCallback> callback)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Get distributed hardware networkId %{public}s.", GetAnonyString(networkId).c_str());
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not get distributed hardware.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->GetDistributedHardware(networkId, enableStep, callback);
}

int32_t DistributedHardwareFwkKit::RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    DHLOGI("Register distributed hardware status sink listener.");
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not register distributed hardware status listener.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterDHStatusListener(listener);
}

int32_t DistributedHardwareFwkKit::UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    DHLOGI("Unregister distributed hardware status sink listener.");
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not unregister distributed hardware status listener.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->UnregisterDHStatusListener(listener);
}

int32_t DistributedHardwareFwkKit::RegisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Register distributed hardware status source listener %{public}s.", GetAnonyString(networkId).c_str());
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not register distributed hardware status listener.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->RegisterDHStatusListener(networkId, listener);
}

int32_t DistributedHardwareFwkKit::UnregisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Unregister distributed hardware status source listener %{public}s.", GetAnonyString(networkId).c_str());
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not unregister distributed hardware status listener.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->UnregisterDHStatusListener(networkId, listener);
}

int32_t DistributedHardwareFwkKit::EnableSink(const std::vector<DHDescriptor> &descriptors)
{
    DHLOGI("Enable distributed hardware sink descriptors %{public}s.", DumpDescriptors(descriptors).c_str());
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not enable sink.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->EnableSink(descriptors);
}

int32_t DistributedHardwareFwkKit::DisableSink(const std::vector<DHDescriptor> &descriptors)
{
    DHLOGI("Disable distributed hardware sink descriptors %{public}s.", DumpDescriptors(descriptors).c_str());
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not disable sink.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->DisableSink(descriptors);
}

int32_t DistributedHardwareFwkKit::EnableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Enable distributed hardware source networkId %{public}s, descriptors %{public}s.",
        GetAnonyString(networkId).c_str(), DumpDescriptors(descriptors).c_str());
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not enable source.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->EnableSource(networkId, descriptors);
}

int32_t DistributedHardwareFwkKit::DisableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    if (!IsIdLengthValid(networkId)) {
        return ERR_DH_FWK_PARA_INVALID;
    }
    DHLOGI("Disable distributed hardware source networkId %{public}s, descriptors %{public}s.",
        GetAnonyString(networkId).c_str(), DumpDescriptors(descriptors).c_str());
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not disable source.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->DisableSource(networkId, descriptors);
}

int32_t DistributedHardwareFwkKit::LoadDistributedHDF(const DHType dhType)
{
    DHLOGI("Load distributed HDF, dhType: %{public}u.", dhType);
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGE("DHFWK not online or get proxy failed, try to load DFWK service.");
        if (LoadDistributedHardwareSA() != DH_FWK_SUCCESS) {
            DHLOGE("Load distributed hardware SA failed, can not load distributed HDF.");
            return ERR_DH_FWK_POINTER_IS_NULL;
        }
    }
    auto proxy = DHFWKSAManager::GetInstance().GetDHFWKProxy();
    if (proxy == nullptr) {
        DHLOGE("DHFWK proxy is null, can not load distributed HDF.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return proxy->LoadDistributedHDF(dhType);
}

int32_t DistributedHardwareFwkKit::UnLoadDistributedHDF(const DHType dhType)
{
    DHLOGI("UnLoad distributed HDF, dhType: %{public}u.", dhType);
    if (DHFWKSAManager::GetInstance().GetDHFWKProxy() == nullptr) {
        DHLOGI("DHFWK not online or get proxy failed, can not unload distributed HDF.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return DHFWKSAManager::GetInstance().GetDHFWKProxy()->UnLoadDistributedHDF(dhType);
}

int32_t DistributedHardwareFwkKit::RegisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, int32_t timeOut, const std::string &pkgName)
{
    DHLOGI("Register hardware access listener");
    auto proxy = DHFWKSAManager::GetInstance().GetDHFWKProxy();
    if (proxy == nullptr) {
        DHLOGE("DHFWK proxy is null, can not load distributed HDF.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return proxy->RegisterHardwareAccessListener(dhType, callback, timeOut, pkgName);
}

int32_t DistributedHardwareFwkKit::UnregisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, const std::string &pkgName)
{
    DHLOGI("Unregister hardware access listener");
    auto proxy = DHFWKSAManager::GetInstance().GetDHFWKProxy();
    if (proxy == nullptr) {
        DHLOGE("DHFWK proxy is null, can not load distributed HDF.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    return proxy->UnregisterHardwareAccessListener(dhType, callback, pkgName);
}

void DistributedHardwareFwkKit::SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool granted)
{
    DHLOGI("Set authorization result");
    auto proxy = DHFWKSAManager::GetInstance().GetDHFWKProxy();
    if (proxy == nullptr) {
        DHLOGE("DHFWK proxy is null, can not load distributed HDF.");
        return;
    }
    proxy->SetAuthorizationResult(dhType, requestId, granted);
}
} // DistributedHardware
} // OHOS