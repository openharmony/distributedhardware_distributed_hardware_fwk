/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_PROXY_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_PROXY_TEST_H

#include <cstdint>
#include <gtest/gtest.h>
#include <mutex>

#include "av_trans_errno.h"
#include "device_type.h"
#include "distributed_hardware_errno.h"
#include "idistributed_hardware.h"
#include "distributed_hardware_proxy.h"
#include "distributed_hardware_stub.h"
#include "get_dh_descriptors_callback_stub.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

    class TestDistributedHardwareStub : public
        OHOS::DistributedHardware::DistributedHardwareStub {
    public:
        TestDistributedHardwareStub() = default;
        virtual ~TestDistributedHardwareStub() = default;
        int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener);
        int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener);
        int32_t PublishMessage(const DHTopic topic, const std::string &msg);
        std::string QueryLocalSysSpec(QueryLocalSysSpecType spec);

        int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId);
        int32_t ReleaseAVCenter(int32_t engineId);
        int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId);
        int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event);
        int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAvTransControlCenterCallback> callback);
        int32_t NotifySourceRemoteSinkStarted(std::string &deviceId);
        int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId);
        int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId);
        int32_t StopDistributedHardware(DHType dhType, const std::string &networkId);
        int32_t GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
            const sptr<IGetDhDescriptorsCallback> callback);
        int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener);
        int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener);
        int32_t RegisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener);
        int32_t UnregisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener);
        int32_t EnableSink(const std::vector<DHDescriptor> &descriptors);
        int32_t DisableSink(const std::vector<DHDescriptor> &descriptors);
        int32_t EnableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors);
        int32_t DisableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors);
        int32_t LoadDistributedHDF(const DHType dhType);
        int32_t UnLoadDistributedHDF(const DHType dhType);
        int32_t LoadSinkDMSDPService(const std::string &udid);
        int32_t NotifySinkRemoteSourceStarted(const std::string &udid);
        int32_t RegisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
            int32_t &timeOut, const std::string &pkgName);
        int32_t UnregisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
            const std::string &pkgName);
        void SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool &granted);
    };

    class TestDistributedHardwareStub2 : public TestDistributedHardwareStub {
    public:
        int32_t OnRemoteRequest(uint32_t code,
            MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    };

    class TestGetDistributedHardwareCallback : public GetDhDescriptorsCallbackStub {
    public:
        TestGetDistributedHardwareCallback() = default;
        virtual ~TestGetDistributedHardwareCallback() = default;
    protected:
        void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
            EnableStep enableStep) override;
        void OnError(const std::string &networkId, int32_t error) override;
    };
};

class MockIPublisherListener : public IPublisherListener {
public:
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    void OnMessage(const DHTopic topic, const std::string& message)
    {
        (void)topic;
        (void)message;
    }
};

class MockHDSinkStatusListenerStub : public IRemoteStub<IHDSinkStatusListener> {
public:
    void OnEnable(const DHDescriptor &dhDescriptor) override
    {
        (void)dhDescriptor;
    }

    void OnDisable(const DHDescriptor &dhDescriptor) override
    {
        (void)dhDescriptor;
    }
};

class MockHDSourceStatusListenerStub : public IRemoteStub<IHDSourceStatusListener> {
public:
    void OnEnable(const std::string &networkId, const DHDescriptor &dhDescriptor) override
    {
        (void)networkId;
        (void)dhDescriptor;
    }

    void OnDisable(const std::string &networkId, const DHDescriptor &dhDescriptor) override
    {
        (void)networkId;
        (void)dhDescriptor;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_PROXY_TEST_H