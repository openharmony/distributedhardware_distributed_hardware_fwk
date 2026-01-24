/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "distributedfwkservices_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <unistd.h>

#include "distributed_hardware_errno.h"
#include "distributed_hardware_service.h"
#include "get_dh_descriptors_callback_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t SAID = 4801;
    const uint32_t QUERY_LOCAL_SYS_SIZE = 6;
    const QueryLocalSysSpecType SPEC_TYPE[QUERY_LOCAL_SYS_SIZE] = {
        QueryLocalSysSpecType::MIN, QueryLocalSysSpecType::HISTREAMER_AUDIO_ENCODER,
        QueryLocalSysSpecType::HISTREAMER_AUDIO_DECODER, QueryLocalSysSpecType::HISTREAMER_VIDEO_ENCODER,
        QueryLocalSysSpecType::HISTREAMER_VIDEO_DECODER, QueryLocalSysSpecType::MAX
    };
}

class MyFwkServicesFuzzTest : public IRemoteStub<IPublisherListener> {
public:
    virtual sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
    void OnMessage(const DHTopic topic, const std::string& message) override
    {
        return;
    }
};

class TestGetDistributedHardwareCallback : public GetDhDescriptorsCallbackStub {
public:
    TestGetDistributedHardwareCallback() = default;
    virtual ~TestGetDistributedHardwareCallback() = default;
protected:
    void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
        EnableStep enableStep) override
    {
        (void)networkId;
        (void)descriptors;
        (void)enableStep;
    }
    void OnError(const std::string &networkId, int32_t error) override
    {
        (void)networkId;
        (void)error;
    }
};

class TestAuthorizationResultCallback : public IRemoteStub<IAuthorizationResultCallback> {
public:
    void OnAuthorizationResult(const std::string &networkId, const std::string &requestId) override
    {
        (void)networkId;
        (void)requestId;
    }
};

void FwkServicesQueryLocalSysSpecFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    uint32_t sysSpec = fdp.ConsumeIntegral<uint32_t>();
    QueryLocalSysSpecType spec = SPEC_TYPE[sysSpec % QUERY_LOCAL_SYS_SIZE];
    service.QueryLocalSysSpec(spec);
}

void FwkServicesQueryDhSysSpecFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    std::string targetKey = fdp.ConsumeRandomLengthString();
    std::string attrs = fdp.ConsumeRandomLengthString();
    service.QueryDhSysSpec(targetKey, attrs);
}

void FwkServicesNotifySourceRemoteSinkStartedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    std::string deviceId = fdp.ConsumeRandomLengthString();
    service.NotifySourceRemoteSinkStarted(deviceId);
}

void FwkServicesPauseDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();
    service.PauseDistributedHardware(dhType, networkId);
}

void FwkServicesResumeDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();

    service.ResumeDistributedHardware(dhType, networkId);
}

void FwkServicesStopDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();
    service.StopDistributedHardware(dhType, networkId);
}

void FwkServicesGetDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    service.GetDistributedHardware(networkId, enableStep, callback);
}

void FwkServicesRegisterDHStatusListenerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    sptr<IHDSourceStatusListener> listener = nullptr;
    service.RegisterDHStatusListener(networkId, listener);
}

void FwkServicesUnregisterDHStatusListenerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    std::string networkId = fdp.ConsumeRandomLengthString();
    sptr<IHDSourceStatusListener> listener = nullptr;

    service.UnregisterDHStatusListener(networkId, listener);
}

void FwkServicesEnableSinkFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string dhId = fdp.ConsumeRandomLengthString();
    DHDescriptor descriptor { dhId, dhType };
    std::vector<DHDescriptor> descriptors;
    descriptors.push_back(descriptor);
    service.EnableSink(descriptors);
}

void FwkServicesDisableSinkFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string dhId = fdp.ConsumeRandomLengthString();
    DHDescriptor descriptor { dhId, dhType };
    std::vector<DHDescriptor> descriptors;
    descriptors.push_back(descriptor);
    service.DisableSink(descriptors);
}

void FwkServicesEnableSourceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    DHDescriptor descriptor { dhId, dhType };
    std::vector<DHDescriptor> descriptors;
    descriptors.push_back(descriptor);
    service.EnableSource(networkId, descriptors);
}

void FwkServicesDisableSourceFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string networkId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    DHDescriptor descriptor { dhId, dhType };
    std::vector<DHDescriptor> descriptors;
    descriptors.push_back(descriptor);
    service.DisableSource(networkId, descriptors);
}

void FwkServicesRegisterHardwareAccessListenerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    int32_t timeOut = fdp.ConsumeIntegral<int32_t>();
    std::string pkgName = fdp.ConsumeRandomLengthString();
    sptr<IAuthorizationResultCallback> callback(new TestAuthorizationResultCallback());
    service.RegisterHardwareAccessListener(dhType, callback, timeOut, pkgName);
}

void FwkServicesUnregisterHardwareAccessListenerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string pkgName = fdp.ConsumeRandomLengthString();
    sptr<IAuthorizationResultCallback> callback(new TestAuthorizationResultCallback());
    service.UnregisterHardwareAccessListener(dhType, callback, pkgName);
}

void FwkServicesSetAuthorizationResultFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareService service(SAID, true);
    FuzzedDataProvider fdp(data, size);
    DHType dhType = static_cast<DHType>(fdp.ConsumeIntegral<uint32_t>());
    std::string requestId = fdp.ConsumeRandomLengthString();
    sptr<IAuthorizationResultCallback> callback(new TestAuthorizationResultCallback());
    bool granted = fdp.ConsumeBool();
    service.SetAuthorizationResult(dhType, requestId, granted);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::FwkServicesQueryLocalSysSpecFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesQueryDhSysSpecFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesPauseDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesResumeDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesStopDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesRegisterHardwareAccessListenerFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesUnregisterHardwareAccessListenerFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesSetAuthorizationResultFuzzTest(data, size);
    return 0;
}
