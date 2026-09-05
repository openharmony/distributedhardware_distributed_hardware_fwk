/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dhcommtoolacl_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "constants.h"
#include "dh_transport.h"
#include "dh_comm_tool.h"
#include "socket.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace DistributedHardware {
void CheckCallerAclRightFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string localNetworkId = fdp.ConsumeRandomLengthString();
    std::string remoteNetworkId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->CheckCallerAclRight(localNetworkId, remoteNetworkId);
}

void CheckCallerAclRightWithUserIdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string accountId = fdp.ConsumeRandomLengthString();
    std::string localNetworkId = fdp.ConsumeRandomLengthString();
    std::string remoteNetworkId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->userId_ = userId;
    dhCommTool->accountId_ = accountId;
    dhCommTool->CheckCallerAclRight(localNetworkId, remoteNetworkId);
}

void GetOsAccountInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->GetOsAccountInfo();
    dhCommTool->GetOsAccountInfo();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CheckCallerAclRightFuzzTest(data, size);
    OHOS::DistributedHardware::CheckCallerAclRightWithUserIdFuzzTest(data, size);
    OHOS::DistributedHardware::GetOsAccountInfoFuzzTest(data, size);
    return 0;
}
