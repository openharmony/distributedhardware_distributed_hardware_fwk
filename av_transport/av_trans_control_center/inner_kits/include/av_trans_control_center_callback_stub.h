/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_AV_TRANS_CONTROL_CENTER_CALLBACK_STUB_H
#define OHOS_AV_TRANS_CONTROL_CENTER_CALLBACK_STUB_H

#include "iremote_stub.h"
#include "iav_trans_control_center_callback.h"

namespace OHOS {
namespace DistributedHardware {
class AVTransControlCenterCallbackStub : public IRemoteStub<IAVTransControlCenterCallback> {
public:
    AVTransControlCenterCallbackStub();
    virtual ~AVTransControlCenterCallbackStub() override;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t SetParameterInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetSharedMemoryInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyInner(MessageParcel &data, MessageParcel &reply);

private:
    DISALLOW_COPY_AND_MOVE(AVTransControlCenterCallbackStub);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANS_CONTROL_CENTER_CALLBACK_STUB_H
