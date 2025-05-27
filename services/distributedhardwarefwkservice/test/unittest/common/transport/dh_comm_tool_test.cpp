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

#include "dh_comm_tool.h"

#include <gtest/gtest.h>
#include <string>

#include "capability_info.h"
#include "capability_info_manager.h"
#include "dh_context.h"
#include "dh_transport_obj.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
using namespace std;
constexpr uint16_t TEST_DEV_TYPE = 0x14;
// request remote dh send back full dh capabilities
constexpr int32_t DH_COMM_REQ_FULL_CAPS = 1;
// send back full dh attributes to the requester
constexpr int32_t DH_COMM_RSP_FULL_CAPS = 2;

class DhCommToolTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<DHCommTool> dhCommToolTest_;
};

void DhCommToolTest::SetUpTestCase()
{
}

void DhCommToolTest::TearDownTestCase()
{
}

void DhCommToolTest::SetUp()
{
    dhCommToolTest_ = std::make_shared<DHCommTool>();
    dhCommToolTest_->Init();
}

void DhCommToolTest::TearDown()
{
}

HWTEST_F(DhCommToolTest, TriggerReqFullDHCaps_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteNetworkId = "";
    dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId);

    dhCommToolTest_->dhTransportPtr_ = nullptr;
    dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId);

    remoteNetworkId = "remoteNetworkId_test";
    dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId);

    dhCommToolTest_->Init();
    dhCommToolTest_->TriggerReqFullDHCaps(remoteNetworkId);
    dhCommToolTest_->UnInit();
    EXPECT_NE(nullptr, dhCommToolTest_->dhTransportPtr_);
}

HWTEST_F(DhCommToolTest, GetAndSendLocalFullCaps_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string reqNetworkId = "";
    dhCommToolTest_->dhTransportPtr_ = nullptr;
    dhCommToolTest_->GetAndSendLocalFullCaps(reqNetworkId);

    dhCommToolTest_->Init();
    dhCommToolTest_->GetAndSendLocalFullCaps(reqNetworkId);
    EXPECT_NE(nullptr, dhCommToolTest_->dhTransportPtr_);
}

HWTEST_F(DhCommToolTest, ParseAndSaveRemoteDHCaps_001, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    std::string remoteCaps = "";
    FullCapsRsp ret = dhCommToolTest_->ParseAndSaveRemoteDHCaps(remoteCaps);
    EXPECT_EQ("", ret.networkId);
}

HWTEST_F(DhCommToolTest, ParseAndSaveRemoteDHCaps_002, TestSize.Level1)
{
    ASSERT_TRUE(dhCommToolTest_ != nullptr);
    cJSON *jsonObject = cJSON_CreateObject();
    ASSERT_TRUE(jsonObject != nullptr);
    std::string networkId = "123456";
    cJSON_AddStringToObject(jsonObject, CAPS_RSP_NETWORKID_KEY, networkId.c_str());
    char* cjson = cJSON_PrintUnformatted(jsonObject);
    if (cjson == nullptr) {
        cJSON_Delete(jsonObject);
        return;
    }
    std::string remoteCaps(cjson);
    FullCapsRsp ret = dhCommToolTest_->ParseAndSaveRemoteDHCaps(remoteCaps);
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
    std::string networkId = "";
    FullCapsRsp capsRsp(networkId, caps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp, dhCommToolTest_));

    networkId = "networkId_test";
    FullCapsRsp capsRsp1(networkId, caps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp1, dhCommToolTest_));
}

HWTEST_F(DhCommToolTest, ProcessFullCapsRsp_002, TestSize.Level1)
{
    std::shared_ptr<CommMsg> commMsg = std::make_shared<CommMsg>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    DHCommTool::DHCommToolEventHandler eventHandler(runner, dhCommToolTest_);

    std::vector<std::shared_ptr<CapabilityInfo>> caps;
    std::string networkId = "networkId_test";
    std::string dhId = "Camera_0";
    std::string devId = "123456789";
    std::string devName = "dev_pad";
    std::string dhAttrs = "attr";
    std::string subType = "camera";
    std::shared_ptr<CapabilityInfo> capInfoTest = make_shared<CapabilityInfo>(dhId, devId, devName, TEST_DEV_TYPE,
        DHType::CAMERA, dhAttrs, subType);
    caps.push_back(capInfoTest);
    std::shared_ptr<DHCommTool> dhCommToolPtr = nullptr;
    FullCapsRsp capsRsp1(networkId, caps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp1, dhCommToolPtr));

    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp1, dhCommToolTest_));

    networkId = "333333";
    DHContext::GetInstance().AddOnlineDevice("111111", "222222", "333333");
    FullCapsRsp capsRsp2(networkId, caps);
    ASSERT_NO_FATAL_FAILURE(eventHandler.ProcessFullCapsRsp(capsRsp2, dhCommToolTest_));
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(networkId);
}
}
}