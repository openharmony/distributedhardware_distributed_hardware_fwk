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

#include "distributed_hardware_proxy_test.h"

#include "dhardware_ipc_interface_code.h"
#include "av_trans_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DistributedHardwareProxyTest::SetUpTestCase()
{
}

void DistributedHardwareProxyTest::TearDownTestCase()
{
}

void DistributedHardwareProxyTest::SetUp() {}

void DistributedHardwareProxyTest::TearDown() {}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub2::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    if (code == static_cast<uint32_t>(DHMsgInterfaceCode::GET_DISTRIBUTED_HARDWARE)) {
        return DH_FWK_SUCCESS;
    }
    return OHOS::DistributedHardware::DistributedHardwareStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::RegisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> listener)
{
    (void)topic;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> listener)
{
    (void)topic;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::PublishMessage(const DHTopic topic,
    const std::string &msg)
{
    (void)topic;
    (void)msg;
    return DH_FWK_SUCCESS;
}

std::string DistributedHardwareProxyTest::TestDistributedHardwareStub::QueryLocalSysSpec(QueryLocalSysSpecType spec)
{
    (void)spec;
    return "";
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::InitializeAVCenter(const TransRole &transRole,
    int32_t &engineId)
{
    (void)transRole;
    (void)engineId;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::ReleaseAVCenter(int32_t engineId)
{
    (void)engineId;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::CreateControlChannel(int32_t engineId,
    const std::string &peerDevId)
{
    (void)engineId;
    (void)peerDevId;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::NotifyAVCenter(int32_t engineId,
    const AVTransEvent &event)
{
    (void)engineId;
    (void)event;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAvTransControlCenterCallback> callback)
{
    (void)engineId;
    (void)callback;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::NotifySourceRemoteSinkStarted(std::string &deviceId)
{
    (void)deviceId;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::PauseDistributedHardware(DHType dhType,
    const std::string &networkId)
{
    (void)dhType;
    (void)networkId;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::ResumeDistributedHardware(DHType dhType,
    const std::string &networkId)
{
    (void)dhType;
    (void)networkId;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::StopDistributedHardware(DHType dhType,
    const std::string &networkId)
{
    (void)dhType;
    (void)networkId;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::GetDistributedHardware(
    const std::string &networkId, EnableStep enableStep, const sptr<IGetDhDescriptorsCallback> callback)
{
    (void)networkId;
    (void)enableStep;
    (void)callback;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::RegisterDHStatusListener(
    sptr<IHDSinkStatusListener> listener)
{
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::UnregisterDHStatusListener(
    sptr<IHDSinkStatusListener> listener)
{
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::RegisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    (void)networkId;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::UnregisterDHStatusListener(
    const std::string &networkId, sptr<IHDSourceStatusListener> listener)
{
    (void)networkId;
    (void)listener;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::EnableSink(
    const std::vector<DHDescriptor> &descriptors)
{
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::DisableSink(
    const std::vector<DHDescriptor> &descriptors)
{
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::EnableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    (void)networkId;
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::DisableSource(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors)
{
    (void)networkId;
    (void)descriptors;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::LoadDistributedHDF(const DHType dhType)
{
    (void)dhType;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::UnLoadDistributedHDF(const DHType dhType)
{
    (void)dhType;
    return DH_FWK_SUCCESS;
}

void DistributedHardwareProxyTest::TestGetDistributedHardwareCallback::OnSuccess(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    (void)networkId;
    (void)descriptors;
    (void)enableStep;
}

void DistributedHardwareProxyTest::TestGetDistributedHardwareCallback::OnError(
    const std::string &networkId, int32_t error)
{
    (void)networkId;
    (void)error;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::LoadSinkDMSDPService(const std::string &udid)
{
    (void)udid;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::NotifySinkRemoteSourceStarted(
    const std::string &udid)
{
    (void)udid;
    return DH_FWK_SUCCESS;
}

int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::RegisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, int32_t &timeOut, const std::string &pkgName)
{
    (void)dhType;
    (void)callback;
    (void)timeOut;
    (void)pkgName;
    return DH_FWK_SUCCESS;
}
int32_t DistributedHardwareProxyTest::TestDistributedHardwareStub::UnregisterHardwareAccessListener(const DHType dhType,
    sptr<IAuthorizationResultCallback> callback, const std::string &pkgName)
{
    (void)dhType;
    (void)callback;
    (void)pkgName;
    return DH_FWK_SUCCESS;
}
void DistributedHardwareProxyTest::TestDistributedHardwareStub::SetAuthorizationResult(const DHType dhType,
    const std::string &requestId, bool &granted)
{
    (void)dhType;
    (void)requestId;
    (void)granted;
    return;
}

/**
 * @tc.name: RegisterPublisherListener_001
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterPublisherListener_001, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_MIN;
    sptr<IPublisherListener> listener = nullptr;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.RegisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL, ret);

    sptr<IPublisherListener> listener1(new MockIPublisherListener());
    ASSERT_TRUE(listener1 != nullptr);
    ret = dhProxy.RegisterPublisherListener(topic, listener1);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    topic = DHTopic::TOPIC_MAX;
    ret = dhProxy.RegisterPublisherListener(topic, listener1);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: RegisterAbilityListener_002
 * @tc.desc: Verify the RegisterAbilityListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterPublisherListener_002, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_LOW_LATENCY;
    sptr<IPublisherListener> listener(new MockIPublisherListener());
    ASSERT_TRUE(listener != nullptr);
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.RegisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_001
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_001, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_MIN;
    sptr<IPublisherListener> listener = nullptr;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL, ret);

    sptr<IPublisherListener> listener1(new MockIPublisherListener());
    ASSERT_TRUE(listener1 != nullptr);
    ret = dhProxy.UnregisterPublisherListener(topic, listener1);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    topic = DHTopic::TOPIC_MAX;
    ret = dhProxy.RegisterPublisherListener(topic, listener1);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_002
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_003, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_LOW_LATENCY;
    sptr<IPublisherListener> listener(new MockIPublisherListener());
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    int32_t ret = dhProxy.UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL, ret);
}

/**
 * @tc.name: PublishMessage_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_001, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_MIN;
    std::string msg = "";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    topic = DHTopic::TOPIC_MAX;
    ret = dhProxy.PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: PublishMessage_002
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_002, TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_LOW_LATENCY;
    std::string msg = "";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_MSG_INVALID, ret);

    msg = "msg_test";
    ret = dhProxy.PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: QueryLocalSysSpec_001
 * @tc.desc: Verify the QueryLocalSysSpec function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, QueryLocalSysSpec_001, TestSize.Level1)
{
    QueryLocalSysSpecType spec = QueryLocalSysSpecType::MIN;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.QueryLocalSysSpec(spec);
    EXPECT_EQ("", ret);

    spec = QueryLocalSysSpecType::MAX;
    ret = dhProxy.QueryLocalSysSpec(spec);
    EXPECT_EQ("", ret);
}

HWTEST_F(DistributedHardwareProxyTest, QueryLocalSysSpec_002, TestSize.Level1)
{
    QueryLocalSysSpecType spec = QueryLocalSysSpecType::HISTREAMER_AUDIO_ENCODER;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.QueryLocalSysSpec(spec);
    EXPECT_EQ("", ret);
}

/**
 * @tc.name: InitializeAVCenter_001
 * @tc.desc: Verify the InitializeAVCenter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, InitializeAVCenter_001, TestSize.Level1)
{
    TransRole transRole = TransRole::UNKNOWN;
    int32_t engineId = 0;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.InitializeAVCenter(transRole, engineId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: ReleaseAVCenter_001
 * @tc.desc: Verify the ReleaseAVCenter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, ReleaseAVCenter_001, TestSize.Level1)
{
    int32_t engineId = 0;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.ReleaseAVCenter(engineId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: CreateControlChannel_001
 * @tc.desc: Verify the CreateControlChannel function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, CreateControlChannel_001, TestSize.Level1)
{
    int32_t engineId = 0;
    std::string peerDevId = "peerDevId_test";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.CreateControlChannel(engineId, peerDevId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: NotifyAVCenter_001
 * @tc.desc: Verify the NotifyAVCenter function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, NotifyAVCenter_001, TestSize.Level1)
{
    int32_t engineId = 0;
    AVTransEvent event;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.NotifyAVCenter(engineId, event);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: RegisterCtlCenterCallback_001
 * @tc.desc: Verify the RegisterCtlCenterCallback function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterCtlCenterCallback_001, TestSize.Level1)
{
    int32_t engineId = 0;
    sptr<IAvTransControlCenterCallback> callback = nullptr;
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.RegisterCtlCenterCallback(engineId, callback);
    EXPECT_EQ(ERR_DH_FWK_AVTRANS_CALLBACK_IS_NULL, ret);
}

/**
 * @tc.name: NotifySourceRemoteSinkStarted_001
 * @tc.desc: Verify the NotifySourceRemoteSinkStarted function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, NotifySourceRemoteSinkStarted_001, TestSize.Level1)
{
    std::string deviceId = "devid_test";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.NotifySourceRemoteSinkStarted(deviceId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: PauseDistributedHardware_001
 * @tc.desc: Verify the PauseDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PauseDistributedHardware_001, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    std::string networkId = "";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.PauseDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "123456789";
    ret = dhProxy.PauseDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: ResumeDistributedHardware_001
 * @tc.desc: Verify the ResumeDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, ResumeDistributedHardware_001, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    std::string networkId = "";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.ResumeDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "123456789";
    ret = dhProxy.ResumeDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

/**
 * @tc.name: StopDistributedHardware_001
 * @tc.desc: Verify the StopDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, StopDistributedHardware_001, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    std::string networkId = "";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.StopDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    networkId = "123456789";
    ret = dhProxy.StopDistributedHardware(dhType, networkId);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, GetDistributedHardware_001, TestSize.Level1)
{
    std::string networkId = "123456";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    ASSERT_TRUE(callback != nullptr);
    auto ret = dhProxy.GetDistributedHardware(networkId, enableStep, callback);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
    ret = dhProxy.GetDistributedHardware(std::string(), enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DistributedHardwareProxyTest, GetDistributedHardware_002, TestSize.Level1)
{
    std::string networkId = "123456";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub2());
    ASSERT_TRUE(dhStubPtr != nullptr);
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    ASSERT_TRUE(callback != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.GetDistributedHardware(networkId, enableStep, callback);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DistributedHardwareProxyTest, RegisterDHStatusListener_Source_001, TestSize.Level1)
{
    std::string networkId = "123456";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSourceStatusListener> listener(new MockHDSourceStatusListenerStub());
    auto ret = dhProxy.RegisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
    ret = dhProxy.RegisterDHStatusListener(std::string(), listener);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DistributedHardwareProxyTest, RegisterDHStatusListener_Source_002, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSourceStatusListener> listener(new MockHDSourceStatusListenerStub());
    auto ret = dhProxy.RegisterDHStatusListener(std::string(), nullptr);
    EXPECT_EQ(ERR_DH_FWK_STATUS_LISTENER_IS_NULL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, UnregisterDHStatusListener_Source_001, TestSize.Level1)
{
    std::string networkId = "123456";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSourceStatusListener> listener(new MockHDSourceStatusListenerStub());
    auto ret = dhProxy.UnregisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
    ret = dhProxy.UnregisterDHStatusListener(std::string(), listener);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(DistributedHardwareProxyTest, UnregisterDHStatusListener_Source_002, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSourceStatusListener> listener(new MockHDSourceStatusListenerStub());
    auto ret = dhProxy.UnregisterDHStatusListener(std::string(), nullptr);
    EXPECT_EQ(ERR_DH_FWK_STATUS_LISTENER_IS_NULL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, RegisterDHStatusListener_Sink_001, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSinkStatusListener> listener(new MockHDSinkStatusListenerStub());
    auto ret = dhProxy.RegisterDHStatusListener(listener);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, RegisterDHStatusListener_Sink_002, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSinkStatusListener> listener(new MockHDSinkStatusListenerStub());
    auto ret = dhProxy.RegisterDHStatusListener(nullptr);
    EXPECT_EQ(ERR_DH_FWK_STATUS_LISTENER_IS_NULL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, UnregisterDHStatusListener_Sink_001, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSinkStatusListener> listener(new MockHDSinkStatusListenerStub());
    auto ret = dhProxy.UnregisterDHStatusListener(listener);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, UnregisterDHStatusListener_Sink_002, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    sptr<IHDSinkStatusListener> listener(new MockHDSinkStatusListenerStub());
    auto ret = dhProxy.UnregisterDHStatusListener(nullptr);
    EXPECT_EQ(ERR_DH_FWK_STATUS_LISTENER_IS_NULL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, EnableSink_001, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);

    std::vector<DHDescriptor> descriptors = {{ .id = "camera_1", .dhType = DHType::CAMERA }};
    auto ret = dhProxy.EnableSink(descriptors);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, DisableSink_001, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);

    std::vector<DHDescriptor> descriptors = {{ .id = "camera_1", .dhType = DHType::CAMERA }};
    auto ret = dhProxy.DisableSink(descriptors);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, EnableSource_001, TestSize.Level1)
{
    std::string networkId = "123456";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);

    std::vector<DHDescriptor> descriptors;
    auto ret = dhProxy.EnableSource(networkId, descriptors);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, DisableSource_001, TestSize.Level1)
{
    std::string networkId = "123456";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);

    std::vector<DHDescriptor> descriptors;
    auto ret = dhProxy.DisableSource(networkId, descriptors);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, LoadDistributedHDF_001, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);

    auto ret = dhProxy.LoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, LoadDistributedHDF_002, TestSize.Level1)
{
    DistributedHardwareProxy dhProxy(nullptr);
    auto ret = dhProxy.LoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, UnLoadDistributedHDF_001, TestSize.Level1)
{
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);

    auto ret = dhProxy.UnLoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, UnLoadDistributedHDF_002, TestSize.Level1)
{
    DistributedHardwareProxy dhProxy(nullptr);
    auto ret = dhProxy.UnLoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_REMOTE_IS_NULL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, LoadSinkDMSDPService_001, TestSize.Level1)
{
    std::string udid = "";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.LoadSinkDMSDPService(udid);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, LoadSinkDMSDPService_002, TestSize.Level1)
{
    std::string udid = "udid_123";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.LoadSinkDMSDPService(udid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DistributedHardwareProxyTest, NotifySinkRemoteSourceStarted_001, TestSize.Level1)
{
    std::string udid = "";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.NotifySinkRemoteSourceStarted(udid);
    EXPECT_EQ(ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL, ret);
}

HWTEST_F(DistributedHardwareProxyTest, NotifySinkRemoteSourceStarted_002, TestSize.Level1)
{
    std::string udid = "udid_123";
    sptr<IRemoteObject> dhStubPtr(new TestDistributedHardwareStub());
    ASSERT_TRUE(dhStubPtr != nullptr);
    DistributedHardwareProxy dhProxy(dhStubPtr);
    auto ret = dhProxy.NotifySinkRemoteSourceStarted(udid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
