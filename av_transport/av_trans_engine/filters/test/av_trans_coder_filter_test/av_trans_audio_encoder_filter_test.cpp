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

#include "av_trans_audio_encoder_filter_test.h"

#include "av_trans_constants.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {

const std::string FILTERNAME = "audioEnc";
static constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string INPUT_BUFFERQUEUE_NAME = "AvTransEncoderBufferQueue";

void AvTransportAudioEncoderFilterTest::SetUp()
{
}

void AvTransportAudioEncoderFilterTest::TearDown()
{
}

void AvTransportAudioEncoderFilterTest::SetUpTestCase()
{
}

void AvTransportAudioEncoderFilterTest::TearDownTestCase()
{
}

HWTEST_F(AvTransportAudioEncoderFilterTest, PrepareInputBufferQueue_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->PrepareInputBufferQueue();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, StartAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->StartAudioCodec();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    // Create Audio Codec
    ret = avAudioEncoderTest_->CreateAudioCodec();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, StopAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->StopAudioCodec();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, ReleaseAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->ReleaseAudioCodec();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, ConfigureAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Pipeline::AEncInitParams encInitParams;
    Status ret = avAudioEncoderTest_->ConfigureAudioCodec(encInitParams);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, CreateAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->CreateAudioCodec();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, CheckDecoderFormat_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Pipeline::AEncInitParams encInitParams;
    Status ret = avAudioEncoderTest_->CheckEncoderFormat(encInitParams);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, SetDecoderFormat_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Pipeline::AEncInitParams encInitParams;
    Status ret = avAudioEncoderTest_->SetEncoderFormat(encInitParams);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, ProcessData_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData;
    Status ret = avAudioEncoderTest_->ProcessData(audioData, 0, nullptr);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    // initial codeMem
    OH_AVBuffer *codecMem = nullptr;
    ret = avAudioEncoderTest_->ProcessData(audioData, 0, codecMem);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    delete codecMem;
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnEncInputBufferAvailable_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    avAudioEncoderTest_->OnEncInputBufferAvailable(1, nullptr);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnEncOutputBufferAvailable_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    avAudioEncoderTest_->OnEncOutputBufferAvailable(1, nullptr);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoPrepare_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->DoPrepare();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoStart_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->DoStart();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoStop_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->DoStop();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoFlush_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->DoFlush();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoRelease_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->DoRelease();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoProcessInputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);

    Status ret = avAudioEncoderTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    avAudioEncoderTest_->inputBufferQueue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
                                                                          Media::MemoryType::VIRTUAL_MEMORY,
                                                                          INPUT_BUFFERQUEUE_NAME);
    ret = avAudioEncoderTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, LinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->LinkNext(nullptr, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Status ret = avAudioEncoderTest_->OnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr, nullptr);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}
} // namespace DistributedHardware
} // namespace OHOS