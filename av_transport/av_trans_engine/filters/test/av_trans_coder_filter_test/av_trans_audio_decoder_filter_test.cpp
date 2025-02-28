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

#include "av_trans_audio_decoder_filter_test.h"

#include "av_trans_constants.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {

const std::string FILTERNAME = "audioDec";
static constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string INPUT_BUFFERQUEUE_NAME = "AvTransDecoderBufferQueue";

void AvTransportAudioDecoderFilterTest::SetUp()
{
}

void AvTransportAudioDecoderFilterTest::TearDown()
{
}

void AvTransportAudioDecoderFilterTest::SetUpTestCase()
{
}

void AvTransportAudioDecoderFilterTest::TearDownTestCase()
{
}

HWTEST_F(AvTransportAudioDecoderFilterTest, PrepareInputBufferQueue_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->PrepareInputBufferQueue();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, StartAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->StartAudioCodec();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    // Create Audio Codec
    ret = avAudioDecoderTest_->CreateAudioCodec();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, StopAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->StopAudioCodec();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, ReleaseAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->ReleaseAudioCodec();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, ConfigureAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Pipeline::ADecInitParams decInitParams;
    Status ret = avAudioDecoderTest_->ConfigureAudioCodec(decInitParams);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, CreateAudioCodec_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->CreateAudioCodec();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, CheckDecoderFormat_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Pipeline::ADecInitParams decInitParams;
    Status ret = avAudioDecoderTest_->CheckDecoderFormat(decInitParams);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, SetDecoderFormat_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Pipeline::ADecInitParams decInitParams;
    Status ret = avAudioDecoderTest_->SetDecoderFormat(decInitParams);
    EXPECT_EQ(Status::ERROR_INVALID_PARAMETER, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, ProcessData_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData;
    Status ret = avAudioDecoderTest_->ProcessData(audioData, 0, nullptr);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);

    // initial codeMem
    OH_AVBuffer *codecMem = nullptr;
    ret = avAudioDecoderTest_->ProcessData(audioData, 0, codecMem);
    delete codecMem;
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnDecInputBufferAvailable_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    avAudioDecoderTest_->OnDecInputBufferAvailable(1, nullptr);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnDecOutputBufferAvailable_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    avAudioDecoderTest_->OnDecOutputBufferAvailable(1, nullptr);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoPrepare_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->DoPrepare();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoStart_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->DoStart();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoStop_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->DoStop();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoFlush_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->DoFlush();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoRelease_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->DoRelease();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoProcessInputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);

    Status ret = avAudioDecoderTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);

    avAudioDecoderTest_->inputBufferQueue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
                                                                          Media::MemoryType::VIRTUAL_MEMORY,
                                                                          INPUT_BUFFERQUEUE_NAME);
    ret = avAudioDecoderTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, LinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->LinkNext(nullptr, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Status ret = avAudioDecoderTest_->OnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr, nullptr);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}
} // namespace DistributedHardware
} // namespace OHOS