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

#include "distributed_hardware_proxy_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DistributedHardwareProxyTest::SetUpTestCase()
{
}

void DistributedHardwareProxyTest::TearDownTestCase()
{
}

void DistributedHardwareProxyTest::SetUp()
{
    sptr<IRemoteObject> impl = nullptr;
    hardwareProxy_ = std::make_shared<DistributedHardwareProxy>(impl);
}

void DistributedHardwareProxyTest::TearDown()
{
    hardwareProxy_ = nullptr;
}

/**
 * @tc.name: RegisterPublisherListener_001
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterPublisherListener_001, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = hardwareProxy_->RegisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL, ret);
}

/**
 * @tc.name: RegisterAbilityListener_002
 * @tc.desc: Verify the RegisterAbilityListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterAbilityListener_002, TestSize.Level0)
{
    int32_t invalid = 7;
    DHTopic topic = static_cast<DHTopic>(invalid);
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->RegisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: RegisterAbilityListener_003
 * @tc.desc: Verify the RegisterAbilityListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterAbilityListener_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->RegisterPublisherListener(topic, listener);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_001
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_001, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = hardwareProxy_->UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_002
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_002, TestSize.Level0)
{
    int32_t invalid = 7;
    DHTopic topic = static_cast<DHTopic>(invalid);
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_003
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->UnregisterPublisherListener(topic, listener);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: PublishMessage_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_001, TestSize.Level0)
{
    int32_t invalid = 7;
    std::string msg;
    DHTopic topic = static_cast<DHTopic>(invalid);
    int32_t ret = hardwareProxy_->PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: PublishMessage_002
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_002, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    std::string msg;
    int32_t ret = hardwareProxy_->PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: PublishMessage_003
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    std::string msg = "msg";
    int32_t ret = hardwareProxy_->PublishMessage(topic, msg);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: QueryLocalSysSpec_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, QueryLocalSysSpec_001, TestSize.Level0)
{
    QueryLocalSysSpecType spec = QueryLocalSysSpecType::MIN;
    std::string ret = hardwareProxy_->QueryLocalSysSpec(spec);
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: InitializeAVCenter_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, InitializeAVCenter_001, TestSize.Level0)
{
    TransRole transRole = TransRole::UNKNOWN;;
    int32_t engineId = 0;
    int32_t ret = hardwareProxy_->InitializeAVCenter(transRole, engineId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: ReleaseAVCenter_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, ReleaseAVCenter_001, TestSize.Level0)
{
    int32_t engineId = 0;
    int32_t ret = hardwareProxy_->ReleaseAVCenter(engineId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: CreateControlChannel_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, CreateControlChannel_001, TestSize.Level0)
{
    int32_t engineId = 0;
    std::string peerDevId = "peerDevId_test";
    int32_t ret = hardwareProxy_->CreateControlChannel(engineId, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: NotifyAVCenter_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, NotifyAVCenter_001, TestSize.Level0)
{
    int32_t engineId = 0;
    AVTransEvent event;
    int32_t ret = hardwareProxy_->NotifyAVCenter(engineId, event);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: NotifySourceRemoteSinkStarted_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, NotifySourceRemoteSinkStarted_001, TestSize.Level0)
{
    std::string deviceId = "devid_test";
    int32_t ret = hardwareProxy_->NotifySourceRemoteSinkStarted(deviceId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: PauseDistributedHardware_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PauseDistributedHardware_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string networkId = "networkId_test";
    int32_t ret = hardwareProxy_->PauseDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: ResumeDistributedHardware_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, ResumeDistributedHardware_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string networkId = "networkId_test";
    int32_t ret = hardwareProxy_->ResumeDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: StopDistributedHardware_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, StopDistributedHardware_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string networkId = "networkId_test";
    int32_t ret = hardwareProxy_->StopDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
