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

#include "av_sync_manager_test.h"
#include "av_trans_errno.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
void AVSyncManagerTest::SetUpTestCase(void)
{
}

void AVSyncManagerTest::TearDownTestCase(void)
{
}

void AVSyncManagerTest::SetUp() {}

void AVSyncManagerTest::TearDown()
{
}

/**
 * @tc.name: add_stream_info_001
 * @tc.desc: add stream info function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, add_stream_info_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo stream;
    syncManager_->AddStreamInfo(stream);
    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: add_stream_info_002
 * @tc.desc: add stream info function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, add_stream_info_002, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    syncManager_->AddStreamInfo(streamFirst);
    AVStreamInfo streamSecond;
    syncManager_->AddStreamInfo(streamSecond);
    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: remove_stream_info_001
 * @tc.desc: remove stream info function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, remove_stream_info_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    AVStreamInfo streamSecond;
    streamSecond.sceneType = "streamSecondType";
    streamSecond.peerDevId = "streamSecondpeerDevId";
    syncManager_->AddStreamInfo(streamSecond);

    syncManager_->RemoveStreamInfo(streamSecond);
    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: enable_sender_av_sync_001
 * @tc.desc: enable sender av sync function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, enable_sender_av_sync_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);
    syncManager_->EnableSenderAVSync();

    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: disable_sender_av_sync_001
 * @tc.desc: disable sender av sync function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, disable_sender_av_sync_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    AVStreamInfo streamSecond;
    streamSecond.sceneType = "streamSecondType";
    streamSecond.peerDevId = "streamSecondpeerDevId";
    syncManager_->AddStreamInfo(streamSecond);
    syncManager_->DisableSenderAVSync();

    EXPECT_EQ(2, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: disable_sender_av_sync_002
 * @tc.desc: disable sender av sync function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, disable_sender_av_sync_002, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    syncManager_->DisableSenderAVSync();

    EXPECT_EQ(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: handle_av_sync_message_001
 * @tc.desc: handle av sync function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, handle_av_sync_message_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    message->type_ = (uint32_t)AVTransTag::START_AV_SYNC;
    syncManager_->HandleAvSyncMessage(message);
    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: handle_av_sync_message_002
 * @tc.desc: handle av sync function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, handle_av_sync_message_002, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    std::shared_ptr<AVTransMessage> message = std::make_shared<AVTransMessage>();
    message->type_ = (uint32_t)AVTransTag::STOP_AV_SYNC;
    syncManager_->HandleAvSyncMessage(message);
    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: enable_receiver_av_sync_001
 * @tc.desc: enable receiver av sync function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, enable_receiver_av_sync_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    std::string groupInfo = "groupInfo";
    syncManager_->EnableReceiverAVSync(groupInfo);
    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: disable_receiver_av_sync_001
 * @tc.desc: disable receiver av sync function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, disable_receiver_av_sync_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    std::string groupInfo = "groupInfo";
    syncManager_->DisableReceiverAVSync(groupInfo);
    EXPECT_NE(0, syncManager_->streamInfoList_.size());
}

/**
 * @tc.name: merge_group_info_001
 * @tc.desc: merge group info function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, merge_group_info_001, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = "streamFirstType";
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    std::string groupInfo;
    bool ret = syncManager_->MergeGroupInfo(groupInfo);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: merge_group_info_002
 * @tc.desc: merge group info function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, merge_group_info_002, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = SCENE_TYPE_D_MIC;
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    AVStreamInfo streamSecond;
    streamSecond.sceneType = SCENE_TYPE_D_SPEAKER;
    streamSecond.peerDevId = "streamSecondpeerDevId";
    syncManager_->AddStreamInfo(streamSecond);

    std::string groupInfo;
    bool ret = syncManager_->MergeGroupInfo(groupInfo);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: merge_group_info_003
 * @tc.desc: merge group info function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, merge_group_info_003, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = MIME_VIDEO_RAW;
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    AVStreamInfo streamSecond;
    streamSecond.sceneType = MIME_VIDEO_H264;
    streamSecond.peerDevId = "streamSecondpeerDevId";
    syncManager_->AddStreamInfo(streamSecond);

    std::string groupInfo;
    bool ret = syncManager_->MergeGroupInfo(groupInfo);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: merge_group_info_004
 * @tc.desc: merge group info function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(AVSyncManagerTest, merge_group_info_004, TestSize.Level0)
{
    syncManager_ = std::make_shared<AVSyncManager>();
    syncManager_->streamInfoList_.clear();
    AVStreamInfo streamFirst;
    streamFirst.sceneType = SCENE_TYPE_D_SCREEN;
    streamFirst.peerDevId = "streamFirstpeerDevId";
    syncManager_->AddStreamInfo(streamFirst);

    AVStreamInfo streamSecond;
    streamSecond.sceneType = SCENE_TYPE_D_SPEAKER;
    streamSecond.peerDevId = "streamSecondpeerDevId";
    syncManager_->AddStreamInfo(streamSecond);

    AVStreamInfo streamThird;
    streamThird.sceneType = SCENE_TYPE_D_CAMERA_STR;
    streamThird.peerDevId = "streamThirdpeerDevId";
    syncManager_->AddStreamInfo(streamThird);

    AVStreamInfo streamFourth;
    streamFourth.sceneType = SCENE_TYPE_D_MIC;
    streamFourth.peerDevId = "streamFourthpeerDevId";
    syncManager_->AddStreamInfo(streamFourth);

    std::string groupInfo;
    bool ret = syncManager_->MergeGroupInfo(groupInfo);
    EXPECT_EQ(false, ret);
}
} // namespace DistributedHardware
} // namespace OHOS