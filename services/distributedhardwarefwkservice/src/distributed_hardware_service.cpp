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

#include "distributed_hardware_service.h"

#include <cinttypes>

#include "constants.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "ipublisher_listener.h"
#include "iservice_registry.h"
#include "cJSON.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "access_manager.h"
#include "av_trans_control_center.h"
#include "capability_info_manager.h"
#include "component_manager.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "dh_utils_hisysevent.h"
#include "distributed_hardware_fwk_kit_paras.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager_factory.h"
#include "publisher.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareService"
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedHardwareService, DISTRIBUTED_HARDWARE_SA_ID, true);
namespace {
    constexpr int32_t INIT_BUSINESS_DELAY_TIME_MS = 5 * 100;
    const std::string INIT_TASK_ID = "CheckAndInitDH";
}

DistributedHardwareService::DistributedHardwareService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
}

void DistributedHardwareService::OnStart()
{
    DHLOGI("DistributedHardwareService::OnStart start");
    HiSysEventWriteMsg(DHFWK_INIT_BEGIN, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa start on demand.");

    if (state_ == ServiceRunningState::STATE_RUNNING) {
        DHLOGI("DistributedHardwareService has already started.");
        return;
    }
    if (!Init()) {
        DHLOGE("failed to init DistributedHardwareService");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    DHLOGI("DistributedHardwareService::OnStart start service success.");
}

bool DistributedHardwareService::Init()
{
    DHLOGI("DistributedHardwareService::Init ready to init.");
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("DistributedHardwareService::Init Publish failed!");
            HiSysEventWriteMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
                "dhfwk sa init publish failed.");
            return false;
        }
        registerToService_ = true;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    auto executeInnerFunc = [this] { DoBusinessInit(); };
    eventHandler_->PostTask(executeInnerFunc, INIT_TASK_ID, 0);
    DHLOGI("DistributedHardwareService::Init success.");
    HiSysEventWriteMsg(DHFWK_INIT_END, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "dhfwk sa init success.");
    return true;
}

void DistributedHardwareService::InitLocalDevInfo()
{
    DHLOGI("Init Local device info in DB");
    DistributedHardwareManagerFactory::GetInstance().InitLocalDevInfo();
}

bool DistributedHardwareService::DoBusinessInit()
{
    if (!IsDepSAStart()) {
        DHLOGW("Depend sa not start");
        auto executeInnerFunc = [this] { DoBusinessInit(); };
        eventHandler_->PostTask(executeInnerFunc, INIT_TASK_ID, INIT_BUSINESS_DELAY_TIME_MS);
        return false;
    }

    DHLOGI("Init AccessManager");
    auto ret = AccessManager::GetInstance()->Init();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("DistributedHardwareService::Init failed.");
        HiSysEventWriteErrCodeMsg(DHFWK_INIT_FAIL, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
            ret, "dhfwk sa AccessManager init fail.");
    }
    InitLocalDevInfo();
    AccessManager::GetInstance()->CheckTrustedDeviceOnline();
    return true;
}

bool DistributedHardwareService::IsDepSAStart()
{
    sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (saMgr == nullptr) {
        DHLOGE("Get System Ability Manager failed");
        return false;
    }
    DHLOGI("Check DSoftbus sa");
    sptr<IRemoteObject> remoteObject = saMgr->CheckSystemAbility(SOFTBUS_SERVER_SA_ID);
    if (remoteObject == nullptr) {
        DHLOGW("DSoftbus not start");
        return false;
    }
    DHLOGI("Check KVDB sa");
    remoteObject = saMgr->CheckSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID);
    if (remoteObject == nullptr) {
        DHLOGW("KVDB not start");
        return false;
    }
    DHLOGI("Check DM sa");
    remoteObject = saMgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID);
    if (remoteObject == nullptr) {
        DHLOGW("DM not start");
        return false;
    }
    return true;
}

void DistributedHardwareService::OnStop()
{
    DHLOGI("DistributedHardwareService::OnStop ready to stop service.");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
}

int32_t DistributedHardwareService::RegisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    Publisher::GetInstance().RegisterListener(topic, listener);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    Publisher::GetInstance().UnregisterListener(topic, listener);
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::PublishMessage(const DHTopic topic, const std::string &msg)
{
    Publisher::GetInstance().PublishMessage(topic, msg);
    return DH_FWK_SUCCESS;
}

std::string DistributedHardwareService::QueryLocalSysSpec(const QueryLocalSysSpecType spec)
{
    DeviceInfo localDevInfo = DHContext::GetInstance().GetDeviceInfo();
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(localDevInfo.deviceId, resInfos);
    DHType targetDhType = DHType::UNKNOWN;
    std::string targetKey = "";
    switch (spec) {
        case QueryLocalSysSpecType::HISTREAMER_AUDIO_ENCODER:
            targetKey = KEY_HISTREAMER_AUDIO_ENCODER;
            targetDhType = DHType::AUDIO;
            break;
        case QueryLocalSysSpecType::HISTREAMER_AUDIO_DECODER:
            targetKey = KEY_HISTREAMER_AUDIO_DECODER;
            targetDhType = DHType::AUDIO;
            break;
        case QueryLocalSysSpecType::HISTREAMER_VIDEO_ENCODER:
            targetKey = KEY_HISTREAMER_VIDEO_ENCODER;
            targetDhType = DHType::SCREEN;
            break;
        case QueryLocalSysSpecType::HISTREAMER_VIDEO_DECODER:
            targetKey = KEY_HISTREAMER_VIDEO_DECODER;
            targetDhType = DHType::SCREEN;
            break;
        default:
            break;
    }

    DHLOGE("QueryLocalSysSpec targetKey: %{public}s, targetDhType: %{public}" PRIu32, targetKey.c_str(),
        (uint32_t)targetDhType);
    if (targetDhType == DHType::UNKNOWN) {
        DHLOGE("Can not find matched dhtype");
        return "";
    }

    std::string attrs = "";
    for (const auto &cap : resInfos) {
        if (cap->GetDHType() != targetDhType) {
            continue;
        }
        attrs = cap->GetDHAttrs();
        break;
    }
    if (attrs.empty()) {
        DHLOGE("Can not find dh attrs");
        return "";
    }

    return QueryDhSysSpec(targetKey, attrs);
}

