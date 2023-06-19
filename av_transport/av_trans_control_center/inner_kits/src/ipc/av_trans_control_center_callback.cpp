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

#include "av_trans_control_center_callback.h"

#include "av_trans_errno.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t AVTransControlCenterCallback::SetParameter(AVTransTag tag, const std::string &value)
{
    AVTRANS_LOGW("AVTransControlCenterCallback::SetParameter enter.");
    if ((tag == AVTransTag::START_AV_SYNC) || (tag == AVTransTag::STOP_AV_SYNC) ||
        (tag == AVTransTag::TIME_SYNC_RESULT)) {
        std::shared_ptr<IAVReceiverEngine> rcvEngine = receiverEngine_.lock();
        TRUE_RETURN_V_MSG_E(rcvEngine == nullptr, ERR_DH_AVT_NULL_POINTER, "receiver engine is nullptr.");
        rcvEngine->SetParameter(tag, value);
    }
    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenterCallback::SetSharedMemory(const AVTransSharedMemory &memory)
{
    AVTRANS_LOGW("AVTransControlCenterCallback::SetSharedMemory enter.");

    std::shared_ptr<IAVSenderEngine> sendEngine = senderEngine_.lock();
    if (sendEngine != nullptr) {
        sendEngine->SetParameter(AVTransTag::SHARED_MEMORY_FD, MarshalSharedMemory(memory));
    }

    std::shared_ptr<IAVReceiverEngine> rcvEngine = receiverEngine_.lock();
    if (rcvEngine != nullptr) {
        rcvEngine->SetParameter(AVTransTag::SHARED_MEMORY_FD, MarshalSharedMemory(memory));
    }

    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenterCallback::Notify(const AVTransEvent& event)
{
    AVTRANS_LOGW("AVTransControlCenterCallback::Notify enter.");
    return DH_AVT_SUCCESS;
}

void AVTransControlCenterCallback::SetSenderEngine(const std::shared_ptr<IAVSenderEngine> &sender)
{
    TRUE_RETURN(sender == nullptr, "input sender engine is nullptr.");
    senderEngine_ = sender;
}

void AVTransControlCenterCallback::SetReceiverEngine(const std::shared_ptr<IAVReceiverEngine> &receiver)
{
    TRUE_RETURN(receiver == nullptr, "input receiver engine is nullptr.");
    receiverEngine_ = receiver;
}
}
}