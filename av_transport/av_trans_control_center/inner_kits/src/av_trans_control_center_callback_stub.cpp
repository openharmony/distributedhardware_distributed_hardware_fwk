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

#include "av_trans_control_center_callback_stub.h"

#include "av_trans_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
AVTransControlCenterCallbackStub::AVTransControlCenterCallbackStub()
{
}

AVTransControlCenterCallbackStub::~AVTransControlCenterCallbackStub()
{
}

int32_t AVTransControlCenterCallbackStub::OnRemoteRequest(uint32_t code,
    MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    (void)option;
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("Read valid token failed");
        return ERR_INVALID_DATA;
    }
    switch (code) {
        case (uint32_t)IAVTransControlCenterCallback::Message::SET_PARAMETER: {
            return SetParameterInner(data, reply);
        }
        case (uint32_t)IAVTransControlCenterCallback::Message::SET_SHARED_MEMORY: {
            return SetSharedMemoryInner(data, reply);
        }
        case (uint32_t)IAVTransControlCenterCallback::Message::NOTIFY_AV_EVENT: {
            return NotifyInner(data, reply);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return NO_ERROR;
}

int32_t AVTransControlCenterCallbackStub::SetParameterInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t transTag = data.ReadUint32();
    std::string tagValue = data.ReadString();
    int32_t ret = SetParameter((AVTransTag)transTag, tagValue);
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    return NO_ERROR;
}

int32_t AVTransControlCenterCallbackStub::SetSharedMemoryInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t fd = data.ReadFileDescriptor();
    int32_t size = data.ReadInt32();
    std::string name = data.ReadString();
    int32_t ret = SetSharedMemory(AVTransSharedMemory{ fd, size, name });
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    return NO_ERROR;
}

int32_t AVTransControlCenterCallbackStub::NotifyInner(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = data.ReadUint32();
    std::string content = data.ReadString();
    std::string peerDevId = data.ReadString();
    int32_t ret = Notify(AVTransEvent{(EventType)type, content, peerDevId});
    if (!reply.WriteInt32(ret)) {
        DHLOGE("Write ret code failed");
        return ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL;
    }
    return NO_ERROR;
}
} // namespace DistributedHardware
} // namespace OHOS