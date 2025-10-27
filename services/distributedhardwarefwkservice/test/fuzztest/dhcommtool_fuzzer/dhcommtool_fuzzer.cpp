/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <unistd.h>

#include "constants.h"
#include "dh_transport.h"
#include "dh_comm_tool.h"
#include "socket.h"
#include "softbus_bus_center.h"

namespace OHOS {
namespace DistributedHardware {
void DhTransportTriggerReqFullDHCapsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t socketId = fdp.ConsumeIntegral<int32_t>();
    std::string remoteNetworkId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->dhTransportPtr_ = std::make_shared<DHTransport>(dhCommTool);
    dhCommTool->dhTransportPtr_->remoteDevSocketIds_[remoteNetworkId] = socketId;
    dhCommTool->TriggerReqFullDHCaps(remoteNetworkId);
    dhCommTool->dhTransportPtr_->remoteDevSocketIds_.clear();
    dhCommTool->dhTransportPtr_ = nullptr;
}

void DhTransportGetAndSendLocalFullCapsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    bool isSyncMeta = true;
    FuzzedDataProvider fdp(data, size);
    std::string remoteNetworkId = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->dhTransportPtr_ = std::make_shared<DHTransport>(dhCommTool);
    dhCommTool->GetAndSendLocalFullCaps(remoteNetworkId, isSyncMeta);
    dhCommTool->dhTransportPtr_ = nullptr;
}

void DhTransportParseAndSaveRemoteDHCapsFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    bool isSyncMeta = true;
    FuzzedDataProvider fdp(data, size);
    std::string remoteCaps = fdp.ConsumeRandomLengthString();
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhCommTool->ParseAndSaveRemoteDHCaps(remoteCaps, isSyncMeta);
}

void DHCommToolEventHandlerProcessEventFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }
    auto runner = AppExecFwk::EventRunner::Create(true);
    auto commTool = std::make_shared<DHCommTool>();
    auto handler = std::make_shared<DHCommTool::DHCommToolEventHandler>(runner, commTool);

    FuzzedDataProvider fdp(data, size);
    uint32_t eventId = fdp.ConsumeIntegral<uint32_t>();

    auto event = AppExecFwk::InnerEvent::Get(eventId);
    handler->ProcessEvent(event);
}

void DHCommToolEventHandlerProcessFullCapsRspFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    auto runner = AppExecFwk::EventRunner::Create(true);
    auto commTool = std::make_shared<DHCommTool>();
    DHCommTool::DHCommToolEventHandler handler(runner, commTool);

    bool isSyncMeta = true;
    FullCapsRsp capsRsp;
    handler.ProcessFullCapsRsp(capsRsp, commTool, isSyncMeta);

    FuzzedDataProvider fdp(data, size);
    capsRsp.networkId = fdp.ConsumeRandomLengthString();
    handler.ProcessFullCapsRsp(capsRsp, commTool, isSyncMeta);

    auto capInfo = std::make_shared<CapabilityInfo>();
    capsRsp.caps.push_back(capInfo);
    handler.ProcessFullCapsRsp(capsRsp, nullptr, isSyncMeta);

    auto commToolNoTrans = std::make_shared<DHCommTool>();
    handler.ProcessFullCapsRsp(capsRsp, commToolNoTrans, isSyncMeta);

    commTool->Init();
    handler.ProcessFullCapsRsp(capsRsp, commTool, isSyncMeta);
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
    OHOS::DistributedHardware::DHCommToolEventHandlerProcessEventFuzzTest(data, size);
    OHOS::DistributedHardware::DHCommToolEventHandlerProcessFullCapsRspFuzzTest(data, size);
    return 0;
}

