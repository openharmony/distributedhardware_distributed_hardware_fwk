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

#include <gtest/gtest.h>
#include <string>

#include "device_manager.h"
#include "device_manager_impl.h"

#include "capability_info.h"
#include "capability_info_manager.h"
#include "dh_transport_obj.h"
#include "dh_utils_tool.h"
#include "dh_comm_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "impl_utils.h"
#include "mock_other_method.h"

using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace DistributedHardware {
using namespace std;
constexpr uint16_t TEST_DEV_TYPE = 0x14;
// request remote dh send back full dh capabilities
constexpr int32_t DH_COMM_REQ_FULL_CAPS = 1;
// send back full dh attributes to the requester
constexpr int32_t DH_COMM_RSP_FULL_CAPS = 2;
constexpr int32_t INVALID_USER_ID = -100;
constexpr int32_t INVALID_ACCOUNT_INFO_VALUE = -101;


static std::string g_mocklocalNetworkId = "123456789";
static bool g_mockDMValue = false;

std::string GetLocalNetworkId()
{
    return g_mocklocalNetworkId;
}

bool DeviceManagerImpl::CheckSrcAccessControl(const DmAccessCaller &caller, const DmAccessCallee &callee)
{
    return g_mockDMValue;
}

class DhCommToolTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline shared_ptr<DeviceOtherMethodMock> otherMethodMock_ = nullptr;
private:
    std::shared_ptr<DHCommTool> dhCommToolTest_;
};

void DhCommToolTest::SetUpTestCase()
{
    otherMethodMock_ = make_shared<DeviceOtherMethodMock>();
    DeviceOtherMethodMock::otherMethod = otherMethodMock_;
}

void DhCommToolTest::TearDownTestCase()
{
    DeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
}

void DhCommToolTest::SetUp()
{
    dhCommToolTest_ = std::make_shared<DHCommTool>();
    dhCommToolTest_->Init();
    g_mockDMValue = false;
    g_mocklocalNetworkId = "123456789";
}

void DhCommToolTest::TearDown()
{
}

HWTEST_F(DhCommToolTest, TriggerReqFullDHCaps_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteNetworkId = "";
    dhCommToolTest_->dhTransportPtr_ = nullptr;
    dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId);

    remoteNetworkId = "123456789";
    dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId);
}

HWTEST_F(DhCommToolTest, TriggerReqFullDHCaps_002, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteNetworkId = "123456789";
    g_mocklocalNetworkId = "";
    ASSERT_NO_FATAL_FAILURE(dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId));
    dhCommToolTest_->UnInit();
}

HWTEST_F(DhCommToolTest, TriggerReqFullDHCaps_003, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteNetworkId = "123456789";
    std::vector<int32_t> userIds;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(INVALID_USER_ID)));
    ASSERT_NO_FATAL_FAILURE(dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId));
}

HWTEST_F(DhCommToolTest, TriggerReqFullDHCaps_004, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteNetworkId = "123456789";
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));

    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(DH_FWK_SUCCESS)));
    ASSERT_NO_FATAL_FAILURE(dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId));
}

HWTEST_F(DhCommToolTest, GetAndSendLocalFullCaps_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string reqNetworkId = "";
    bool isSyncMeta = false;
    dhCommToolTest_->dhTransportPtr_ = nullptr;
    ASSERT_NO_FATAL_FAILURE(dhCommToolTest_->GetAndSendLocalFullCaps(reqNetworkId, isSyncMeta));

    dhCommToolTest_->Init();
    isSyncMeta = true;
    ASSERT_NO_FATAL_FAILURE(dhCommToolTest_->GetAndSendLocalFullCaps(reqNetworkId, isSyncMeta));
}

HWTEST_F(DhCommToolTest, ParseAndSaveRemoteDHCaps_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteCaps = "";
    bool isSyncMeta = false;
    FullCapsRsp ret = dhCommToolTest_->ParseAndSaveRemoteDHCaps(remoteCaps, isSyncMeta);
    EXPECT_EQ("", ret.networkId);
}

HWTEST_F(DhCommToolTest, ParseAndSaveRemoteDHCaps_002, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteCaps = "remoteCaps_test";
    bool isSyncMeta = false;
    FullCapsRsp ret = dhCommToolTest_->ParseAndSaveRemoteDHCaps(remoteCaps, isSyncMeta);
    EXPECT_EQ("", ret.networkId);

    isSyncMeta = true;
    ret = dhCommToolTest_->ParseAndSaveRemoteDHCaps(remoteCaps, isSyncMeta);
    EXPECT_EQ("", ret.networkId);
}

HWTEST_F(DhCommToolTest, ParseAndSaveRemoteDHCaps_003, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    std::string networkId = "123456";
    bool isSyncMeta = false;
    cJSON_AddStringToObject(jsonObject, CAPS_RSP_NETWORKID_KEY, networkId.c_str());
    char* cjson = cJSON_PrintUnformatted(jsonObject);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObject);
        return;
    }
    std::string remoteCaps(cjson);
    FullCapsRsp ret = dhCommToolTest_->ParseAndSaveRemoteDHCaps(remoteCaps, isSyncMeta);
    EXPECT_EQ(networkId, ret.networkId);
    cJSON_Delete(jsonObject);
}

