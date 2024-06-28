/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "distributedhardwarefwkkit_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <thread>
#include <unistd.h>

#include "distributed_hardware_fwk_kit.h"
#include "distributed_hardware_errno.h"
#include "publisher_listener_stub.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t SLEEP_TIME_MS = 1000;
}

void TestPublisherListener::OnMessage(const DHTopic topic, const std::string &message)
{
    (void)topic;
    (void)message;
}

void RegisterPublisherListenerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    sptr<TestPublisherListener> listener = new TestPublisherListener();
    DistributedHardwareFwkKit dhfwkKit;
    DHTopic topic = static_cast<DHTopic>(*(reinterpret_cast<const uint32_t*>(data)));
    dhfwkKit.RegisterPublisherListener(topic, listener);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
}

void PublishMessageFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    sptr<TestPublisherListener> listener = new TestPublisherListener();
    DistributedHardwareFwkKit dhfwkKit;
    DHTopic topic = static_cast<DHTopic>(*(reinterpret_cast<const uint32_t*>(data)));
    std::string message(reinterpret_cast<const char*>(data), size);
    dhfwkKit.PublishMessage(topic, message);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
}

void UnregisterPublisherListenerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    sptr<TestPublisherListener> listener = new TestPublisherListener();
    DistributedHardwareFwkKit dhfwkKit;
    DHTopic topic = static_cast<DHTopic>(*(reinterpret_cast<const uint32_t*>(data)));
    dhfwkKit.UnregisterPublisherListener(topic, listener);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::RegisterPublisherListenerFuzzTest(data, size);
    OHOS::DistributedHardware::PublishMessageFuzzTest(data, size);
    OHOS::DistributedHardware::UnregisterPublisherListenerFuzzTest(data, size);
    return 0;
}