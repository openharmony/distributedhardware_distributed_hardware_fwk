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

#ifndef OHOS_DISTRIBUTED_HARDWARE_STUB_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_STUB_TEST_H

#include <gtest/gtest.h>

#include "accesstoken_kit_mock.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_fwk_kit_paras.h"
#include "distributed_hardware_stub.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedHardwareStub> stubTest_ = nullptr;
    std::shared_ptr<AccessTokenKitMock> token_ = nullptr;
};

class MockDistributedHardwareStub : public DistributedHardwareStub {
public:
int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener)
{
    (void)topic;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> listener)
{
    (void)topic;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t PublishMessage(const DHTopic topic, const std::string &msg)
{
    (void)topic;
    (void)msg;
    return DH_FWK_SUCCESS;
}

std::string QueryLocalSysSpec(QueryLocalSysSpecType spec)
{
    (void)spec;
    return "";
}

int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId)
{
    (void)transRole;
    (void)engineId;
    return DH_FWK_SUCCESS;
}

int32_t ReleaseAVCenter(int32_t engineId)
{
    (void)engineId;
    return DH_FWK_SUCCESS;
}

int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    (void)engineId;
    (void)peerDevId;
    return DH_FWK_SUCCESS;
}

int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event)
{
    (void)engineId;
    (void)event;
    return DH_FWK_SUCCESS;
}

int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAvTransControlCenterCallback> callback)
{
    (void)engineId;
    (void)callback;
    return DH_FWK_SUCCESS;
}

int32_t NotifySourceRemoteSinkStarted(std::string &deviceId)
{
    (void)deviceId;
    return DH_FWK_SUCCESS;
}

int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId)
{
    (void)dhType;
    (void)networkId;
    return DH_FWK_SUCCESS;
}

int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId)
{
    (void)dhType;
    (void)networkId;
    return DH_FWK_SUCCESS;
}

int32_t StopDistributedHardware(DHType dhType, const std::string &networkId)
{
    (void)dhType;
    (void)networkId;
    return DH_FWK_SUCCESS;
}

int32_t GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
    const sptr<IGetDhDescriptorsCallback> callback)
{
    (void)networkId;
    (void)enableStep;
    (void)callback;
    return DH_FWK_SUCCESS;
}

int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener)
{
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t RegisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    (void)networkId;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t UnregisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    (void)networkId;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t EnableSink(const std::vector<DHDescriptor> &descriptors)
{
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t DisableSink(const std::vector<DHDescriptor> &descriptors)
{
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t EnableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    (void)networkId;
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t DisableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    (void)networkId;
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t LoadDistributedHDF(const DHType dhType)
{
    (void)dhType;
    return DH_FWK_SUCCESS;
}

int32_t UnLoadDistributedHDF(const DHType dhType)
{
    (void)dhType;
    return DH_FWK_SUCCESS;
}

int32_t LoadSinkDMSDPService(const std::string &udid)
{
    (void)udid;
    return DH_FWK_SUCCESS;
}

int32_t NotifySinkRemoteSourceStarted(const std::string &udid)
{
    (void)udid;
    return DH_FWK_SUCCESS;
}

int32_t RegisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
    int32_t &timeOut, const std::string &pkgName)
{
    (void)dhType;
    (void)callback;
    (void)timeOut;
    (void)pkgName;
    return DH_FWK_SUCCESS;
}
int32_t UnregisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
    const std::string &pkgName)
{
    (void)dhType;
    (void)callback;
    (void)pkgName;
    return DH_FWK_SUCCESS;
}
void SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool &granted)
{
    (void)dhType;
    (void)requestId;
    (void)granted;
    return;
}
};

class MockGetDhDescriptorsCallbackStub : public IRemoteStub<IGetDhDescriptorsCallback> {
public:
    void OnSuccess(const std::string &networkId, const std::vector<DHDescriptor> &descriptors,
        EnableStep enableStep) override
    {
        (void)networkId;
        (void)descriptors;
        (void)enableStep;
    }
    void OnError(const std::string &networkId, int32_t error) override
    {
        (void)networkId;
        (void)error;
    }
};

class MockIPublisherListener : public IRemoteStub<IPublisherListener> {
public:
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
#endif
