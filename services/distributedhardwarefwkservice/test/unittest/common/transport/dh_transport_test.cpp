/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dh_transport.h"
#include <string>

#include "dh_comm_tool.h"
#include "distributed_hardware_errno.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
using namespace std;

namespace {
    int32_t g_socketid = 1;
    std::string g_networkid = "networkId_test";
}
class DhTransportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<DHTransport> dhTransportTest_;
};

void DhTransportTest::SetUpTestCase()
{
}

void DhTransportTest::TearDownTestCase()
{
}

void DhTransportTest::SetUp()
{
    std::shared_ptr<DHCommTool> dhCommTool = std::make_shared<DHCommTool>();
    dhTransportTest_ = std::make_shared<DHTransport>(dhCommTool);
}

void DhTransportTest::TearDown()
{
}

HWTEST_F(DhTransportTest, OnSocketClosed_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    ShutdownReason reason = ShutdownReason::SHUTDOWN_REASON_UNKNOWN;
    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    dhTransportTest_->OnSocketClosed(2, reason);

    dhTransportTest_->OnSocketClosed(g_socketid, reason);
    EXPECT_EQ(0, dhTransportTest_->remoteDevSocketIds_.size());
}

HWTEST_F(DhTransportTest, OnBytesReceived_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    int32_t sessionId = -1;
    char *data = nullptr;
    uint32_t dataLen = 0;
    dhTransportTest_->OnBytesReceived(sessionId, data, dataLen);

    dhTransportTest_->OnBytesReceived(g_socketid, data, dataLen);

    char dataMsg[10] = "dataMsg";
    dhTransportTest_->OnBytesReceived(sessionId, dataMsg, dataLen);

    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);

    dataLen = 1;
    dhTransportTest_->OnBytesReceived(g_socketid, data, dataLen);

    dhTransportTest_->OnBytesReceived(sessionId, data, dataLen);
    dhTransportTest_->OnBytesReceived(sessionId, dataMsg, dataLen);
    dataLen = 5 * 1024 * 1024;
    dhTransportTest_->OnBytesReceived(sessionId, data, dataLen);
    dhTransportTest_->OnBytesReceived(sessionId, dataMsg, dataLen);
    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);

    dataLen = 1;
    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);
    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    dhTransportTest_->OnBytesReceived(g_socketid, dataMsg, dataLen);
    EXPECT_EQ(1, dhTransportTest_->remoteDevSocketIds_.size());
}

HWTEST_F(DhTransportTest, Init_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    dhTransportTest_->isSocketSvrCreateFlag_ = true;
    auto ret = dhTransportTest_->Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    dhTransportTest_->isSocketSvrCreateFlag_ = false;
    dhTransportTest_->Init();
    EXPECT_NE(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);
}

HWTEST_F(DhTransportTest, UnInit_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    auto ret = dhTransportTest_->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    dhTransportTest_->isSocketSvrCreateFlag_ = true;
    dhTransportTest_->UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DhTransportTest, IsDeviceSessionOpened_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    dhTransportTest_->remoteDevSocketIds_.clear();
    auto ret = dhTransportTest_->IsDeviceSessionOpened(g_networkid, g_socketid);
    EXPECT_EQ(false, ret);

    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    ret = dhTransportTest_->IsDeviceSessionOpened(g_networkid, g_socketid);
    EXPECT_EQ(true, ret);
}

HWTEST_F(DhTransportTest, StartSocket_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    auto ret = dhTransportTest_->StartSocket(g_networkid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    std::string remoteNetworkId = "remoteNetworkId_test";
    ret = dhTransportTest_->StartSocket(remoteNetworkId);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);
}

HWTEST_F(DhTransportTest, StopSocket_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    dhTransportTest_->remoteDevSocketIds_.clear();
    auto ret = dhTransportTest_->StopSocket(g_networkid);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);

    dhTransportTest_->remoteDevSocketIds_[g_networkid] = g_socketid;
    ret = dhTransportTest_->StopSocket(g_networkid);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DhTransportTest, Send_001, TestSize.Level0)
{
    if (dhTransportTest_ == nullptr) {
        return;
    }
    std::string payload = "payload_test";
    dhTransportTest_->remoteDevSocketIds_.clear();
    auto ret = dhTransportTest_->Send(g_networkid, payload);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED, ret);
}
}
}