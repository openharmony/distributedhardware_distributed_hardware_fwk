/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "av_sender_engine_provider_test.h"

#include "av_sender_engine.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {

void AvSenderEngineProviderTest::SetUp()
{
}

void AvSenderEngineProviderTest::TearDown()
{
}

void AvSenderEngineProviderTest::SetUpTestCase()
{
}

void AvSenderEngineProviderTest::TearDownTestCase()
{
}

HWTEST_F(AvSenderEngineProviderTest, CreateAVSenderEngine_001, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "peerDevId";
    std::string ownerName = "ownerName";
    auto avSendProTest_ = std::make_shared<AVSenderEngineProvider>(ownerName);
    auto avSenderEngine = avSendProTest_->CreateAVSenderEngine(peerDevId);
    EXPECT_EQ(nullptr, avSenderEngine);
}

HWTEST_F(AvSenderEngineProviderTest, GetAVSenderEngineList_001, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "peerDevId";
    std::string ownerName = "ownerName";
    auto avSendProTest_ = std::make_shared<AVSenderEngineProvider>(ownerName);
    auto avSenderEngine = avSendProTest_->CreateAVSenderEngine(peerDevId);
    std::vector<std::shared_ptr<IAVSenderEngine>> senderEngineList = avSendProTest_->GetAVSenderEngineList();
    bool bRet = (senderEngineList.empty()) ? false : true;
    EXPECT_NE(true, bRet);
}

HWTEST_F(AvSenderEngineProviderTest, GetAVSenderEngineList_002, testing::ext::TestSize.Level1)
{
    std::string peerDevId = "peerDevId";
    std::string ownerName = "ownerName";
    auto avSendProTest_ = std::make_shared<AVSenderEngineProvider>(ownerName);
    auto avSenderEngine = avSendProTest_->CreateAVSenderEngine(peerDevId);
    auto sender = std::make_shared<AVSenderEngine>(ownerName, peerDevId);
    avSendProTest_->senderEngineList_.push_back(sender);
    std::vector<std::shared_ptr<IAVSenderEngine>> senderEngineList = avSendProTest_->GetAVSenderEngineList();
    bool bRet = (senderEngineList.empty()) ? false : true;
    EXPECT_EQ(true, bRet);
}

HWTEST_F(AvSenderEngineProviderTest, RegisterProviderCallback_001, testing::ext::TestSize.Level1)
{
    std::string ownerName = "ownerName";
    std::string peerDevId = "peerDevId";
    auto avSendProTest_ = std::make_shared<AVSenderEngineProvider>(ownerName);
    std::shared_ptr<IAVEngineProviderCallback> callback = nullptr;
    int32_t ret = avSendProTest_->RegisterProviderCallback(callback);
    AVTransEvent event = {EventType::EVENT_ADD_STREAM, ownerName, peerDevId};
    avSendProTest_->providerCallback_ = nullptr;
    avSendProTest_->OnChannelEvent(event);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS