/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "device_manager.h"
#include "ipc_skeleton.h"
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#include "token_setproc.h"

#include "anonymous_string.h"
#include "capability_info_manager.h"
#include "component_manager.h"
#include "dh_context.h"
#include "dh_transport_obj.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "local_capability_info_manager.h"
#include "meta_info_manager.h"
#include "task_executor.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DHCommTool"

// request remote dh send back full dh capabilities
constexpr int32_t DH_COMM_REQ_FULL_CAPS = 1;
// send back full dh attributes to the requester
constexpr int32_t DH_COMM_RSP_FULL_CAPS = 2;

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
    static std::shared_ptr<DHCommTool> instance = std::make_shared<DHCommTool>();
    return instance;
}

bool DHCommTool::CheckCallerAclRight(const std::string &localNetworkId, const std::string &remoteNetworkId)
{
    if (!GetOsAccountInfo()) {
        DHLOGE("GetOsAccountInfo failed.");
        return false;
    }
    tokenId_ = IPCSkeleton::GetCallingTokenID();
    DmAccessCaller dmSrcCaller = {
        .accountId = accountId_,
        .pkgName = DH_FWK_PKG_NAME,
        .networkId = localNetworkId,
        .userId = userId_,
        .tokenId = tokenId_,
    };
    DmAccessCallee dmDstCallee = {
        .networkId = remoteNetworkId,
    };
    DHLOGI("CheckAclRight dmSrcCaller localNetworkId: %{public}s, accountId: %{public}s, remoteNetworkId: %{public}s",
        GetAnonyString(localNetworkId).c_str(), GetAnonyString(accountId_).c_str(),
        GetAnonyString(remoteNetworkId).c_str());
    if (!DeviceManager::GetInstance().CheckSrcAccessControl(dmSrcCaller, dmDstCallee)) {
        DHLOGE("Caller ACL check failed.");
        return false;
    }
    return true;
}

bool DHCommTool::GetOsAccountInfo()
{
    DHLOGI("GetOsAccountInfo start.");
#ifdef OS_ACCOUNT_PART
    std::vector<int32_t> ids;
    int32_t ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != DH_FWK_SUCCESS || ids.empty()) {
        DHLOGE("Get userId from active os accountIds fail, ret: %{public}d", ret);
        return false;
    }
    userId_ = ids[0];

    AccountSA::OhosAccountInfo osAccountInfo;
    ret = AccountSA::OhosAccountKits::GetInstance().GetOhosAccountInfo(osAccountInfo);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Get accountId from ohos account info fail, ret: %{public}d", ret);
        return false;
    }
    accountId_ = osAccountInfo.uid_;
#endif
    return true;
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
    if (!CheckCallerAclRight(localNetworkId, remoteNetworkId)) {
        DHLOGE("ACL check failed.");
        return;
    }
    if (dhTransportPtr_->StartSocket(remoteNetworkId) != DH_FWK_SUCCESS) {
        DHLOGE("Start socket error");
        return;
    }
    CommMsg commMsg(DH_COMM_REQ_FULL_CAPS, userId_, tokenId_, localNetworkId, accountId_, true);
    std::string payload = GetCommMsgString(commMsg);

    int32_t ret = dhTransportPtr_->Send(remoteNetworkId, payload);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Trigger req remote full attrs error");
        return;
    }
    DHLOGI("Trigger req remote full attrs success.");
}

std::string DHCommTool::GetLocalFullCapsInfo(bool isSyncMeta)
{
    DHLOGI("get local cap info start");
    std::string localDeviceId = DHContext::GetInstance().GetDeviceInfo().deviceId;
    std::vector<std::shared_ptr<CapabilityInfo>> localFullCapInfos;
    CapabilityInfoManager::GetInstance()->GetCapabilitiesByDeviceId(localDeviceId, localFullCapInfos);
    FullCapsRsp capsRsp;
    capsRsp.networkId = GetLocalNetworkId();
    capsRsp.caps = localFullCapInfos;
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        DHLOGE("Create cJSON object failed.");
        return "";
    }
    ToJson(root, capsRsp, isSyncMeta);
    char *msg = cJSON_PrintUnformatted(root);
    if (msg == nullptr) {
        cJSON_Delete(root);
        return "";
    }
    std::string fullCapsMsg(msg);
    cJSON_free(msg);
    cJSON_Delete(root);
    return fullCapsMsg;
}

std::string DHCommTool::GetLocalFullMetaCapsInfo(bool isSyncMeta)
{
    DHLOGI("get local metaCap info start");
    std::string localUdidHash = DHContext::GetInstance().GetDeviceInfo().udidHash;
    std::vector<std::shared_ptr<MetaCapabilityInfo>> lcoalFullMetaCapInfos;
    MetaInfoManager::GetInstance()->GetMetaCapInfosByUdidHash(localUdidHash, lcoalFullMetaCapInfos);
    FullCapsRsp capsRsp;
    capsRsp.networkId = GetLocalNetworkId();
    capsRsp.metaCaps = lcoalFullMetaCapInfos;
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        DHLOGE("Create cJSON object failed.");
        return "";
    }
    ToJson(root, capsRsp, isSyncMeta);
    char *fullMetaInfo = cJSON_PrintUnformatted(root);
    if (fullMetaInfo == nullptr) {
        cJSON_Delete(root);
        return "";
    }
    std::string fullMetaCaps(fullMetaInfo);
    cJSON_free(fullMetaInfo);
    cJSON_Delete(root);
    return fullMetaCaps;
}

