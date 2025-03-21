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

#include "av_trans_audio_output_filter_test.h"
#include "av_trans_audio_encoder_filter.h"
#include "av_trans_constants.h"
using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string INPUT_BUFFER_QUEUE_NAME = "AVTransAudioInputBufferQueue";

void AvTransportAudioOutputFilterTest::SetUp()
{
    dAudioOutputTest_ = std::make_shared<Pipeline::DAudioOutputFilter>("builtin.daudio.output",
        Pipeline::FilterType::FILTERTYPE_SSINK);
}

void AvTransportAudioOutputFilterTest::TearDown()
{
}

void AvTransportAudioOutputFilterTest::SetUpTestCase()
{
}

void AvTransportAudioOutputFilterTest::TearDownTestCase()
{
}

HWTEST_F(AvTransportAudioOutputFilterTest, Init_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    dAudioOutputTest_->Init(nullptr, nullptr);
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoInitAfterLink_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoInitAfterLink();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, PrepareInputBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    dAudioOutputTest_->PrepareInputBuffer();
    EXPECT_NE(dAudioOutputTest_->outputBufQue_, nullptr);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoPrepare_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoPrepare();
    EXPECT_NE(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoStart_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoStart();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoPause_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoPause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoPauseDragging_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoPauseDragging();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoResume_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoResume();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoResumeDragging_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoResumeDragging();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoStop_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoStop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoFlush_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoFlush();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoRelease_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    Status ret = dAudioOutputTest_->DoRelease();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoProcessInputBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    dAudioOutputTest_->PrepareInputBuffer();
    Status ret = dAudioOutputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    
    dAudioOutputTest_->outputBufQue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
        Media::MemoryType::VIRTUAL_MEMORY, INPUT_BUFFER_QUEUE_NAME);
    ret = dAudioOutputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoProcessOutputBuffer_001, testing::ext::TestSize.Level1)
{
    Status ret = dAudioOutputTest_->DoProcessOutputBuffer(1, true, true, 2, 3);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, ProcessAndSendBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status ret = dAudioOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, ProcessAndSendBuffer_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData;
    Status ret = dAudioOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    audioData = std::make_shared<Media::AVBuffer>();
    dAudioOutputTest_->eventReceiver_ = std::make_shared<EventReceiverTest>();
    ret = dAudioOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, SetParameterAndGetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Media::Meta> meta = nullptr;
    std::shared_ptr<Media::Meta> meta2 = std::make_shared<Media::Meta>();
    dAudioOutputTest_->SetParameter(meta2);
    dAudioOutputTest_->GetParameter(meta);
    EXPECT_EQ(meta, meta2);
}

HWTEST_F(AvTransportAudioOutputFilterTest, LinkNext_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dAudioOutputTest_->LinkNext(avAudioEncoderTest_, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, UpdateNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter2",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dAudioOutputTest_->UpdateNext(filter, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, UnLinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter2",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dAudioOutputTest_->UnLinkNext(filter, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, OnLinked_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    Status ret = dAudioOutputTest_->OnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, meta, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, OnUpdated_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dAudioOutputTest_ != nullptr);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    Status ret = dAudioOutputTest_->OnUpdated(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, meta, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, OnUnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter2",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dAudioOutputTest_->OnUnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr);
    EXPECT_EQ(Status::OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS