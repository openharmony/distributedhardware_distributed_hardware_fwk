/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
};

class MockDistributedHardwareStub : public DistributedHardwareStub {
public:
int32_t RegisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> &listener)
{
    (void)topic;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t UnregisterPublisherListener(const DHTopic topic, const sptr<IPublisherListener> &listener)
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

int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAVTransControlCenterCallback> &callback)
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
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
