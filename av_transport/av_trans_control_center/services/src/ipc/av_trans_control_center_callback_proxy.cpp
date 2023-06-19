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

#include "av_trans_control_center_callback_proxy.h"

#include <unordered_set>
#include "nlohmann/json.hpp"
#include "parcel.h"

#include "av_trans_constants.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "av_trans_types.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AVTransControlCenterCallbackProxy::SetParameter(AVTransTag tag, const std::string& value)
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
    if (!data.WriteUint32((uint32_t)tag)) {
        AVTRANS_LOGE("Write tag failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(value)) {
        AVTRANS_LOGE("Write value failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest((uint32_t)IAVTransControlCenterCallback::Message::SET_PARAMETER,
        data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t AVTransControlCenterCallbackProxy::SetSharedMemory(const AVTransSharedMemory &memory)
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
    if (!data.WriteFileDescriptor(memory.fd)) {
        AVTRANS_LOGE("Write memory fd failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteInt32(memory.size)) {
        AVTRANS_LOGE("Write memory size failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(memory.name)) {
        AVTRANS_LOGE("Write memory name failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest((uint32_t)IAVTransControlCenterCallback::Message::SET_SHARED_MEMORY,
        data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}

int32_t AVTransControlCenterCallbackProxy::Notify(const AVTransEvent& event)
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
    int32_t ret = remote->SendRequest((uint32_t)IAVTransControlCenterCallback::Message::NOTIFY_AV_EVENT,
        data, reply, option);
    if (ret != NO_ERROR) {
        AVTRANS_LOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    return reply.ReadInt32();
}
} // namespace DistributedHardware
} // namespace OHOS