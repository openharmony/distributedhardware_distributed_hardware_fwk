/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "av_trans_control_center_callback_test.h"
#include "av_trans_errno.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void AVTransControlCenterCallbackTest::SetUpTestCase(void)
{
}

void AVTransControlCenterCallbackTest::TearDownTestCase(void)
{
}

void AVTransControlCenterCallbackTest::SetUp() {}

void AVTransControlCenterCallbackTest::TearDown()
{
}

/**
 * @tc.name: set_parameter_001
 * @tc.desc: set parameter function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackTest, set_parameter_001, TestSize.Level0)
{
    callBack_ = std::make_shared<AVTransControlCenterCallback>();
    callBack_->receiverEngine_ = std::shared_ptr<ReceiverEngineTest>();
    callBack_->senderEngine_ = std::shared_ptr<SenderEngineTest>();
    AVTransTag tag = AVTransTag::START_AV_SYNC;
    std::string value;
    int32_t ret = callBack_->SetParameter(tag, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: set_parameter_002
 * @tc.desc: set parameter function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackTest, set_parameter_002, TestSize.Level0)
{
    callBack_ = std::make_shared<AVTransControlCenterCallback>();
    callBack_->receiverEngine_ = std::shared_ptr<ReceiverEngineTest>();
    callBack_->senderEngine_ = std::shared_ptr<SenderEngineTest>();
    AVTransTag tag = AVTransTag::STOP_AV_SYNC;
    std::string value;
    int32_t ret = callBack_->SetParameter(tag, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: set_parameter_003
 * @tc.desc: set parameter function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackTest, set_parameter_003, TestSize.Level0)
{
    callBack_ = std::make_shared<AVTransControlCenterCallback>();
    callBack_->receiverEngine_ = std::shared_ptr<ReceiverEngineTest>();
    callBack_->senderEngine_ = std::shared_ptr<SenderEngineTest>();
    AVTransTag tag = AVTransTag::TIME_SYNC_RESULT;
    std::string value;
    int32_t ret = callBack_->SetParameter(tag, value);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    callBack_->receiverEngine_ = std::shared_ptr<ReceiverEngineTest>();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: set_shared_memory_001
 * @tc.desc: set shared memory function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackTest, set_shared_memory_001, TestSize.Level0)
{
    callBack_ = std::make_shared<AVTransControlCenterCallback>();
    callBack_->receiverEngine_ = std::shared_ptr<ReceiverEngineTest>();
    callBack_->senderEngine_ = std::shared_ptr<SenderEngineTest>();
    AVTransSharedMemory memory;
    int32_t ret = callBack_->SetSharedMemory(memory);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
    callBack_->senderEngine_ = std::shared_ptr<SenderEngineTest>();
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}

/**
 * @tc.name: notify_001
 * @tc.desc: notify function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVTransControlCenterCallbackTest, notify_001, TestSize.Level0)
{
    callBack_ = std::make_shared<AVTransControlCenterCallback>();
    callBack_->receiverEngine_ = std::shared_ptr<ReceiverEngineTest>();
    callBack_->senderEngine_ = std::shared_ptr<SenderEngineTest>();
    AVTransEvent event;
    event.type = EventType::EVENT_ADD_STREAM;
    int32_t ret = callBack_->Notify(event);
    std::shared_ptr<IAVSenderEngine> sender = std::shared_ptr<SenderEngineTest>();
    callBack_->SetSenderEngine(sender);
    sender = nullptr;
    callBack_->SetSenderEngine(sender);
    std::shared_ptr<IAVReceiverEngine> receiver = std::shared_ptr<ReceiverEngineTest>();
    callBack_->SetReceiverEngine(receiver);
    receiver = nullptr;
    callBack_->SetReceiverEngine(receiver);
    EXPECT_EQ(DH_AVT_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS