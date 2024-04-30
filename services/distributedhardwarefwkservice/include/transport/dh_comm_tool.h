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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMM_TOOL_H
#define OHOS_DISTRIBUTED_HARDWARE_COMM_TOOL_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "dh_transport.h"
#include "event_handler.h"

namespace OHOS {
namespace DistributedHardware {
// request remote dh send back full dh capabilities
constexpr uint32_t DH_COMM_REQ_FULL_CAPS = 1;
// send back full dh attributes to the requester
constexpr uint32_t DH_COMM_RSP_FULL_CAPS = 2;

class DHCommTool : public enable_shared_from_this<DHCommTool> {
public:
    explicit DHCommTool();
    virtual ~DHCommTool() = default;
    void Init();
    /**
     * @brief trigger request remote dh send back full capatilities.
     *        payload: {code: DH_COMM_REQ_FULL_CAPS, msg: localNetworkId}.
     *        msg means the device need the dh capatilities, the remote side should use
     *        localNetworkId to send dh capatilities msg back.
     *
     * @param remoteNetworkId the target device network id
     */
    void TriggerReqFullDHCaps(const std::string &remoteNetworkId);

    class DHCommToolEventHandler : public AppExecFwk::EventHandler {
        public:
            DHCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
            ~DHCommToolEventHandler() override = default;
            void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    };
    std::shared_ptr<DHCommTool::DHCommToolEventHandler> GetEventHandler();

private:
    void DealReqFullDHCaps(const std::string &msg);
    void DealRspFullDHCaps(const std::string &msg);
    void GetAndSendLocalFullCaps(const std::string &reqNetworkId);
    void ParseAndSaveRemoteDHCaps(const std::string &remoteCaps);

private:
    std::shared_ptr<DHTransport> dhTransportPtr_;
    std::shared_ptr<DHCommTool::DHCommToolEventHandler> eventHandler_;
};
} // DistributedHardware
} // OHOS
#endif