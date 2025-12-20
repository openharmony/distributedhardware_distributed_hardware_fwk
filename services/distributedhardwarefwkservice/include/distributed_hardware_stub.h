/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_STUB_H
#define OHOS_DISTRIBUTED_HARDWARE_STUB_H

#include "iremote_stub.h"

#include "idistributed_hardware.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareStub : public IRemoteStub<IDistributedHardware> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t RegisterPublisherListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterPublisherListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t PublishMessageInner(MessageParcel &data, MessageParcel &reply);
    int32_t QueryLocalSysSpecInner(MessageParcel &data, MessageParcel &reply);

    int32_t InitializeAVCenterInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReleaseAVCenterInner(MessageParcel &data, MessageParcel &reply);
    int32_t CreateControlChannelInner(MessageParcel &data, MessageParcel &reply);
    int32_t NotifyAVCenterInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterControlCenterCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifySourceRemoteSinkStarted(MessageParcel &data, MessageParcel &reply);
    int32_t PauseDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t ResumeDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t StopDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterDHStatusSinkListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterDHStatusSinkListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterDHStatusSourceListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterDHStatusSourceListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableSinkInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableSinkInner(MessageParcel &data, MessageParcel &reply);
    int32_t EnableSourceInner(MessageParcel &data, MessageParcel &reply);
    int32_t DisableSourceInner(MessageParcel &data, MessageParcel &reply);
    int32_t LoadDistributedHDFInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnLoadDistributedHDFInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleLoadSinkDMSDPService(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifySinkRemoteSourceStarted(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterHardwareAccessListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterHardwareAccessListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetAuthorizationResultInner(MessageParcel &data, MessageParcel &reply);

    int32_t ReadDescriptors(MessageParcel &data, std::vector<DHDescriptor> &descriptors);
    int32_t WriteDescriptors(MessageParcel &data, const std::vector<DHDescriptor> &descriptors);
    int32_t OnRemoteRequestEx(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
    int32_t OnRemoteRequestRPC(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);

    bool ValidTopic(uint32_t topic);
    bool ValidQueryLocalSpec(uint32_t spec);
    bool HasAccessDHPermission();
    bool IsSystemHap();
    bool IsNativeSA();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_STUB_H
