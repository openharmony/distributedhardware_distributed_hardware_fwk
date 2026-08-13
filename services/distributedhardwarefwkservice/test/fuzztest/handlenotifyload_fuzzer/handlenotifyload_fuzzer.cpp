/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "handlenotifyload_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "distributed_hardware_stub.h"
#include "dhardware_ipc_interface_code.h"

namespace OHOS {
namespace DistributedHardware {

class DistributedHardwareStubImpl : public DistributedHardwareStub {
public:
    DistributedHardwareStubImpl() = default;
    ~DistributedHardwareStubImpl() override {}

    int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) override
    {
        return 0;
    }
    
    int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) override
    {
        return 0;
    }
    
    int32_t PublishMessage(const DHTopic topic, const std::string &msg) override
    {
        return 0;
    }
    
    std::string QueryLocalSysSpec(QueryLocalSysSpecType spec) override
    {
        return "fuzz";
    }
    
    int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId) override
    {
        return 0;
    }
    
    int32_t ReleaseAVCenter(int32_t engineId) override
    {
        return 0;
    }
    
    int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId) override
    {
        return 0;
    }
    
    int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event) override
    {
        return 0;
    }
    
    int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAvTransControlCenterCallback> callback) override
    {
        return 0;
    }
    
    int32_t NotifySourceRemoteSinkStarted(std::string &deviceId) override
    {
        return 0;
    }
    
    int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId) override
    {
        return 0;
    }
    
    int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId) override
    {
        return 0;
    }
    
    int32_t StopDistributedHardware(DHType dhType, const std::string &networkId) override
    {
        return 0;
    }
    
    int32_t GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
        const sptr<IGetDhDescriptorsCallback> callback) override
    {
        return 0;
    }
    
    int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener) override
    {
        return 0;
    }
    
    int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener) override
    {
        return 0;
    }
    
    int32_t RegisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener) override
    {
        return 0;
    }
    
    int32_t UnregisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener) override
    {
        return 0;
    }
    
    int32_t EnableSink(const std::vector<DHDescriptor> &descriptors) override
    {
        return 0;
    }
    
    int32_t DisableSink(const std::vector<DHDescriptor> &descriptors) override
    {
        return 0;
    }
    
    int32_t EnableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) override
    {
        return 0;
    }
    
    int32_t DisableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) override
    {
        return 0;
    }
    
    int32_t LoadDistributedHDF(const DHType dhType) override
    {
        return 0;
    }
    
    int32_t UnLoadDistributedHDF(const DHType dhType) override
    {
        return 0;
    }
    
    int32_t LoadSinkDMSDPService(const std::string &udid) override
    {
        return 0;
    }
    
    int32_t NotifySinkRemoteSourceStarted(const std::string &udid) override
    {
        return 0;
    }
    
    int32_t RegisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        int32_t &timeOut, const std::string &pkgName) override
    {
        return 0;
    }
    
    int32_t UnregisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        const std::string &pkgName) override
    {
        return 0;
    }
    
    void SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool &granted) override
    {
    }
};

void HandleNotifySourceRemoteSinkStartedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint16_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string udid = fdp.ConsumeRandomLengthString();
    uint32_t code = static_cast<uint32_t>(DHMsgInterfaceCode::NOTIFY_SOURCE_DEVICE_REMOTE_DMSDP_STARTED);
    MessageParcel datas;
    datas.WriteString(udid);
    MessageParcel reply;
    MessageOption option;
    auto stub = std::make_shared<DistributedHardwareStubImpl>();

    stub->OnRemoteRequest(code, datas, reply, option);
}

void HandleLoadSinkDMSDPServiceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint16_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    std::string udid = fdp.ConsumeRandomLengthString();
    uint32_t code = static_cast<uint32_t>(DHMsgInterfaceCode::INIT_SINK_DMSDP);
    MessageParcel datas;
    datas.WriteString(udid);
    MessageParcel reply;
    MessageOption option;
    auto stub = std::make_shared<DistributedHardwareStubImpl>();

    stub->OnRemoteRequest(code, datas, reply, option);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::HandleNotifySourceRemoteSinkStartedFuzzTest(data, size);
    OHOS::DistributedHardware::HandleLoadSinkDMSDPServiceFuzzTest(data, size);
    return 0;
}

