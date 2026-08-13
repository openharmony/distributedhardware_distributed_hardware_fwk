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

#ifndef OHOS_DISTRIBUTED_HARDWARE_PROXY_H
#define OHOS_DISTRIBUTED_HARDWARE_PROXY_H

#include <cstdint>

#include "distributed_hardware_fwk_kit_paras.h"
#include "iremote_proxy.h"
#include "refbase.h"
#include "idistributed_hardware.h"
#include "publisher_listener_stub.h"

namespace OHOS {
namespace DistributedHardware {
class IHdfDeathListener : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedHardware.DistributedHardwareFwk.IHdfDeathListener");
};
class HdfDeathListenerStub : public IRemoteStub<IHdfDeathListener> {
public:
};
class DistributedHardwareProxy : public IRemoteProxy<IDistributedHardware> {
public:
    explicit DistributedHardwareProxy(const sptr<IRemoteObject> impl)
        : IRemoteProxy<IDistributedHardware>(impl)
    {
        hdfDeathListenerStub_ = sptr<HdfDeathListenerStub>(new HdfDeathListenerStub());
    }

    virtual ~DistributedHardwareProxy() {}
    int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) override;
    int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) override;
    int32_t PublishMessage(const DHTopic topic, const std::string &msg) override;
    std::string QueryLocalSysSpec(QueryLocalSysSpecType spec) override;

    int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId) override;
    int32_t ReleaseAVCenter(int32_t engineId) override;
    int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId) override;
    int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event) override;
    int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAvTransControlCenterCallback> callback) override;
    int32_t NotifySourceRemoteSinkStarted(std::string &deviceId) override;
    int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t StopDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
        const sptr<IGetDhDescriptorsCallback> callback) override;
    int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener) override;
    int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener) override;
    int32_t RegisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener) override;
    int32_t UnregisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener) override;
    int32_t EnableSink(const std::vector<DHDescriptor> &descriptors) override;
    int32_t DisableSink(const std::vector<DHDescriptor> &descriptors) override;
    int32_t EnableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) override;
    int32_t DisableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) override;
    int32_t LoadDistributedHDF(const DHType dhType) override;
    int32_t UnLoadDistributedHDF(const DHType dhType) override;
    int32_t LoadSinkDMSDPService(const std::string &udid) override;
    int32_t NotifySinkRemoteSourceStarted(const std::string &udid) override;
    int32_t RegisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        int32_t &timeOut, const std::string &pkgName) override;
    int32_t UnregisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        const std::string &pkgName) override;
    void SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool &granted) override;

private:
    int32_t ReadDescriptors(MessageParcel &data, std::vector<DHDescriptor> &descriptors);
    int32_t WriteDescriptors(MessageParcel &data, const std::vector<DHDescriptor> &descriptors);

private:
    static inline BrokerDelegator<DistributedHardwareProxy> delegator_;
    sptr<HdfDeathListenerStub> hdfDeathListenerStub_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_PROXY_H
