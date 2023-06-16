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

#ifndef OHOS_AV_TRANS_CONTROL_CENTER_CALLBACK_H
#define OHOS_AV_TRANS_CONTROL_CENTER_CALLBACK_H

#include "av_trans_control_center_callback_stub.h"

#include "i_av_receiver_engine.h"
#include "i_av_sender_engine.h"

namespace OHOS {
namespace DistributedHardware {
class AVTransControlCenterCallback : public AVTransControlCenterCallbackStub {
public:
    AVTransControlCenterCallback() = default;
    ~AVTransControlCenterCallback() override = default;

    int32_t SetParameter(AVTransTag tag, const std::string &value) override;
    int32_t SetSharedMemory(const AVTransSharedMemory &memory) override;
    int32_t Notify(const AVTransEvent &event) override;

    void SetSenderEngine(const std::shared_ptr<IAVSenderEngine> &sender);
    void SetReceiverEngine(const std::shared_ptr<IAVReceiverEngine> &receiver);

private:
    std::weak_ptr<IAVSenderEngine> senderEngine_;
    std::weak_ptr<IAVReceiverEngine> receiverEngine_;
};
}
}
#endif