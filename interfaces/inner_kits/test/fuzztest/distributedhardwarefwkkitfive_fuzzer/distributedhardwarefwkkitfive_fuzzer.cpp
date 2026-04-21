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

#include "distributedhardwarefwkkitfive_fuzzer.h"

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

void RegisterDHStatusListenerOneParamFuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    DistributedHardwareFwkKit dhfwkKit;
    sptr<IHDSinkStatusListener> listener(new TestHDSinkStatusListener());
    dhfwkKit.RegisterDHStatusListener(listener);
}

void UnregisterDHStatusListenerOneParamFuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    DistributedHardwareFwkKit dhfwkKit;
    sptr<IHDSinkStatusListener> listener(new TestHDSinkStatusListener());
    dhfwkKit.UnregisterDHStatusListener(listener);
}

void IsQueryLocalSysSpecTypeValidFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    QueryLocalSysSpecType specType = static_cast<QueryLocalSysSpecType>(*(reinterpret_cast<const uint32_t*>(data)));
    dhfwkKit.IsQueryLocalSysSpecTypeValid(specType);
}

void RegisterCtlCenterCallbackFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    int32_t engineId = *(reinterpret_cast<const int32_t*>(data));
    sptr<IAvTransControlCenterCallback> listener(new TestAVTransControlCenterCallback());
    dhfwkKit.RegisterCtlCenterCallback(engineId, listener);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::RegisterDHStatusListenerOneParamFuzzTest(data, size);
    OHOS::DistributedHardware::UnregisterDHStatusListenerOneParamFuzzTest(data, size);
    OHOS::DistributedHardware::IsQueryLocalSysSpecTypeValidFuzzTest(data, size);
    OHOS::DistributedHardware::RegisterCtlCenterCallbackFuzzTest(data, size);
    return 0;
}
