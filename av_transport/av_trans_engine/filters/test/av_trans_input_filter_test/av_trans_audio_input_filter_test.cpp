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

#include "av_trans_audio_input_filter_test.h"
#include "av_trans_audio_encoder_filter.h"
#include "av_trans_constants.h"
using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string INPUT_BUFFER_QUEUE_NAME = "AVTransAudioInputBufferQueue";

void AvTransportAudioInputFilterTest::SetUp()
{
}

void AvTransportAudioInputFilterTest::TearDown()
{
}

void AvTransportAudioInputFilterTest::SetUpTestCase()
{
}

void AvTransportAudioInputFilterTest::TearDownTestCase()
{
}

HWTEST_F(AvTransportAudioInputFilterTest, PrepareInputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    avAudioInputTest_->PrepareInputBuffer();
}

HWTEST_F(AvTransportAudioInputFilterTest, ProcessAndSendBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status ret = avAudioInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, DoPrepare_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    Status ret = avAudioInputTest_->DoPrepare();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, DoStart_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    Status ret = avAudioInputTest_->DoStart();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, DoStop_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    Status ret = avAudioInputTest_->DoStop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, DoFlush_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    Status ret = avAudioInputTest_->DoFlush();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, DoRelease_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    Status ret = avAudioInputTest_->DoRelease();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, DoProcessInputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    Status ret = avAudioInputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
    
    avAudioInputTest_->inputBufQue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
                                                                          Media::MemoryType::VIRTUAL_MEMORY,
                                                                          INPUT_BUFFER_QUEUE_NAME);
    ret = avAudioInputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, LinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = avAudioInputTest_->LinkNext(avAudioEncoderTest_, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioInputFilterTest, OnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avAudioInputTest_ =
        std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avAudioInputTest_ != nullptr);
    Status ret = avAudioInputTest_->OnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr, nullptr);
    EXPECT_EQ(Status::OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS