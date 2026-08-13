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

#include "distributedfwkstub_fuzzer.h"

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

const int32_t SAID = 4801;
const uint32_t MIN_DH_TYPE = 0;
const uint32_t MAX_DH_TYPE = 10;

void LoadDistributedHDFInnerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    
    FuzzedDataProvider fdp(data, size);
    auto distributedHardwareService = std::make_shared<DistributedHardwareService>(SAID, false);
    
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;

    uint32_t dhType = fdp.ConsumeIntegralInRange<uint32_t>(MIN_DH_TYPE, MAX_DH_TYPE);
    dataParcel.WriteInterfaceToken(distributedHardwareService->GetDescriptor());
    dataParcel.WriteUint32(dhType);

    if (fdp.ConsumeBool()) {
        sptr<IRemoteObject> remoteObject = nullptr;
        dataParcel.WriteRemoteObject(remoteObject);
    } else {
        dataParcel.WriteRemoteObject(nullptr);
    }
    distributedHardwareService->LoadDistributedHDFInner(dataParcel, reply);
}

void UnLoadDistributedHDFInnerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    
    FuzzedDataProvider fdp(data, size);
    auto distributedHardwareService = std::make_shared<DistributedHardwareService>(SAID, false);
    
    MessageParcel dataParcel;
    MessageParcel reply;
    MessageOption option;

    uint32_t dhType = fdp.ConsumeIntegralInRange<uint32_t>(MIN_DH_TYPE, MAX_DH_TYPE);
    dataParcel.WriteInterfaceToken(distributedHardwareService->GetDescriptor());
    dataParcel.WriteUint32(dhType);

    if (fdp.ConsumeBool()) {
        sptr<IRemoteObject> remoteObject = nullptr;
        dataParcel.WriteRemoteObject(remoteObject);
    } else {
        dataParcel.WriteRemoteObject(nullptr);
    }
    distributedHardwareService->UnLoadDistributedHDFInner(dataParcel, reply);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::LoadDistributedHDFInnerFuzzTest(data, size);
    OHOS::DistributedHardware::UnLoadDistributedHDFInnerFuzzTest(data, size);
    return 0;
}
