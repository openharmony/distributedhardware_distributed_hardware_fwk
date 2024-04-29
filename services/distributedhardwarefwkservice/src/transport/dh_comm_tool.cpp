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
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"
#include "local_capability_info_manager.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DHCommTool"

DHCommTool::DHCommTool() : memberFuncMap_({}), dhTransportPtr_(nullptr)
{
    DHLOGI("Ctor DHCommTool");
}

void DHCommTool::Init()
{
    DHLOGI("Init DHCommTool");
    dhTransportPtr_ = std::make_shared<DHTransport>(shared_from_this());
    RegMemberFuncs();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    eventHandler_ = std::make_shared<DHCommTool::DHCommToolEventHandler>(runner);
}

void DHCommTool::RegMemberFuncs()
{
    memberFuncMap_[DH_COMM_REQ_FULL_ATTRS] = &DHCommTool::DealReqFullDHAttrs;
    memberFuncMap_[DH_COMM_RSP_FULL_ATTRS] = &DHCommTool::DealRspFullDHAttrs;
}

void DHCommTool::DealReqFullDHAttrs(const std::string &msg)
{

}

void DHCommTool::DealRspFullDHAttrs(const std::string &msg)
{

}

void DHCommTool::TriggerReqFullDHAttrs(const std::string &remoteNetworkId)
{
    DHLOGI("TriggerReqFullDHAttrs, remote networkId: %{public}s", GetAnonyString(remoteNetworkId).c_str());
    if (dhTransportPtr_ == nullptr) {
        DHLOGE("transport is null")
        return;
    }
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        DHLOGE("Get local network id error")
        return;
    }
    if (dhTransportPtr_->StartSocket(remoteNetworkId) != DH_FWK_SUCCESS) {
        DHLOGE("Start socket error");
        return;
    }
    CommMsg commMsg = {
        .code = DH_COMM_REQ_FULL_ATTRS,
        .msg = localNetworkId;
    };
    cJSON *root = cJSON_CreateObject();
    ToJson(root, commMsg);
    char *msgStr = cJSON_PrintUnformatted(root);
    std::string payload = std::string(msgStr);
    cJSON_free(msgStr);
    cJSON_Delete(root);
    int32_t ret = dhTransportPtr_->Send(remoteNetworkId, payload);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Trigger req remote full attrs error");
        return;
    }
    DHLOGI("Trigger req remote full attrs success");
}

DHCommTool::DHCommToolEventHandler : public AppExecFwk::EventHandler {
        public:
            DHCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
            ~DHCommToolEventHandler() override = default;
            void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    };
    std::shared_ptr<DHCommTool::DHCommToolEventHandler> GetEventHandler();


DHCommTool::DHCommToolEventHandler::DHCommToolEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner) : AppExecFwk::EventHandler(runner)
{
    DHLOGI("Ctor DHCommToolEventHandler");
}

void DHCommTool::DHCommToolEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    switch (eventId) {
        case DH_COMM_REQ_FULL_ATTRS:
            std::shared_ptr<CommMsg> commMsg = event->GetSharedObject<CommMsg>();
            GetAndSendLocalFullAttrs(commMsg->msg);
            break;
        case DH_COMM_RSP_FULL_ATTRS:
            std::shared_ptr<CommMsg> commMsg = event->GetSharedObject<CommMsg>();
            // parse remote rsp full attrs and save to local db
            FullCapsRsp capsRsp = ParseAndSaveRemoteDHAttrs(commMsg->msg);
            if (!capsRsp.networkId.empty()) {
                // TODO: after receive rsp, close dsoftbus channel
            }
            if (!capsRsp.caps.empty()) {
                // TODO: trigger register dh by full attrs
            }
            break;
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

void DHCommTool::GetAndSendLocalFullAttrs(const std::string &reqNetworkId)
{
    DHLOGI("GetAndSendLocalFullAttrs, reqNetworkId: %{public}s", GetAnonyString(reqNetworkId).c_str());
    std::string localDeviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(localDeviceId, resInfos);
    FullCapsRsp attrsRsp;
    attrsRsp.networkId = GetLocalNetworkId();
    attrsRsp.caps = resInfos;
    cJSON *root = cJSON_CreateObject();
    ToJson(root, attrsRsp);
    char *msg = cJSON_PrintUnformatted(root);
    std::string payload(msg);
    cJSON_Delete(root);

    if (dhTransportPtr_ == nullptr) {
        DHLOGE("transport is null")
        return;
    }

    int32_t ret = dhTransportPtr_->Send(reqNetworkId, payload);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Send back attrs failed, ret: %{public}d", ret);
        return;
    }
    DHLOGI("Send back attrs success");
}

FullCapsRsp DHCommTool::ParseAndSaveRemoteDHAttrs(const std::string &remoteAttrs)
{
    FullCapsRsp capsRsp;
    DHLOGI("ParseAndSaveRemoteDHAttrs enter");
    cJSON *root = cJSON_Parse(remoteAttrs.c_str());
    if (root == NULL) {
        DHLOGE("Parse remote attrs failed");
        return capsRsp;
    }

    FromJson(root, capsRsp);
    cJSON_Delete(root);
    int32_t ret = LocalCapabilityInfoManager::GetInstance().AddCapability(capsRsp.caps);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Save local capabilities error, ret: %{public}d", ret);
        return capsRsp;
    }
    DHLOGE("Save local capabilities success");
    return capsRsp;
}

std::shared_ptr<DHCommTool::DHCommToolEventHandler> DHCommTool::GetEventHandler()
{
    return this->eventHandler_;
}

} // DistributedHardware
} // OHOS