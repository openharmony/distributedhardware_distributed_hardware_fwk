/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "av_trans_control_center_kit.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "av_trans_control_center_proxy.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(AVTransControlCenterKit);
AVTransControlCenterKit::AVTransControlCenterKit() : ctlCenterProxy_(nullptr)
{
    DHLOGI("Ctor AVTransControlCenterKit");
}

AVTransControlCenterKit::~AVTransControlCenterKit()
{
    DHLOGI("Dtor AVTransControlCenterKit");
    ctlCenterProxy_ = nullptr;
}

int32_t AVTransControlCenterKit::Initialize(const TransRole &transRole, int32_t &engineId)
{
    DHLOGI("Initialize av control center enter.");

    sptr<IDistributedHardware> proxy = GetAVTransCtlCenterProxy();
    TRUE_RETURN_V_MSG_E((proxy == nullptr), ERR_DH_AVT_NULL_POINTER, "av control center sa proxy is null.");

    int32_t ret = proxy->Initialize(transRole, engineId);
    DHLOGI("Initialize av control center finish, ret: %" PRId32, ret);

    return ret;
}

int32_t AVTransControlCenterKit::Release(int32_t engineId)
{
    DHLOGI("Release av control center enter.");

    sptr<IDistributedHardware> proxy = GetAVTransCtlCenterProxy();
    TRUE_RETURN_V_MSG_E((proxy == nullptr), ERR_DH_AVT_NULL_POINTER, "av control center sa proxy is null.");

    int32_t ret = proxy->Release(engineId);
    DHLOGI("Release av control center finish, ret: %" PRId32, ret);

    return ret;
}

int32_t AVTransControlCenterKit::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    DHLOGI("Create av control center channel for peerDevId=%s.", GetAnonyString(peerDevId).c_str());
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM_VALUE, "input peerDevId is empty");

    sptr<IDistributedHardware> proxy = GetAVTransCtlCenterProxy();
    TRUE_RETURN_V_MSG_E((proxy == nullptr), ERR_DH_AVT_NULL_POINTER, "av control center sa proxy is null.");

    int32_t ret = proxy->CreateControlChannel(engineId, peerDevId);
    DHLOGI("Create av control center channel finish, ret: %" PRId32, ret);

    return ret;
}

int32_t AVTransControlCenterKit::Notify(int32_t engineId, const AVTransEvent &event)
{
    DHLOGI("Notify av control center enter. event type=%" PRId32, event.type);

    sptr<IDistributedHardware> proxy = GetAVTransCtlCenterProxy();
    TRUE_RETURN_V_MSG_E((proxy == nullptr), ERR_DH_AVT_NULL_POINTER, "av control center sa proxy is null.");

    int32_t ret = proxy->Notify(engineId, event);
    DHLOGI("Notify av control center finish, ret: %" PRId32, ret);

    return ret;
}

int32_t AVTransControlCenterKit::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAVTransControlCenterCallback> &callback)
{
    DHLOGI("Register av control center callback enter.");

    sptr<IDistributedHardware> proxy = GetAVTransCtlCenterProxy();
    TRUE_RETURN_V_MSG_E((proxy == nullptr), ERR_DH_AVT_NULL_POINTER, "av control center sa proxy is null.");

    int32_t ret = proxy->RegisterCtlCenterCallback(engineId, callback);
    DHLOGI("Register av control center callback finish, ret: %" PRId32, ret);

    return ret;
}

sptr<IDistributedHardware> AVTransControlCenterKit::GetAVTransCtlCenterProxy()
{
    std::lock_guard<std::mutex> lock(proxyMutex_);
    if (ctlCenterProxy_ == nullptr) {
        sptr<ISystemAbilityManager> saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saMgr == nullptr) {
            DHLOGE("Get System Ability Manager failed");
            return nullptr;
        }
        DHLOGI("Try get AVTransControlCenter sa");
        sptr<IRemoteObject> remoteObject = saMgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_SA_ID);
        if (remoteObject == nullptr) {
            DHLOGE("Get AVTransControlCenter proxy return null");
            return nullptr;
        }

        ctlCenterProxy_ = iface_cast<IDistributedHardware>(remoteObject);
        if (!ctlCenterProxy_ || !ctlCenterProxy_->AsObject()) {
            DHLOGE("Failed to Get AVTransControlCenter Proxy");
            return nullptr;
        }
    }
    return ctlCenterProxy_;
}
} // DistributedHardware
} // OHOS