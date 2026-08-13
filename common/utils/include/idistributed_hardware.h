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

#ifndef OHOS_IDISTRIBUTED_HARDWARE_H
#define OHOS_IDISTRIBUTED_HARDWARE_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "iremote_broker.h"
#include "device_type.h"
#include "dhardware_descriptor.h"
#include "iauthorization_result_callback.h"
#include "iget_dh_descriptors_callback.h"
#include "ihardware_status_listener.h"

#include "distributed_hardware_fwk_kit_paras.h"
#include "iav_trans_control_center_callback.h"
#include "ipublisher_listener.h"

namespace OHOS {
namespace DistributedHardware {
class IDistributedHardware : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.distributedhardware.accessToken");
    IDistributedHardware() = default;
    virtual ~IDistributedHardware() = default;
    virtual int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) = 0;
    virtual int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) = 0;
    virtual int32_t PublishMessage(const DHTopic topic, const std::string &msg) = 0;
    virtual std::string QueryLocalSysSpec(QueryLocalSysSpecType spec) = 0;

    virtual int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId) = 0;
    virtual int32_t ReleaseAVCenter(int32_t engineId) = 0;
    virtual int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId) = 0;
    virtual int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event) = 0;
    virtual int32_t RegisterCtlCenterCallback(int32_t engineId,
        const sptr<IAvTransControlCenterCallback> callback) = 0;
    virtual int32_t NotifySourceRemoteSinkStarted(std::string &deviceId) = 0;
    virtual int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId) = 0;
    virtual int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId) = 0;
    virtual int32_t StopDistributedHardware(DHType dhType, const std::string &networkId) = 0;
    virtual int32_t GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
        const sptr<IGetDhDescriptorsCallback> callback) = 0;
    virtual int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener) = 0;
    virtual int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener) = 0;
    virtual int32_t RegisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener) = 0;
    virtual int32_t UnregisterDHStatusListener(const std::string &networkId,
        sptr<IHDSourceStatusListener> listener) = 0;
    virtual int32_t EnableSink(const std::vector<DHDescriptor> &descriptors) = 0;
    virtual int32_t DisableSink(const std::vector<DHDescriptor> &descriptors) = 0;
    virtual int32_t EnableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) = 0;
    virtual int32_t DisableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) = 0;
    virtual int32_t LoadDistributedHDF(const DHType dhType) = 0;
    virtual int32_t UnLoadDistributedHDF(const DHType dhType) = 0;
    virtual int32_t LoadSinkDMSDPService(const std::string &udid) = 0;
    virtual int32_t NotifySinkRemoteSourceStarted(const std::string &udid) = 0;
    virtual int32_t RegisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        int32_t &timeOut, const std::string &pkgName) = 0;
    virtual int32_t UnregisterHardwareAccessListener(const DHType dhType,
        sptr<IAuthorizationResultCallback> callback, const std::string &pkgName) = 0;
    virtual void SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool &granted) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IDISTRIBUTED_HARDWARE_H
