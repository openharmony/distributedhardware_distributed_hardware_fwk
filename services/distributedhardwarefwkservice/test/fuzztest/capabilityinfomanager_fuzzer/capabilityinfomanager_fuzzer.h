/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef TEST_CAPABILITYINFOMANAGER_FUZZER_H
#define TEST_CAPABILITYINFOMANAGER_FUZZER_H

#define FUZZ_PROJECT_NAME "capabilityinfomanager_fuzzer"

#include <string>
#include "get_dh_descriptors_callback_stub.h"

namespace OHOS {
namespace DistributedHardware {

class TestGetDistributedHardwareCallback : public GetDhDescriptorsCallbackStub {
public:
    TestGetDistributedHardwareCallback() = default;
    virtual ~TestGetDistributedHardwareCallback() = default;
protected:
    void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
        EnableStep enableStep) override;
    void OnError(const std::string &networkId, int32_t error) override;
};
} // namespace DistributedHardware
} // namespace OHOS

#endif

