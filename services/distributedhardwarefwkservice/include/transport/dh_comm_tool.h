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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMM_TOOL_H
#define OHOS_DISTRIBUTED_HARDWARE_COMM_TOOL_H

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "dh_transport.h"
#include "dh_transport_obj.h"
#include "event_handler.h"

namespace OHOS {
namespace DistributedHardware {
class DHCommTool : public std::enable_shared_from_this<DHCommTool> {
public:
    DHCommTool();
    virtual ~DHCommTool() = default;
    static std::shared_ptr<DHCommTool> GetInstance();
    void Init();
    void UnInit();
    /**
     * @brief trigger request remote dh send back full capatilities.
     *        payload: {code: DH_COMM_REQ_FULL_CAPS, msg: localNetworkId}.
     *        msg means the device need the dh capatilities, the remote side should use
     *        localNetworkId to send dh capatilities msg back.
     *
     * @param remoteNetworkId the target device network id
     */
    void TriggerReqFullDHCaps(const std::string &remoteNetworkId);
    void GetAndSendLocalFullCaps(const std::string &reqNetworkId, bool isSyncMeta);
    FullCapsRsp ParseAndSaveRemoteDHCaps(const std::string &remoteCaps, bool isSyncMeta);
    std::string GetLocalFullMetaCapsInfo(bool isSyncMeta);
    std::string GetLocalFullCapsInfo(bool isSyncMeta);

    class DHCommToolEventHandler : public AppExecFwk::EventHandler {
    public:
        DHCommToolEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> runner,
            std::shared_ptr<DHCommTool> dhCommToolPtr);
        ~DHCommToolEventHandler() override = default;
        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    private:
        void ProcessFullCapsRsp(const FullCapsRsp &capsRsp, const std::shared_ptr<DHCommTool> dhCommToolPtr,
            bool isSyncMeta);
        std::weak_ptr<DHCommTool> dhCommToolWPtr_;
    };
    std::shared_ptr<DHCommTool::DHCommToolEventHandler> GetEventHandler();
    const std::shared_ptr<DHTransport> GetDHTransportPtr();

private:
    bool CheckCallerAclRight(const std::string &localNetworkId, const std::string &remoteNetworkId);
    bool GetOsAccountInfo();

private:
    std::shared_ptr<DHTransport> dhTransportPtr_;
    std::shared_ptr<DHCommTool::DHCommToolEventHandler> eventHandler_;
    std::string accountId_;
    int32_t userId_ = -1;
    uint64_t tokenId_ = 0;
};
} // DistributedHardware
} // OHOS
#endif