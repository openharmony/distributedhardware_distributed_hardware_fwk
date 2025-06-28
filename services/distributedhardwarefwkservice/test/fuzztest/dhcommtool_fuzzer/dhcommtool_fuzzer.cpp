/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dhcommtool_fuzzer.h"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <unistd.h>

#include "constants.h"
#include "dh_transport.h"
#include "dh_comm_tool.h"
#include "socket.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t SOCKETID = 1;
}
void DhTransportTriggerReqFullDHCapsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string remoteNetworkId(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->dhTransportPtr_ = std::make_shared<DHTransport>(dhCommTool);
    dhCommTool->dhTransportPtr_->remoteDevSocketIds_[remoteNetworkId] = SOCKETID;
    dhCommTool->TriggerReqFullDHCaps(remoteNetworkId);
    dhCommTool->dhTransportPtr_->remoteDevSocketIds_.clear();
    dhCommTool->dhTransportPtr_ = nullptr;
}

void DhTransportGetAndSendLocalFullCapsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string remoteNetworkId(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->dhTransportPtr_ = std::make_shared<DHTransport>(dhCommTool);
    dhCommTool->GetAndSendLocalFullCaps(remoteNetworkId);
    dhCommTool->dhTransportPtr_ = nullptr;
}

void DhTransportParseAndSaveRemoteDHCapsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string remoteCaps(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->ParseAndSaveRemoteDHCaps(remoteCaps);
}

void DhTransportInitFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->Init();
}

void DhTransportUnInitFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->UnInit();
}

void DHCommToolEventHandlerCtorFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    auto runner = AppExecFwk::EventRunner::Create(true);
    std::shared_ptr<DHCommTool> commToolPtr;
    DHCommTool::DHCommToolEventHandler handler(runner, commToolPtr);
}

void DHCommToolEventHandlerProcessEventFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    auto runner = AppExecFwk::EventRunner::Create(true);
    auto commTool = std::make_shared<DHCommTool>();
    auto handler = std::make_shared<DHCommTool::DHCommToolEventHandler>(runner, commTool);

    uint32_t eventId = (size > 0) ? data[0] % 4 : 0;

    auto event = AppExecFwk::InnerEvent::Get(eventId);
    handler->ProcessEvent(event);
    auto eventNoMsg = AppExecFwk::InnerEvent::Get(eventId);
    handler->ProcessEvent(eventNoMsg);
}

void DHCommToolEventHandlerProcessFullCapsRspFuzzTest(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    auto runner = AppExecFwk::EventRunner::Create(true);
    auto commTool = std::make_shared<DHCommTool>();
    DHCommTool::DHCommToolEventHandler handler(runner, commTool);

    FullCapsRsp capsRsp;
    handler.ProcessFullCapsRsp(capsRsp, commTool);

    capsRsp.networkId = "fuzz_network";
    handler.ProcessFullCapsRsp(capsRsp, commTool);

    auto capInfo = std::make_shared<CapabilityInfo>();
    capsRsp.caps.push_back(capInfo);
    handler.ProcessFullCapsRsp(capsRsp, nullptr);

    auto commToolNoTrans = std::make_shared<DHCommTool>();
    handler.ProcessFullCapsRsp(capsRsp, commToolNoTrans);

    commTool->Init();
    handler.ProcessFullCapsRsp(capsRsp, commTool);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */

    OHOS::DistributedHardware::DhTransportTriggerReqFullDHCapsFuzzTest(data, size);
    OHOS::DistributedHardware::DhTransportGetAndSendLocalFullCapsFuzzTest(data, size);
    OHOS::DistributedHardware::DhTransportParseAndSaveRemoteDHCapsFuzzTest(data, size);
    OHOS::DistributedHardware::DhTransportInitFuzzTest(data, size);
    OHOS::DistributedHardware::DhTransportUnInitFuzzTest(data, size);
    OHOS::DistributedHardware::DHCommToolEventHandlerCtorFuzzTest(data, size);
    OHOS::DistributedHardware::DHCommToolEventHandlerProcessEventFuzzTest(data, size);
    OHOS::DistributedHardware::DHCommToolEventHandlerProcessFullCapsRspFuzzTest(data, size);
    return 0;
}

