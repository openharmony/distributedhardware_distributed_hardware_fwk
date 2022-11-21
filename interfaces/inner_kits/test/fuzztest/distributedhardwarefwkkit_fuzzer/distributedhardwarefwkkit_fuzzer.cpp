/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
    std::map<DHTopic, std::string> TOPIC_MSGS = {
        { DHTopic::TOPIC_START_DSCREEN, "Start DScreen" },
        { DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, "Sink Project Window" },
        { DHTopic::TOPIC_STOP_DSCREEN, "Stop DScreen" },
        { DHTopic::TOPIC_DEV_OFFLINE, "Dev Offline" }
    };
    const int32_t SLEEP_TIME_MS = 1000;
}

void TestPublisherListener::OnMessage(const DHTopic topic, const std::string &message)
{
    (void)topic;
    (void)message;
}

void DistributedHardwareFwkKitFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sptr<TestPublisherListener> listener = new TestPublisherListener();
    DistributedHardwareFwkKit dhfwkKit;
    dhfwkKit.RegisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener);
    dhfwkKit.RegisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener);
    dhfwkKit.RegisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener);
    dhfwkKit.RegisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener);

    dhfwkKit.PublishMessage(DHTopic::TOPIC_START_DSCREEN, TOPIC_MSGS[DHTopic::TOPIC_START_DSCREEN]);
    dhfwkKit.PublishMessage(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO,
        TOPIC_MSGS[DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO]);
    dhfwkKit.PublishMessage(DHTopic::TOPIC_STOP_DSCREEN, TOPIC_MSGS[DHTopic::TOPIC_STOP_DSCREEN]);
    dhfwkKit.PublishMessage(DHTopic::TOPIC_DEV_OFFLINE, TOPIC_MSGS[DHTopic::TOPIC_DEV_OFFLINE]);

    dhfwkKit.UnregisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener);
    dhfwkKit.UnregisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener);
    dhfwkKit.UnregisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener);
    dhfwkKit.UnregisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DistributedHardwareFwkKitFuzzTest(data, size);
    return 0;
}