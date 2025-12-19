/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "event_handler.h"
#include "ipc_object_stub.h"
#include "system_ability.h"
#include "system_ability_load_callback_stub.h"

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
    int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) override;
    int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener) override;
    int32_t PublishMessage(const DHTopic topic, const std::string &msg) override;
    std::string QueryLocalSysSpec(const QueryLocalSysSpecType spec) override;
    int Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId) override;
    int32_t ReleaseAVCenter(int32_t engineId) override;
    int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId) override;
    int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event) override;
    int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAvTransControlCenterCallback> callback) override;
    int32_t NotifySourceRemoteSinkStarted(std::string &udid) override;
    int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t StopDistributedHardware(DHType dhType, const std::string &networkId) override;
    int32_t GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
        const sptr<IGetDhDescriptorsCallback> callback) override;
    int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener) override;
    int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener) override;
    int32_t RegisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener) override;
    int32_t UnregisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener) override;
    int32_t EnableSink(const std::vector<DHDescriptor> &descriptors) override;
    int32_t DisableSink(const std::vector<DHDescriptor> &descriptors) override;
    int32_t EnableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) override;
    int32_t DisableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors) override;
    int32_t LoadDistributedHDF(const DHType dhType) override;
    int32_t UnLoadDistributedHDF(const DHType dhType) override;
    int32_t LoadSinkDMSDPService(const std::string &udid) override;
    int32_t NotifySinkRemoteSourceStarted(const std::string &udid) override;
    int32_t RegisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        int32_t &timeOut, const std::string &pkgName) override;
    int32_t UnregisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        const std::string &pkgName) override;
    void SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool &granted) override;
    int32_t CheckDHAccessPermission(const std::string &udid);

public:
    class LoadDMSDPServiceCallback : public SystemAbilityLoadCallbackStub {
    public:
        LoadDMSDPServiceCallback() = default;
        ~LoadDMSDPServiceCallback() override = default;

        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
    };

protected:
    void OnStart() override;
    void OnStop() override;
    int32_t OnIdle(const SystemAbilityOnDemandReason& idleReason) override;

private:
    bool Init();
    std::string QueryDhSysSpec(const std::string &targetKey, std::string &attrs);
    void InitLocalDevInfo();
    bool DoBusinessInit();
    bool IsDepSAStart();
    int32_t GetDeviceDhInfo(const std::string &realNetworkId, const std::string &udidHash, const std::string &deviceId,
        EnableStep enableStep, const sptr<IGetDhDescriptorsCallback> callback);

private:
    bool registerToService_ = false;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif