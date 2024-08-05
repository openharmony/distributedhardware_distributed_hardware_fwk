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

#include "dh_comm_tool.h"

#include "cJSON.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "component_manager.h"
#include "dh_context.h"
#include "dh_transport_obj.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "local_capability_info_manager.h"
#include "task_executor.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DHCommTool"

DHCommTool::DHCommTool() : dhTransportPtr_(nullptr)
{
    DHLOGI("Ctor DHCommTool");
}

void DHCommTool::Init()
{
    DHLOGI("Init DHCommTool");
    dhTransportPtr_ = std::make_shared<DHTransport>(shared_from_this());
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DHCommTool::DHCommToolEventHandler>(runner, shared_from_this());
    dhTransportPtr_->Init();
}

void DHCommTool::UnInit()
{
    DHLOGI("UnInit DHCommTool");
    if (dhTransportPtr_ == nullptr) {
        DHLOGI("dhTransportPtr_ is null");
        return;
    }
    dhTransportPtr_->UnInit();
}

std::shared_ptr<DHCommTool> DHCommTool::GetInstance()
{
    static std::shared_ptr<DHCommTool> instance(new(std::nothrow) DHCommTool);
    if (instance == nullptr) {
        DHLOGE("instance is nullptr, because applying memory fail!");
        return nullptr;
    }
    return instance;
}

void DHCommTool::TriggerReqFullDHCaps(const std::string &remoteNetworkId)
{
    DHLOGI("TriggerReqFullDHCaps, remote networkId: %{public}s", GetAnonyString(remoteNetworkId).c_str());
    if (remoteNetworkId.empty() || dhTransportPtr_ == nullptr) {
        DHLOGE("remoteNetworkId or transport is null");
        return;
    }
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Get local network id error");
        return;
    }
    if (dhTransportPtr_->StartSocket(remoteNetworkId) != DH_FWK_SUCCESS) {
        DHLOGE("Start socket error");
        return;
    }
    CommMsg commMsg(DH_COMM_REQ_FULL_CAPS, localNetworkId);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dhTransportPtr_->Send(remoteNetworkId, payload);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Trigger req remote full attrs error");
        return;
    }
    DHLOGI("Trigger req remote full attrs success.");
}

void DHCommTool::GetAndSendLocalFullCaps(const std::string &reqNetworkId)
{
    DHLOGI("GetAndSendLocalFullCaps, reqNetworkId: %{public}s", GetAnonyString(reqNetworkId).c_str());
    if (dhTransportPtr_ == nullptr) {
        DHLOGE("transport is null");
        return;
    }
    std::string localDeviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(localDeviceId, resInfos);
    FullCapsRsp capsRsp;
    capsRsp.networkId = GetLocalNetworkId();
    capsRsp.caps = resInfos;
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        DHLOGE("Create cJSON object failed.");
        return;
    }
    ToJson(root, capsRsp);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return;
    }
    std::string fullCapsMsg(msg);
    cJSON_free(msg);
    cJSON_Delete(root);

    CommMsg commMsg(DH_COMM_RSP_FULL_CAPS, fullCapsMsg);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dhTransportPtr_->Send(reqNetworkId, payload);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Send back Caps failed, ret: %{public}d", ret);
        return;
    }
    DHLOGI("Send back Caps success");
}

FullCapsRsp DHCommTool::ParseAndSaveRemoteDHCaps(const std::string &remoteCaps)
{
    FullCapsRsp capsRsp;
    DHLOGI("ParseAndSaveRemoteDHCaps enter");
    cJSON *root = cJSON_Parse(remoteCaps.c_str());
    if (root == NULL) {
        DHLOGE("Parse remote Caps failed");
        return capsRsp;
    }

    FromJson(root, capsRsp);
    cJSON_Delete(root);
    int32_t ret = LocalCapabilityInfoManager::GetInstance()->AddCapability(capsRsp.caps);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Save local capabilities error, ret: %{public}d", ret);
        return capsRsp;
    }
    DHLOGE("Save local capabilities success");
    return capsRsp;
}

DHCommTool::DHCommToolEventHandler::DHCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
    std::shared_ptr<DHCommTool> dhCommToolPtr) : AppExecFwk::EventHandler(runner), dhCommToolWPtr_(dhCommToolPtr)
{
    DHLOGI("Ctor DHCommToolEventHandler");
}

void DHCommTool::DHCommToolEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    std::shared_ptr<CommMsg> commMsg = event->GetSharedObject<CommMsg>();
    if (commMsg == nullptr) {
        DHLOGE("ProcessEvent commMsg is null");
        return;
    }
    if (dhCommToolWPtr_.expired()) {
        DHLOGE("dhCommToolWPtr_ is expired");
        return;
    }
    std::shared_ptr<DHCommTool> dhCommToolPtr = dhCommToolWPtr_.lock();
    if (dhCommToolPtr == nullptr) {
        DHLOGE("dhCommToolPtr is null");
        return;
    }
    switch (eventId) {
        case DH_COMM_REQ_FULL_CAPS: {
            dhCommToolPtr->GetAndSendLocalFullCaps(commMsg->msg);
            break;
        }
        case DH_COMM_RSP_FULL_CAPS: {
            // parse remote rsp full attrs and save to local db
            FullCapsRsp capsRsp = dhCommToolPtr->ParseAndSaveRemoteDHCaps(commMsg->msg);
            DHLOGI("Receive full remote capabilities, remote networkid: %{public}s, caps size: %{public}" PRIu32,
                GetAnonyString(capsRsp.networkId).c_str(), static_cast<uint32_t>(capsRsp.caps.size()));
            ProcessFullCapsRsp(capsRsp, dhCommToolPtr);
            break;
        }
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

void DHCommTool::DHCommToolEventHandler::ProcessFullCapsRsp(const FullCapsRsp &capsRsp,
    const std::shared_ptr<DHCommTool> dhCommToolPtr)
{
    if (capsRsp.networkId.empty() || capsRsp.caps.empty()) {
        DHLOGE("Receive remote caps info invalid!");
        return;
    }
    // after receive rsp, close dsoftbus channel
    DHLOGI("we receive full remote capabilities, close channel, remote networkId: %{public}s",
        GetAnonyString(capsRsp.networkId).c_str());
    dhCommToolPtr->GetDHTransportPtr()->StopSocket(capsRsp.networkId);

    // trigger register dh by full attrs
    std::string uuid = DHContext::GetInstance().GetUUIDByNetworkId(capsRsp.networkId);
    if (uuid.empty()) {
        DHLOGE("Can not find remote device uuid by networkid: %{public}s", GetAnonyString(capsRsp.networkId).c_str());
        return;
    }

    for (auto const &cap : capsRsp.caps) {
        BusinessState curState = ComponentManager::GetInstance().QueryBusinessState(capsRsp.networkId, cap->GetDHId());
        DHLOGI("DH state: %{public}" PRIu32 ", networkId: %{public}s, dhId: %{public}s",
            (uint32_t)curState, GetAnonyString(capsRsp.networkId).c_str(), GetAnonyString(cap->GetDHId()).c_str());
        TaskParam taskParam = {
            .networkId = capsRsp.networkId,
            .uuid = uuid,
            .dhId = cap->GetDHId(),
            .dhType = cap->GetDHType()
        };
        if (curState != BusinessState::RUNNING && curState != BusinessState::PAUSING) {
            DHLOGI("The dh not busy, refresh it");
            auto task = TaskFactory::GetInstance().CreateTask(TaskType::ENABLE, taskParam, nullptr);
            TaskExecutor::GetInstance().PushTask(task);
        } else {
            DHLOGI("The dh busy, save and refresh after idle");
            ComponentManager::GetInstance().SaveNeedRefreshTask(taskParam);
        }
    }
}

std::shared_ptr<DHCommTool::DHCommToolEventHandler> DHCommTool::GetEventHandler()
{
    return this->eventHandler_;
}

const std::shared_ptr<DHTransport> DHCommTool::GetDHTransportPtr()
{
    return this->dhTransportPtr_;
}

} // DistributedHardware
} // OHOS