std::string DistributedHardwareService::QueryDhSysSpec(const std::string &targetKey, std::string &attrs)
{
    cJSON *attrJson = cJSON_Parse(attrs.c_str());
    if (attrJson == NULL) {
        DHLOGE("attrs json is invalid, attrs: %{public}s", attrs.c_str());
        return "";
    }
    if (!IsString(attrJson, targetKey)) {
        DHLOGE("Attrs Json not contains key: %{public}s", targetKey.c_str());
        cJSON_Delete(attrJson);
        return "";
    }
    std::string result = cJSON_GetObjectItem(attrJson, targetKey.c_str())->valuestring;
    cJSON_Delete(attrJson);
    return result;
}

int32_t DistributedHardwareService::InitializeAVCenter(const TransRole &transRole, int32_t &engineId)
{
    return AVTransControlCenter::GetInstance().InitializeAVCenter(transRole, engineId);
}

int32_t DistributedHardwareService::ReleaseAVCenter(int32_t engineId)
{
    return AVTransControlCenter::GetInstance().ReleaseAVCenter(engineId);
}

int32_t DistributedHardwareService::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    return AVTransControlCenter::GetInstance().CreateControlChannel(engineId, peerDevId);
}

int32_t DistributedHardwareService::NotifyAVCenter(int32_t engineId, const AVTransEvent &event)
{
    return AVTransControlCenter::GetInstance().NotifyAVCenter(engineId, event);
}

int32_t DistributedHardwareService::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAVTransControlCenterCallback> &callback)
{
    return AVTransControlCenter::GetInstance().RegisterCtlCenterCallback(engineId, callback);
}

int32_t DistributedHardwareService::NotifySourceRemoteSinkStarted(std::string &deviceId)
{
    DHLOGI("DistributedHardwareService NotifySourceRemoteSinkStarted Init DHMS Ready Start.");
    Publisher::GetInstance().PublishMessage(DHTopic::TOPIC_INIT_DHMS_READY, deviceId);
    DHLOGI("DistributedHardwareService NotifySourceRemoteSinkStarted Init DHMS Ready End.");
    return DH_FWK_SUCCESS;
}

int DistributedHardwareService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("DistributedHardwareService  Dump.");

    std::vector<std::string> argsStr {};
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    std::string result("");
    int ret = AccessManager::GetInstance()->Dump(argsStr, result);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Dump error, ret = %{public}d", ret);
    }

    if (dprintf(fd, "%s\n", result.c_str()) < 0) {
        DHLOGE("Hidump dprintf error");
        ret = ERR_DH_FWK_HIDUMP_DPRINTF_ERROR;
    }

    return ret;
}

int32_t DistributedHardwareService::PauseDistributedHardware(DHType dhType, const std::string &networkId)
{
    std::map<DHType, IDistributedHardwareSink*> sinkMap = ComponentManager::GetInstance().GetDHSinkInstance();
    if (sinkMap.find(dhType) == sinkMap.end()) {
        DHLOGE("PauseDistributedHardware for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = sinkMap[dhType]->PauseDistributedHardware(networkId);
    if (ret != 0) {
        DHLOGE("PauseDistributedHardware for DHType: %{public}u failed, ret: %{public}d", (uint32_t)dhType, ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::ResumeDistributedHardware(DHType dhType, const std::string &networkId)
{
    std::map<DHType, IDistributedHardwareSink*> sinkMap = ComponentManager::GetInstance().GetDHSinkInstance();
    if (sinkMap.find(dhType) == sinkMap.end()) {
        DHLOGE("ResumeDistributedHardware for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = sinkMap[dhType]->ResumeDistributedHardware(networkId);
    if (ret != 0) {
        DHLOGE("ResumeDistributedHardware for DHType: %{public}u failed, ret: %{public}d", (uint32_t)dhType, ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareService::StopDistributedHardware(DHType dhType, const std::string &networkId)
{
    std::map<DHType, IDistributedHardwareSink*> sinkMap = ComponentManager::GetInstance().GetDHSinkInstance();
    if (sinkMap.find(dhType) == sinkMap.end()) {
        DHLOGE("StopDistributedHardware for DHType: %{public}u not init sink handler", (uint32_t)dhType);
        return ERR_DH_FWK_PARA_INVALID;
    }
    int32_t ret = sinkMap[dhType]->StopDistributedHardware(networkId);
    if (ret != 0) {
        DHLOGE("StopDistributedHardware for DHType: %{public}u failed, ret: %{public}d", (uint32_t)dhType, ret);
        return ret;
    }
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
