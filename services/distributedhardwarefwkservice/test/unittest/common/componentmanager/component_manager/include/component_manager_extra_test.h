/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_EXTRA_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_EXTRA_TEST_H

#include "gtest/gtest.h"

#include "get_dh_descriptors_callback_stub.h"

namespace OHOS {
namespace DistributedHardware {
class ComponentManagerExtraTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    class TestGetDistributedHardwareCallback : public GetDhDescriptorsCallbackStub {
    public:
        TestGetDistributedHardwareCallback() = default;
        virtual ~TestGetDistributedHardwareCallback() = default;
    protected:
        void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
            EnableStep enableStep) override;
        void OnError(const std::string &networkId, int32_t error) override;
    };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
