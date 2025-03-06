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

#ifndef TEST_DISTRIBUTEDHARDWAREFWK_KIT_FUZZER_H
#define TEST_DISTRIBUTEDHARDWAREFWK_KIT_FUZZER_H

#define FUZZ_PROJECT_NAME "distributedhardwarefwkkit_fuzzer"

#include <string>
#include "hardware_status_listener_stub.h"
#include "publisher_listener_stub.h"

namespace OHOS {
namespace DistributedHardware {
class TestPublisherListener : public PublisherListenerStub {
public:
    TestPublisherListener() = default;
    virtual ~TestPublisherListener() = default;
    void OnMessage(const DHTopic topic, const std::string &message);
};

class TestHDSinkStatusListener : public HDSinkStatusListenerStub {
public:
    TestHDSinkStatusListener() = default;
    virtual ~TestHDSinkStatusListener() = default;
protected:
    void OnEnable(const DHDescriptor &dhDescriptor) override;
    void OnDisable(const DHDescriptor &dhDescriptor) override;
};

class TestHDSourceStatusListener : public HDSourceStatusListenerStub {
public:
    TestHDSourceStatusListener() = default;
    virtual ~TestHDSourceStatusListener() = default;
protected:
    void OnEnable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;
    void OnDisable(const std::string &networkId, const DHDescriptor &dhDescriptor) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif