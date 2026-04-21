/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "distributedhardwarefwkkittwo_fuzzer.h"

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

void TestHDSinkStatusListener::OnEnable(const DHDescriptor &dhDescriptor)
{
    (void)dhDescriptor;
}

void TestHDSinkStatusListener::OnDisable(const DHDescriptor &dhDescriptor)
{
    (void)dhDescriptor;
}

void TestHDSourceStatusListener::OnEnable(
    const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    (void)networkId;
    (void)dhDescriptor;
}

void TestHDSourceStatusListener::OnDisable(
    const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    (void)networkId;
    (void)dhDescriptor;
}

void TestGetDistributedHardwareCallback::OnSuccess(const std::string &networkId,
    const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    (void)networkId;
    (void)descriptors;
    (void)enableStep;
}

void TestGetDistributedHardwareCallback::OnError(const std::string &networkId, int32_t error)
{
    (void)networkId;
    (void)error;
}

int32_t TestAVTransControlCenterCallback::SetParameter(uint32_t tag, const std::string &value)
{
    (void)tag;
    (void)value;
    return DH_FWK_SUCCESS;
}

int32_t TestAVTransControlCenterCallback::Notify(const AVTransEventExt &event)
{
    (void)event;
    return DH_FWK_SUCCESS;
}

void TestAuthorizationResultCallback::OnAuthorizationResult(const std::string &networkId, const std::string &requestId)
{
    (void)networkId;
    (void)requestId;
}

void ResumeDistributedHardwareFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sptr<TestPublisherListener> listener(new TestPublisherListener());
    DistributedHardwareFwkKit dhfwkKit;
    DHType dhType = DHType::AUDIO;
    std::string networkId(reinterpret_cast<const char*>(data), size);
    dhfwkKit.ResumeDistributedHardware(dhType, networkId);
}

void StopDistributedHardwareFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sptr<TestPublisherListener> listener(new TestPublisherListener());
    DistributedHardwareFwkKit dhfwkKit;
    DHType dhType = DHType::AUDIO;
    std::string networkId(reinterpret_cast<const char*>(data), size);
    dhfwkKit.StopDistributedHardware(dhType, networkId);
}

void GetDistributedHardwareFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    DistributedHardwareFwkKit dhfwkKit;
    std::string networkId(reinterpret_cast<const char*>(data), size);
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    dhfwkKit.GetDistributedHardware(networkId, enableStep, callback);
}

void RegisterDHStatusListenerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    std::string networkId(reinterpret_cast<const char*>(data), size);
    sptr<IHDSourceStatusListener> listener(new TestHDSourceStatusListener());
    dhfwkKit.RegisterDHStatusListener(networkId, listener);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::ResumeDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::StopDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::GetDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterDHStatusListenerFuzzTest(data, size);
    return 0;
}
