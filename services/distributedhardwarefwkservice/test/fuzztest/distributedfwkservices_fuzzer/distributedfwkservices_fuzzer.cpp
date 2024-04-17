/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <string>
#include <unistd.h>

#include "distributed_hardware_errno.h"
#include "distributed_hardware_service.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const uint32_t DH_TYPE_SIZE = 10;
    const DHType dhTypeFuzz[DH_TYPE_SIZE] = {
        DHType::CAMERA, DHType::AUDIO, DHType::SCREEN, DHType::VIRMODEM_AUDIO,
        DHType::INPUT, DHType::A2D, DHType::GPS, DHType::HFP
    };
    const uint32_t QUERY_LOCAL_SYS_SIZE = 6;
    const uint32_t IS_BOOL = 2;
    const uint32_t TRANS_ROLE_SIZE = 3;
    const QueryLocalSysSpecType specType[QUERY_LOCAL_SYS_SIZE] = {
        QueryLocalSysSpecType::MIN, QueryLocalSysSpecType::HISTREAMER_AUDIO_ENCODER,
        QueryLocalSysSpecType::HISTREAMER_AUDIO_DECODER, QueryLocalSysSpecType::HISTREAMER_VIDEO_ENCODER,
        QueryLocalSysSpecType::HISTREAMER_VIDEO_DECODER, QueryLocalSysSpecType::MAX
    };
    const TransRole transRole[TRANS_ROLE_SIZE] = {
        TransRole::AV_SENDER, TransRole::AV_RECEIVER, TransRole::UNKNOWN
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

void FwkServicesServicesFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    const int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
}

void FwkServicesInitLocalDevInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    const int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
    service.InitLocalDevInfo();
}

void FwkServicesOnStopFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    const int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
    service.OnStop();
}

void FwkServicesQueryLocalSysSpecFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
    QueryLocalSysSpecType spec = specType[data[0] % QUERY_LOCAL_SYS_SIZE];

    service.QueryLocalSysSpec(spec);
}

void FwkServicesQueryDhSysSpecFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
    std::string targetKey(reinterpret_cast<const char*>(data), size);
    std::string attrs(reinterpret_cast<const char*>(data), size);

    service.QueryDhSysSpec(targetKey, attrs);
}

void FwkServicesInitializeAVCenterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
    TransRole transRoleType = transRole[data[0] % TRANS_ROLE_SIZE];
    int32_t engineId = *(reinterpret_cast<const uint32_t*>(data));

    service.InitializeAVCenter(transRoleType, engineId);
}

void FwkServicesReleaseAVCenterFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
    int32_t engineId = *(reinterpret_cast<const uint32_t*>(data));
    
    service.ReleaseAVCenter(engineId);
}

void FwkServicesCreateControlChannelFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    DistributedHardwareService service(asId, runOnCreate);
    int32_t engineId = *(reinterpret_cast<const uint32_t*>(data));
    std::string peerDevId(reinterpret_cast<const char*>(data), size);

    service.CreateControlChannel(engineId, peerDevId);
}

void FwkServicesRegisterCtlCenterCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }
    sptr<IAVTransControlCenterCallback> callback = nullptr;

    DistributedHardwareService service(asId, runOnCreate);
    int32_t engineId = *(reinterpret_cast<const uint32_t*>(data));

    service.RegisterCtlCenterCallback(engineId, callback);
}

void FwkServicesNotifySourceRemoteSinkStartedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }

    DistributedHardwareService service(asId, runOnCreate);
    std::string deviceId(reinterpret_cast<const char*>(data), size);

    service.NotifySourceRemoteSinkStarted(deviceId);
}

void FwkServicesPauseDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }

    DistributedHardwareService service(asId, runOnCreate);
    DHType dhType = dhTypeFuzz[data[0] % DH_TYPE_SIZE];
    std::string networkId(reinterpret_cast<const char*>(data), size);

    service.PauseDistributedHardware(dhType, networkId);
}

void FwkServicesResumeDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }

    DistributedHardwareService service(asId, runOnCreate);
    DHType dhType = dhTypeFuzz[data[0] % DH_TYPE_SIZE];
    std::string networkId(reinterpret_cast<const char*>(data), size);

    service.ResumeDistributedHardware(dhType, networkId);
}

void FwkServicesStopDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    int32_t asId = *(reinterpret_cast<const uint32_t*>(data)) % IS_BOOL;
    bool runOnCreate = false;
    if (asId == 0) {
        runOnCreate = false;
    } else {
        runOnCreate = true;
    }

    DistributedHardwareService service(asId, runOnCreate);
    DHType dhType = dhTypeFuzz[data[0] % DH_TYPE_SIZE];
    std::string networkId(reinterpret_cast<const char*>(data), size);
    
    service.StopDistributedHardware(dhType, networkId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::FwkServicesServicesFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesInitLocalDevInfoFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesOnStopFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesQueryLocalSysSpecFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesQueryDhSysSpecFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesInitializeAVCenterFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesReleaseAVCenterFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesCreateControlChannelFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesRegisterCtlCenterCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesPauseDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesResumeDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::FwkServicesStopDistributedHardwareFuzzTest(data, size);
    return 0;
}
