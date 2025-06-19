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

#ifndef TEST_GET_DH_DESCRIPTORS_CALLBACK_STUB_FUZZER_H
#define TEST_GET_DH_DESCRIPTORS_CALLBACK_STUB_FUZZER_H

#define FUZZ_PROJECT_NAME "getdhdescriptorscallbackstub_fuzzer.cpp"

#include <string>
#include "get_dh_descriptors_callback_stub.h"
#include "dhardware_descriptor.h"
#include "constants.h"

namespace OHOS {
namespace DistributedHardware {

class FuzzGetDhDescriptorsCallbackStub : public GetDhDescriptorsCallbackStub {
public:
    void OnSuccess(const std::string &, const std::vector<DHDescriptor> &, EnableStep) override;
    void OnError(const std::string &, int32_t) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif