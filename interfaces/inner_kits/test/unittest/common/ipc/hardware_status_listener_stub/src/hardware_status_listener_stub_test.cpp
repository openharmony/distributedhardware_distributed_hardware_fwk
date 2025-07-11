/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "hardware_status_listener_stub_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
constexpr uint32_t MAX_DH_DESCRIPTOR_ARRAY_SIZE = 4100;
constexpr uint32_t INVALID_CODE = 100;

void HardwareStatusListenerStubTest::SetUpTestCase()
{
}

void HardwareStatusListenerStubTest::TearDownTestCase()
{
}

void HardwareStatusListenerStubTest::SetUp()
{
    sinkListenerStub_ = std::make_shared<MockHDSinkStatusListenerStub>();
    sourceListenerStub_ = std::make_shared<MockHDSourceStatusListenerStub>();
    getDHCallbackStub_ = std::make_shared<TestGetDhDescriptorsCallbackStub>();
}

void HardwareStatusListenerStubTest::TearDown()
{
    sinkListenerStub_ = nullptr;
    sourceListenerStub_ = nullptr;
    getDHCallbackStub_ = nullptr;
}

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_001, TestSize.Level0)
{
    ASSERT_TRUE(sinkListenerStub_ != nullptr);
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(ERR_INVALID_DATA, sinkListenerStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: OnRemoteRequest_002
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_002, TestSize.Level0)
{
    ASSERT_TRUE(sinkListenerStub_ != nullptr);
    uint32_t code = static_cast<uint32_t>(IHDSinkStatusListener::Message::ON_ENABLE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(sinkListenerStub_->GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(static_cast<int32_t>(DHType::AUDIO));
    data.WriteString("id_test");
    EXPECT_EQ(DH_FWK_SUCCESS, sinkListenerStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: OnRemoteRequest_002
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_003, TestSize.Level0)
{
    ASSERT_TRUE(sinkListenerStub_ != nullptr);
    uint32_t code = static_cast<uint32_t>(IHDSinkStatusListener::Message::ON_DISABLE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(sinkListenerStub_->GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(static_cast<int32_t>(DHType::AUDIO));
    data.WriteString("id_test");
    EXPECT_EQ(DH_FWK_SUCCESS, sinkListenerStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: OnRemoteRequest_101
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_101, TestSize.Level0)
{
    ASSERT_TRUE(sourceListenerStub_ != nullptr);
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(ERR_INVALID_DATA, sourceListenerStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: OnRemoteRequest_102
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_102, TestSize.Level0)
{
    ASSERT_TRUE(sourceListenerStub_ != nullptr);
    uint32_t code = static_cast<uint32_t>(IHDSinkStatusListener::Message::ON_ENABLE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(sourceListenerStub_->GetDescriptor());
    data.WriteString("netWorkId_test");
    data.WriteInt32(1);
    data.WriteInt32(static_cast<int32_t>(DHType::AUDIO));
    data.WriteString("id_test");
    EXPECT_EQ(DH_FWK_SUCCESS, sourceListenerStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: OnRemoteRequest_103
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_103, TestSize.Level0)
{
    ASSERT_TRUE(sourceListenerStub_ != nullptr);
    uint32_t code = static_cast<uint32_t>(IHDSinkStatusListener::Message::ON_DISABLE);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(sourceListenerStub_->GetDescriptor());
    data.WriteString("netWorkId_test");
    data.WriteInt32(1);
    data.WriteInt32(static_cast<int32_t>(DHType::AUDIO));
    data.WriteString("id_test");
    EXPECT_EQ(DH_FWK_SUCCESS, sourceListenerStub_->OnRemoteRequest(code, data, reply, option));
}

HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_104, TestSize.Level0)
{
    ASSERT_TRUE(getDHCallbackStub_ != nullptr);
    std::vector<DHDescriptor> dhDescriptors;
    uint32_t code = static_cast<uint32_t>(IGetDhDescriptorsCallback::Message::ON_SUCCESS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(getDHCallbackStub_->GetDescriptor());
    data.WriteString("netWorkId_test");
    data.WriteUint32(MAX_DH_DESCRIPTOR_ARRAY_SIZE);
    auto ret = getDHCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_105, TestSize.Level0)
{
    ASSERT_TRUE(getDHCallbackStub_ != nullptr);
    uint32_t code = static_cast<uint32_t>(IGetDhDescriptorsCallback::Message::ON_SUCCESS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(getDHCallbackStub_->GetDescriptor());
    data.WriteString("netWorkId_test");
    data.WriteUint32(1);
    data.WriteUint32(static_cast<uint32_t>(DHType::CAMERA));
    data.WriteString("camera_1");
    auto ret = getDHCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_106, TestSize.Level0)
{
    ASSERT_TRUE(getDHCallbackStub_ != nullptr);
    uint32_t code = static_cast<uint32_t>(IGetDhDescriptorsCallback::Message::ON_ERROR);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(getDHCallbackStub_->GetDescriptor());
    data.WriteString("netWorkId_test");
    data.WriteInt32(1);
    auto ret = getDHCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(HardwareStatusListenerStubTest, OnRemoteRequest_107, TestSize.Level0)
{
    ASSERT_TRUE(getDHCallbackStub_ != nullptr);
    uint32_t code = INVALID_CODE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(getDHCallbackStub_->GetDescriptor());
    auto ret = getDHCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
