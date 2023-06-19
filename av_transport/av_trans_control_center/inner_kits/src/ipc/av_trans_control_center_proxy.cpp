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

#include "av_trans_control_center_proxy.h"

#include <unordered_set>
#include "nlohmann/json.hpp"
#include "parcel.h"

#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "dhardware_ipc_interface_code.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AVTransControlCenterProxy::Initialize(const TransRole &transRole, int32_t &engineId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        AVTRANS_LOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AVTRANS_LOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)transRole)) {
        AVTRANS_LOGE("Write trans role failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::INIT_CTL_CEN), data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }
    engineId = reply.ReadInt32();

    return reply.ReadInt32();
}

int32_t AVTransControlCenterProxy::Release(int32_t engineId)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        AVTRANS_LOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AVTRANS_LOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        AVTRANS_LOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::RELEASE_CTL_CEN), data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t AVTransControlCenterProxy::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    TRUE_RETURN_V_MSG_E(peerDevId.empty(), ERR_DH_AVT_INVALID_PARAM_VALUE, "input peerDevId is empty");

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        AVTRANS_LOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AVTRANS_LOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        AVTRANS_LOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(peerDevId)) {
        AVTRANS_LOGE("Write peerDevId failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::CREATE_CTL_CEN_CHANNEL),
        data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t AVTransControlCenterProxy::Notify(int32_t engineId, const AVTransEvent& event)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        AVTRANS_LOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AVTRANS_LOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        AVTRANS_LOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteUint32((uint32_t)event.type)) {
        AVTRANS_LOGE("Write event type failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(event.content)) {
        AVTRANS_LOGE("Write event content failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(event.peerDevId)) {
        AVTRANS_LOGE("Write event peerDevId failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_AV_EVENT), data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t AVTransControlCenterProxy::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAVTransControlCenterCallback> &callback)
{
    TRUE_RETURN_V_MSG_E((callback == nullptr), ERR_DH_AVT_INVALID_PARAM_VALUE, "av control center callback is null");

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        AVTRANS_LOGE("remote service is null");
        return ERR_DH_AVT_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        AVTRANS_LOGE("WriteInterfaceToken fail!");
        return ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteInt32(engineId)) {
        AVTRANS_LOGE("Write engine id failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(callback->AsObject())) {
        AVTRANS_LOGE("Write callback failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DHMsgInterfaceCode::REGISTER_CTL_CEN_CALLBACK),
        data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t AVTransControlCenterProxy::RegisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenterProxy::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenterProxy::PublishMessage(const DHTopic topic, const std::string &msg)
{
    return DH_AVT_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS