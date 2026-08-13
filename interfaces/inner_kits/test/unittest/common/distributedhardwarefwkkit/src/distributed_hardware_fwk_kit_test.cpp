/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "distributed_hardware_fwk_kit_test.h"

#include <cstdint>
#include <chrono>
#include <cinttypes>
#include <thread>

#include "anonymous_string.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"

#include "distributed_hardware_errno.h"
#include "dhfwk_sa_manager.h"
#include "distributed_hardware_fwk_kit.h"
#include "distributed_hardware_log.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

using OHOS::DistributedHardware::DHTopic;

namespace OHOS {
namespace DistributedHardware {
namespace {
    std::map<DHTopic, std::string> TOPIC_MSGS = {
        { DHTopic::TOPIC_START_DSCREEN, "Start DScreen" },
        { DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, "Sink Project Window" },
        { DHTopic::TOPIC_STOP_DSCREEN, "Stop DScreen" },
        { DHTopic::TOPIC_DEV_OFFLINE, "Dev Offline" }
    };
    const std::string DHARDWARE_PROC_NAME = "dhardware";
}
void DistributedHardwareFwkKitTest::SetUp()
{
    dhfwkPtr_ = std::make_shared<DistributedHardwareFwkKit>();
}

void DistributedHardwareFwkKitTest::TearDown()
{
    if (dhfwkPtr_ != nullptr) {
        dhfwkPtr_.reset();
        dhfwkPtr_ = nullptr;
    }
}

void DistributedHardwareFwkKitTest::SetUpTestCase()
{
}

void DistributedHardwareFwkKitTest::TearDownTestCase()
{
}

void DistributedHardwareFwkKitTest::TestPublisherListener::OnMessage(const DHTopic topic, const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    msgCnts_[topic]++;
}

uint32_t DistributedHardwareFwkKitTest::TestPublisherListener::GetTopicMsgCnt(const DHTopic topic)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return msgCnts_[topic];
}

void DistributedHardwareFwkKitTest::TestHDSinkStatusListener::OnEnable(const DHDescriptor &dhDescriptor)
{
    (void)dhDescriptor;
}

void DistributedHardwareFwkKitTest::TestHDSinkStatusListener::OnDisable(const DHDescriptor &dhDescriptor)
{
    (void)dhDescriptor;
}

void DistributedHardwareFwkKitTest::TestHDSourceStatusListener::OnEnable(
    const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    (void)networkId;
    (void)dhDescriptor;
}

void DistributedHardwareFwkKitTest::TestHDSourceStatusListener::OnDisable(
    const std::string &networkId, const DHDescriptor &dhDescriptor)
{
    (void)networkId;
    (void)dhDescriptor;
}

void DistributedHardwareFwkKitTest::TestDmInitCallback::OnRemoteDied()
{
}

void DistributedHardwareFwkKitTest::TestGetDistributedHardwareCallback::OnSuccess(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    (void)networkId;
    (void)descriptors;
    (void)enableStep;
}

void DistributedHardwareFwkKitTest::TestGetDistributedHardwareCallback::OnError(
    const std::string &networkId, int32_t error)
{
    (void)networkId;
    (void)error;
}

/**
 * @tc.name: RegisterPublisherListener_001
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, RegisterPublisherListener_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    sptr<TestPublisherListener> listener1(new TestPublisherListener());
    int32_t ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    sptr<TestPublisherListener> listener2(new TestPublisherListener());
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener2);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    sptr<TestPublisherListener> listener3(new TestPublisherListener());
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener3);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    sptr<TestPublisherListener> listener4(new TestPublisherListener());
    ret = dhfwkPtr_->RegisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener4);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_START_DSCREEN, listener1);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_SINK_PROJECT_WINDOW_INFO, listener2);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_STOP_DSCREEN, listener3);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    ret = dhfwkPtr_->UnregisterPublisherListener(DHTopic::TOPIC_DEV_OFFLINE, listener4);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: PublishMessage_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, PublishMessage_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    uint32_t invalid = 7;
    DHTopic topic = static_cast<DHTopic>(invalid);
    std::string message;
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, dhfwkPtr_->PublishMessage(topic, message));
}

/**
 * @tc.name: PublishMessage_002
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, PublishMessage_002, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    std::string message;
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, dhfwkPtr_->PublishMessage(topic, message));
}

/**
 * @tc.name: PublishMessage_003
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, PublishMessage_003, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    std::string message = "TOPIC_STOP_DSCREEN";
    EXPECT_EQ(ERR_DH_FWK_PUBLISH_MSG_FAILED, dhfwkPtr_->PublishMessage(topic, message));
}

/**
 * @tc.name: PublishMessage_004
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, PublishMessage_004, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    std::string message;
    uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024 + 10;
    message.resize(MAX_MESSAGE_LEN);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, dhfwkPtr_->PublishMessage(topic, message));
}

/**
 * @tc.name: RegisterPublisherListener_002
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, RegisterPublisherListener_002, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHTopic topic = DHTopic::TOPIC_MIN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = dhfwkPtr_->RegisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: RegisterPublisherListener_003
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, RegisterPublisherListener_003, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = dhfwkPtr_->RegisterPublisherListener(topic, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_001
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, UnregisterPublisherListener_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHTopic topic = DHTopic::TOPIC_MIN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = dhfwkPtr_->UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_002
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, UnregisterPublisherListener_002, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHTopic topic = DHTopic::TOPIC_START_DSCREEN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = dhfwkPtr_->UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: QueryLocalSysSpec_001
 * @tc.desc: Verify the QueryLocalSysSpec function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, QueryLocalSysSpec_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    QueryLocalSysSpecType spec = QueryLocalSysSpecType::MIN;
    std::string ret = dhfwkPtr_->QueryLocalSysSpec(spec);
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: QueryLocalSysSpec_002
 * @tc.desc: Verify the QueryLocalSysSpec function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, QueryLocalSysSpec_002, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    QueryLocalSysSpecType spec = QueryLocalSysSpecType::HISTREAMER_AUDIO_ENCODER;
    std::string ret = dhfwkPtr_->QueryLocalSysSpec(spec);
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: InitializeAVCenter_001
 * @tc.desc: Verify the InitializeAVCenter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, InitializeAVCenter_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    TransRole transRole = TransRole::UNKNOWN;
    int32_t engineId = 0;
    int32_t ret = dhfwkPtr_->InitializeAVCenter(transRole, engineId);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: ReleaseAVCenter_001
 * @tc.desc: Verify the ReleaseAVCenter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, ReleaseAVCenter_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    int32_t engineId = 0;
    int32_t ret = dhfwkPtr_->ReleaseAVCenter(engineId);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: CreateControlChannel_001
 * @tc.desc: Verify the CreateControlChannel function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, CreateControlChannel_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    int32_t engineId = 0;
    std::string peerDevId = "peerDevId_test";
    int32_t ret = dhfwkPtr_->CreateControlChannel(engineId, peerDevId);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: NotifyAVCenter_001
 * @tc.desc: Verify the NotifyAVCenter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, NotifyAVCenter_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    int32_t engineId = 0;
    AVTransEvent event;
    int32_t ret = dhfwkPtr_->NotifyAVCenter(engineId, event);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: RegisterCtlCenterCallback_001
 * @tc.desc: Verify the RegisterCtlCenterCallback function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, RegisterCtlCenterCallback_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    int32_t engineId = 0;
    sptr<IAvTransControlCenterCallback> callback = nullptr;
    int32_t ret = dhfwkPtr_->RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: PauseDistributedHardware_001
 * @tc.desc: Verify the PauseDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, PauseDistributedHardware_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHType dhType = DHType::CAMERA;
    std::string networkId = "";
    int32_t ret = dhfwkPtr_->PauseDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = dhfwkPtr_->PauseDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: ResumeDistributedHardware_001
 * @tc.desc: Verify the ResumeDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, ResumeDistributedHardware_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHType dhType = DHType::CAMERA;
    std::string networkId = "";
    int32_t ret = dhfwkPtr_->ResumeDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = dhfwkPtr_->ResumeDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: StopDistributedHardware_001
 * @tc.desc: Verify the StopDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, StopDistributedHardware_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    DHType dhType = DHType::CAMERA;
    std::string networkId = "";
    int32_t ret = dhfwkPtr_->StopDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = dhfwkPtr_->StopDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: RegisterDHStatusListener_001
 * @tc.desc: Verify the RegisterDHStatusListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, RegisterDHStatusListener_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    sptr<IHDSinkStatusListener> listener(new TestHDSinkStatusListener());
    int32_t ret = dhfwkPtr_->RegisterDHStatusListener(listener);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: UnregisterDHStatusListener_001
 * @tc.desc: Verify the UnregisterDHStatusListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, UnregisterDHStatusListener_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    sptr<IHDSinkStatusListener> listener(new TestHDSinkStatusListener());
    int32_t ret = dhfwkPtr_->UnregisterDHStatusListener(listener);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: UnregisterDHStatusListener_002
 * @tc.desc: Verify the UnregisterDHStatusListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, UnregisterDHStatusListener_002, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    std::string networkId = "";
    sptr<IHDSourceStatusListener> listener(new TestHDSourceStatusListener());
    int32_t ret = dhfwkPtr_->UnregisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = dhfwkPtr_->UnregisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: EnableSink_001
 * @tc.desc: Verify the EnableSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, EnableSink_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    std::vector<DHDescriptor> descriptors;
    int32_t ret = dhfwkPtr_->EnableSink(descriptors);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: DisableSink_001
 * @tc.desc: Verify the DisableSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, DisableSink_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    std::vector<DHDescriptor> descriptors;
    int32_t ret = dhfwkPtr_->DisableSink(descriptors);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: EnableSource_001
 * @tc.desc: Verify the EnableSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, EnableSource_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    std::string networkId = "";
    std::vector<DHDescriptor> descriptors;
    int32_t ret = dhfwkPtr_->EnableSource(networkId, descriptors);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = dhfwkPtr_->EnableSource(networkId, descriptors);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: DisableSource_001
 * @tc.desc: Verify the DisableSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, DisableSource_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    std::string networkId = "";
    std::vector<DHDescriptor> descriptors;
    int32_t ret = dhfwkPtr_->DisableSource(networkId, descriptors);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "networkId_test";
    ret = dhfwkPtr_->DisableSource(networkId, descriptors);
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);
}

/**
 * @tc.name: LoadDistributedHDF_001
 * @tc.desc: Verify the LoadDistributedHDF function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, LoadDistributedHDF_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    int32_t ret = dhfwkPtr_->LoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: UnLoadDistributedHDF_001
 * @tc.desc: Verify the UnLoadDistributedHDF function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, UnLoadDistributedHDF_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    int32_t ret = dhfwkPtr_->UnLoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: GetDistributedHardware_001
 * @tc.desc: Verify the GetDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, GetDistributedHardware_001, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    std::string networkId = "";
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    int32_t ret = dhfwkPtr_->GetDistributedHardware(networkId, enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    sptr<IHDSourceStatusListener> listener(new TestHDSourceStatusListener());
    ret = dhfwkPtr_->RegisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: GetDistributedHardware_001
 * @tc.desc: Verify the GetDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareFwkKitTest, GetDistributedHardware_002, testing::ext::TestSize.Level0)
{
    ASSERT_TRUE(dhfwkPtr_ != nullptr);
    std::string networkId = "networkId_test";
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    int32_t ret = dhfwkPtr_->GetDistributedHardware(networkId, enableStep, callback);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL, ret);

    sptr<IHDSourceStatusListener> listener(new TestHDSourceStatusListener());
    ret = dhfwkPtr_->RegisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL, ret);
}
} // namespace DistributedHardware
} // namespace OHOS