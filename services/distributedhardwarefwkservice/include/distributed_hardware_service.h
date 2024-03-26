/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_SERVICE_H
#define OHOS_DISTRIBUTED_HARDWARE_SERVICE_H

#include "system_ability.h"
#include "ipc_object_stub.h"

#include "distributed_hardware_fwk_kit.h"
#include "distributed_hardware_fwk_kit_paras.h"
#include "distributed_hardware_stub.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

class DistributedHardwareService : public SystemAbility, public DistributedHardwareStub {
DECLARE_SYSTEM_ABILITY(DistributedHardwareService);
public:
    DistributedHardwareService(int32_t saId, bool runOnCreate);
    ~DistributedHardwareService() = default;
    int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> &listener) override;
    int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> &listener) override;
    int32_t PublishMessage(const DHTopic topic, const std::string &msg) override;
    std::string QueryLocalSysSpec(const QueryLocalSysSpecType spec) override;
    int Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId) override;
    int32_t ReleaseAVCenter(int32_t engineId) override;
    int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId) override;
    int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event) override;
    int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAVTransControlCenterCallback> &callback) override;
    int32_t NotifySourceRemoteSinkStarted(std::string &deviceId) override;
    int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t StopDistributedHardware(DHType dhType, const std::string &networkId) override;

protected:
    void OnStart() override;
    void OnStop() override;

private:
    bool Init();
    std::string QueryDhSysSpec(const std::string &targetKey, std::string &attrs);
    void InitLocalDevInfo();

private:
    bool registerToService_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif