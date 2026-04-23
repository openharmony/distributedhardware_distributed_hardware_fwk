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

#include "distributedhardwarefwkkitthree_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "distributed_hardware_fwk_kit.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const size_t MAX_SIZE = 4096;
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

void UnregisterDHStatusListenerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0) || (size > MAX_SIZE)) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    std::string networkId(reinterpret_cast<const char*>(data), size);
    sptr<IHDSourceStatusListener> listener(new TestHDSourceStatusListener());
    dhfwkKit.UnregisterDHStatusListener(networkId, listener);
}

void EnableSourceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0) || (size > MAX_SIZE)) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    std::string networkId(reinterpret_cast<const char*>(data), size);
    std::vector<DHDescriptor> descriptors;
    dhfwkKit.EnableSource(networkId, descriptors);
}

void DisableSourceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0) || (size > MAX_SIZE)) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    std::string networkId(reinterpret_cast<const char*>(data), size);
    std::vector<DHDescriptor> descriptors;
    dhfwkKit.DisableSource(networkId, descriptors);
}

void EnableSinkFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0) || (size > MAX_SIZE)) {
        return;
    }

    DistributedHardwareFwkKit dhfwkKit;
    std::vector<DHDescriptor> descriptors;
    std::string dhId(reinterpret_cast<const char*>(data), size);
    DHType dhType = DHType::AUDIO;
    DHDescriptor dhDescriptor {
        .id = dhId,
        .dhType = dhType
    };
    descriptors.push_back(dhDescriptor);
    dhfwkKit.EnableSink(descriptors);
}
} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::UnregisterDHStatusListenerFuzzTest(data, size);
    OHOS::DistributedHardware::EnableSourceFuzzTest(data, size);
    OHOS::DistributedHardware::DisableSourceFuzzTest(data, size);
    OHOS::DistributedHardware::EnableSinkFuzzTest(data, size);
    return 0;
}