HWTEST_F(DhCommToolTest, ProcessEvent_001, TestSize.Level1)
{
    std::shared_ptr<DHCommTool> dhCommToolPtr = nullptr;
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    DHCommTool::DHCommToolEventHandler eventHandler(runner, dhCommToolPtr);
    AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(DH_COMM_REQ_FULL_CAPS, commMsg);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessEvent(event));
}

HWTEST_F(DhCommToolTest, ProcessEvent_002, TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    DHCommTool::DHCommToolEventHandler eventHandler(runner, dhCommToolTest_);
    AppExecFwk::InnerEvent::Pointer event1 = AppExecFwk::InnerEvent::Get(DH_COMM_REQ_FULL_CAPS, commMsg);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessEvent(event1));

    AppExecFwk::InnerEvent::Pointer event2 = AppExecFwk::InnerEvent::Get(DH_COMM_RSP_FULL_CAPS, commMsg);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessEvent(event2));

    AppExecFwk::InnerEvent::Pointer event3 = AppExecFwk::InnerEvent::Get(0, commMsg);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessEvent(event3));
}

HWTEST_F(DhCommToolTest, ProcessFullCapsRsp_001, TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    DHCommTool::DHCommToolEventHandler eventHandler(runner, dhCommToolTest_);

    std::vector<std::shared_ptr<CapabilityInfo>> caps;
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCaps;
    std::string networkId = "";
    bool isSyncMeta = false;
    FullCapsRsp capsRsp(networkId, caps, metaCaps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp, dhCommToolTest_, isSyncMeta));

    networkId = "networkId_test";
    FullCapsRsp capsRsp1(networkId, caps, metaCaps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp1, dhCommToolTest_, isSyncMeta));

    std::shared_ptr<DHCommTool> dhCommToolPtr = nullptr;
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp1, dhCommToolPtr, isSyncMeta));
}

HWTEST_F(DhCommToolTest, ProcessFullCapsRsp_002, TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    DHCommTool::DHCommToolEventHandler eventHandler(runner, dhCommToolTest_);

    bool isSyncMeta = false;
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCaps;
    std::vector<std::shared_ptr<CapabilityInfo>> caps;
    std::string networkId = "networkId_test";
    std::string dhId = "Camera_0";
    std::string devId = "123456789";
    std::string devName = "dev_pad";
    std::string dhAttrs = "attr";
    std::string subType = "camera";
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(dhId, devId, devName, TEST_DEV_TYPE,
        DHType::CAMERA, dhAttrs, subType);
    std::shared_ptr<CapabilityInfo> capInfo2 = nullptr;
    caps.push_back(capInfo1);
    caps.push_back(capInfo2);
    FullCapsRsp capsRsp1(networkId, caps, metaCaps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp1, dhCommToolTest_, isSyncMeta));

    isSyncMeta = true;
    CompVersion compVersion;
    compVersion.sinkVersion = "1.0";
    std::string udidHash = "udidHash_123";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo1 = std::make_shared<MetaCapabilityInfo>(
        dhId, devId, devName, TEST_DEV_TYPE, DHType::CAMERA, dhAttrs, subType, udidHash, compVersion);
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo2 = nullptr;
    metaCaps.push_back(metaCapInfo1);
    metaCaps.push_back(metaCapInfo2);
    FullCapsRsp capsRsp2(networkId, caps, metaCaps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp2, dhCommToolTest_, isSyncMeta));
}

HWTEST_F(DhCommToolTest, CheckCallerAclRight_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string localNetworkId;
    std::string remoteNetworkId;
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));

    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(DH_FWK_SUCCESS)));
    g_mockDMValue = false;
    auto ret = dhCommToolTest_->CheckCallerAclRight(localNetworkId, remoteNetworkId);
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhCommToolTest, CheckCallerAclRight_002, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string localNetworkId;
    std::string remoteNetworkId;
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));

    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(DH_FWK_SUCCESS)));
    g_mockDMValue = true;
    auto ret = dhCommToolTest_->CheckCallerAclRight(localNetworkId, remoteNetworkId);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhCommToolTest, GetOsAccountInfo_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(INVALID_USER_ID)));
    auto ret = dhCommToolTest_->GetOsAccountInfo();
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhCommToolTest, GetOsAccountInfo_002, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::vector<int32_t> userIds;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));
    auto ret = dhCommToolTest_->GetOsAccountInfo();
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhCommToolTest, GetOsAccountInfo_003, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));

    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(INVALID_ACCOUNT_INFO_VALUE)));
    auto ret = dhCommToolTest_->GetOsAccountInfo();
    EXPECT_EQ(false, ret);
}

HWTEST_F(DhCommToolTest, GetOsAccountInfo_004, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(DH_FWK_SUCCESS)));

    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(DH_FWK_SUCCESS)));
    auto ret = dhCommToolTest_->GetOsAccountInfo();
    EXPECT_EQ(true, ret);
}
}
}