void DHCommTool::GetAndSendLocalFullCaps(const std::string &reqNetworkId, bool isSyncMeta)
{
    DHLOGI("GetAndSendLocalFullCaps, reqNetworkId: %{public}s", GetAnonyString(reqNetworkId).c_str());
    if (dhTransportPtr_ == nullptr) {
        DHLOGE("transport is null");
        return;
    }
    std::string localFullInfo;
    if (isSyncMeta) {
        localFullInfo = GetLocalFullMetaCapsInfo(isSyncMeta);
    } else {
        localFullInfo = GetLocalFullCapsInfo(isSyncMeta);
    }
    if (localFullInfo.empty()) {
        DHLOGE("Get lcoal full device info failed.");
        return;
    }
    CommMsg commMsg;
    commMsg.code = DH_COMM_RSP_FULL_CAPS;
    commMsg.msg = localFullInfo;
    commMsg.isSyncMeta = true;
    std::string payload = GetCommMsgString(commMsg);
    int32_t ret = dhTransportPtr_->Send(reqNetworkId, payload);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Send back Caps failed, ret: %{public}d", ret);
        return;
    }
    DHLOGI("Send back Caps success");
}

FullCapsRsp DHCommTool::ParseAndSaveRemoteDHCaps(const std::string &remoteCaps, bool isSyncMeta)
{
    FullCapsRsp capsRsp;
    DHLOGI("Receive remote device full capinfos, parse and save remote device capinfos.");
    cJSON *root = cJSON_Parse(remoteCaps.c_str());
    if (root == NULL) {
        DHLOGE("Parse remote Caps failed");
        return capsRsp;
    }

    FromJson(root, capsRsp, isSyncMeta);
    cJSON_Delete(root);
    if (isSyncMeta) {
        int32_t ret = MetaInfoManager::GetInstance()->AddMetaCapInfos(capsRsp.metaCaps);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("Save remote device metaCapInfos error, ret: %{public}d", ret);
            return capsRsp;
        }
    } else {
        int32_t ret = LocalCapabilityInfoManager::GetInstance()->AddCapability(capsRsp.caps);
        if (ret != DH_FWK_SUCCESS) {
            DHLOGE("Save remote device capabilities error, ret: %{public}d", ret);
            return capsRsp;
        }
    }
    DHLOGI("Save remote device capinfo success");
    return capsRsp;
}

DHCommTool::DHCommToolEventHandler::DHCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner,
    std::shared_ptr<DHCommTool> dhCommToolPtr) : AppExecFwk::EventHandler(runner), dhCommToolWPtr_(dhCommToolPtr)
{
    DHLOGI("Ctor DHCommToolEventHandler");
}

void DHCommTool::DHCommToolEventHandler::ProcessEvent(
    const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        DHLOGE("event is null.");
        return;
    }
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
            dhCommToolPtr->GetAndSendLocalFullCaps(commMsg->msg, commMsg->isSyncMeta);
            break;
        }
        case DH_COMM_RSP_FULL_CAPS: {
            FullCapsRsp capsRsp = dhCommToolPtr->ParseAndSaveRemoteDHCaps(commMsg->msg, commMsg->isSyncMeta);
            ProcessFullCapsRsp(capsRsp, dhCommToolPtr, commMsg->isSyncMeta);
            break;
        }
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}

void DHCommTool::DHCommToolEventHandler::ProcessFullCapsRsp(const FullCapsRsp &capsRsp,
    const std::shared_ptr<DHCommTool> dhCommToolPtr, bool isSyncMeta)
{
    if (capsRsp.networkId.empty()) {
        DHLOGE("Receive remote caps info invalid!");
        return;
    }
    if (dhCommToolPtr == nullptr) {
        DHLOGE("dhCommToolPtr is null");
        return;
    }
    if (dhCommToolPtr->GetDHTransportPtr() == nullptr) {
        DHLOGE("Can not get Transport ptr");
        return;
    }
    // after receive rsp, close dsoftbus channel
    DHLOGI("receive full remote capabilities, close channel, remote networkId: %{public}s",
        GetAnonyString(capsRsp.networkId).c_str());
    dhCommToolPtr->GetDHTransportPtr()->StopSocket(capsRsp.networkId);

    // Return remote capInfos to the business
    std::vector<DHDescriptor> descriptors;
    if (isSyncMeta) {
        for (auto const &metaCap : capsRsp.metaCaps) {
            if (metaCap == nullptr) {
                continue;
            }
            DHDescriptor descriptor;
            descriptor.id = metaCap->GetDHId();
            descriptor.dhType = metaCap->GetDHType();
            descriptors.push_back(descriptor);
        }
    } else {
        for (auto const &cap : capsRsp.caps) {
            if (cap == nullptr) {
                continue;
            }
            DHDescriptor descriptor;
            descriptor.id = cap->GetDHId();
            descriptor.dhType = cap->GetDHType();
            descriptors.push_back(descriptor);
        }
    }
    ComponentManager::GetInstance().OnGetDescriptors(capsRsp.networkId, descriptors);
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