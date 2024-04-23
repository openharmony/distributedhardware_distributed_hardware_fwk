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

#include "distributedhardwarefwkstub_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <thread>
#include <unistd.h>

#include "distributed_hardware_fwk_kit.h"
#include "distributed_hardware_errno.h"
#include "publisher_listener_stub.h"

namespace OHOS {
namespace DistributedHardware {

class MyPublisherListenerStub : public PublisherListenerStub {
public:
    void OnMessage(const DHTopic topic, const std::string& message) override
    {
        return;
    }
};

void DistributedHardwareFwkStubFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    MessageParcel pdata;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = *(reinterpret_cast<const uint32_t*>(data));
    pdata.WriteInterfaceToken(PublisherListenerStub::GetDescriptor());
    if (code == (uint32_t)IPublisherListener::Message::ON_MESSAGE) {
        uint32_t tag = *(reinterpret_cast<const uint32_t*>(data));
        std::string value(reinterpret_cast<const char*>(data), size);
        pdata.WriteUint32(tag);
        pdata.WriteString(value);
    }
    sptr<PublisherListenerStub> publisherListenerStub(new (std::nothrow)
        MyPublisherListenerStub());
    publisherListenerStub->OnRemoteRequest(code, pdata, reply, option);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DistributedHardwareFwkStubFuzzTest(data, size);
    return 0;
}