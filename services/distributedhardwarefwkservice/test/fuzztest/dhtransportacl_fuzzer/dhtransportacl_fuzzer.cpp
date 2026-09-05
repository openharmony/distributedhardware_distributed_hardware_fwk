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

#include "dhtransportacl_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "dh_transport.h"
#include "dh_comm_tool.h"
#include "dh_transport_obj.h"

namespace OHOS {
namespace DistributedHardware {
void CheckCalleeAclRightFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t userId = fdp.ConsumeIntegral<int32_t>();
    std::string msg = fdp.ConsumeRandomLengthString();
    std::string accountId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    std::shared_ptr<DHTransport> dhTransportTest = std::make_shared<DHTransport>(dhCommTool);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = userId;
    commMsg->msg = msg;
    commMsg->accountId = accountId;
    dhTransportTest->CheckCalleeAclRight(commMsg);
}

void GetForegroundUserInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    int32_t targetUserId = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    std::shared_ptr<DHTransport> dhTransportTest = std::make_shared<DHTransport>(dhCommTool);
    int32_t userId = 0;
    std::string accountId = "";
    dhTransportTest->GetForegroundUserInfo(targetUserId, userId, accountId);
}

void CheckCalleeAclRightNegOneUserIdFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string msg = fdp.ConsumeRandomLengthString();
    std::string accountId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    std::shared_ptr<DHTransport> dhTransportTest = std::make_shared<DHTransport>(dhCommTool);
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    commMsg->userId = -1;
    commMsg->msg = msg;
    commMsg->accountId = accountId;
    dhTransportTest->CheckCalleeAclRight(commMsg);
}

void HandleReceiveMessageFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    FuzzedDataProvider fdp(data, size);
    std::string payload = fdp.ConsumeRandomLengthString();
    std::string remoteNetworkId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    std::shared_ptr<DHTransport> dhTransportTest = std::make_shared<DHTransport>(dhCommTool);
    dhTransportTest->HandleReceiveMessage(payload, remoteNetworkId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CheckCalleeAclRightFuzzTest(data, size);
    OHOS::DistributedHardware::GetForegroundUserInfoFuzzTest(data, size);
    OHOS::DistributedHardware::CheckCalleeAclRightNegOneUserIdFuzzTest(data, size);
    OHOS::DistributedHardware::HandleReceiveMessageFuzzTest(data, size);
    return 0;
}
