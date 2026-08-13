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
#include "av_trans_audio_encoder_filter.h"
#include "av_trans_log.h"
#include "av_sync_utils.h"
#include "native_avmagic.h"
#include "av_trans_constants.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {

class AudioFilterDecCallback : public Pipeline::FilterLinkCallback {
public:
    explicit AudioFilterDecCallback(std::shared_ptr<Pipeline::AudioDecoderFilter> filter)
        : inputFilter_(std::move(filter)) {}
    ~AudioFilterDecCallback() = default;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer> &queue,
                        std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = inputFilter_.lock()) {
            filter->OnLinkedResult(queue, meta);
        } else {
            AVTRANS_LOGE("Invalid inputfilter");
        }
    }
    void OnUnlinkedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = inputFilter_.lock()) {
            filter->OnUnLinkedResult(meta);
        } else {
            AVTRANS_LOGE("Invalid inputfilter");
        }
    }
    void OnUpdatedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = inputFilter_.lock()) {
            filter->OnUpdatedResult(meta);
        } else {
            AVTRANS_LOGE("Invalid inputfilter");
        }
    }

private:
    std::weak_ptr<Pipeline::AudioDecoderFilter> inputFilter_{};
};

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

HWTEST_F(AvTransportAudioDecoderFilterTest, StartAudioCodec_002, testing::ext::TestSize.Level1)
{
    DAudioAccessConfigManager::GetInstance().ClearAccessConfig();
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
            Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);

    // Create Audio Codec
    avAudioDecoderTest_->initDecParams_.codecType = Pipeline::AudioCodecType::AUDIO_CODEC_AAC;
    avAudioDecoderTest_->CreateAudioCodec();
    std::string networkId = "0";
    DAudioAccessConfigManager::GetInstance().currentNetworkId_ = networkId;
    int32_t timeOutMs = 3000;
    Status ret = avAudioDecoderTest_->StartAudioCodec();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeOutMs));
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    ret = avAudioDecoderTest_->StartAudioCodec();
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, false);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    ret = avAudioDecoderTest_->StartAudioCodec();
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    DAudioAccessConfigManager::GetInstance().currentNetworkId_ = "";
    ret = avAudioDecoderTest_->StartAudioCodec();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
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

HWTEST_F(AvTransportAudioDecoderFilterTest, DoInitAfterLink_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Status status = avAudioDecoderTest_->DoInitAfterLink();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoPause_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Status status = avAudioDecoderTest_->DoPause();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoPauseDragging_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Status status = avAudioDecoderTest_->DoPauseDragging();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoResume_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Status status = avAudioDecoderTest_->DoResume();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoResumeDragging_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Status status = avAudioDecoderTest_->DoResumeDragging();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, DoProcessOutputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    EXPECT_EQ(avAudioDecoderTest_->DoProcessOutputBuffer(0, false, false, 0, 0), Status::OK);
    EXPECT_EQ(avAudioDecoderTest_->DoProcessOutputBuffer(-1, false, false, 0, 0), Status::OK);
    EXPECT_EQ(avAudioDecoderTest_->DoProcessOutputBuffer(0, false, false,
                                                         std::numeric_limits<uint32_t>::max(), 0),
              Status::OK);
    EXPECT_EQ(avAudioDecoderTest_->DoProcessOutputBuffer(0, false, false,
                                                         0, std::numeric_limits<int64_t>::max()),
              Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    avAudioDecoderTest_->SetParameter(meta);
    EXPECT_EQ(avAudioDecoderTest_->meta_, meta);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, LinkNext_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    std::shared_ptr<Pipeline::AudioEncoderFilter> nextFilter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = avAudioDecoderTest_->LinkNext(nextFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(ret, Status::ERROR_NULL_POINTER);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, UpdateNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    std::shared_ptr<Pipeline::AudioEncoderFilter> nextFilter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioDecoderTest_->UpdateNext(nextFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, UnLinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Status status = avAudioDecoderTest_->UnLinkNext(nullptr, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnLinked_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Pipeline::StreamType inType = Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO;
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<AudioFilterDecCallback> callback = std::make_shared<AudioFilterDecCallback>(avAudioDecoderTest_);
    Status status = avAudioDecoderTest_->OnLinked(inType, meta, callback);
    EXPECT_EQ(status, Status::OK);
    status = avAudioDecoderTest_->OnLinked(inType, meta, nullptr);
    EXPECT_EQ(status, Status::ERROR_NULL_POINTER);
    status = avAudioDecoderTest_->OnLinked(inType, nullptr, callback);
    EXPECT_EQ(status, Status::ERROR_NULL_POINTER);
    avAudioDecoderTest_->initDecParams_.codecType = Pipeline::AudioCodecType::AUDIO_CODEC_OPUS;
    status = avAudioDecoderTest_->OnLinked(inType, meta, callback);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnUpdated_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<AudioFilterDecCallback> callback = std::make_shared<AudioFilterDecCallback>(avAudioDecoderTest_);
    Status status = avAudioDecoderTest_->OnUpdated(Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO, meta, callback);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnUnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    auto status = avAudioDecoderTest_->OnUnLinked(Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO, nullptr);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnLinkedResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    sptr<Media::AVBufferQueueProducer> queue = nullptr;
    std::shared_ptr<Media::Meta> meta = nullptr;
    avAudioDecoderTest_->OnLinkedResult(queue, meta);
    EXPECT_EQ(avAudioDecoderTest_->outputProducer_, nullptr);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnUnLinkedResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    avAudioDecoderTest_->OnUnLinkedResult(meta);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnUpdatedResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    avAudioDecoderTest_->OnUpdatedResult(meta);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, ConfigureAudioCodec_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Pipeline::ADecInitParams initParams;
    Status status = avAudioDecoderTest_->ConfigureAudioCodec(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
    initParams = {};
    status = avAudioDecoderTest_->ConfigureAudioCodec(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
    avAudioDecoderTest_->audioDecoder_ = nullptr;
    status = avAudioDecoderTest_->ConfigureAudioCodec(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, SetDecoderFormat_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    Pipeline::ADecInitParams decInitParams;
    decInitParams.channel = 2;
    decInitParams.sampleRate = 44100;
    decInitParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    Status status = avAudioDecoderTest_->SetDecoderFormat(decInitParams);
    EXPECT_EQ(status, Status::ERROR_NULL_POINTER);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, CheckDecoderFormat_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    Pipeline::ADecInitParams initParams;
    initParams.channel = 2;
    initParams.sampleRate = 44100;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    Status status = avAudioDecoderTest_->CheckDecoderFormat(initParams);
    EXPECT_EQ(status, Status::OK);

    initParams.channel = 0;
    initParams.sampleRate = 44100;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    status = avAudioDecoderTest_->CheckDecoderFormat(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);

    initParams.channel = 2;
    initParams.sampleRate = 1000;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    status = avAudioDecoderTest_->CheckDecoderFormat(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);

    initParams.channel = 2;
    initParams.sampleRate = 1000;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::INVALID_WIDTH;
    status = avAudioDecoderTest_->CheckDecoderFormat(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, InputDecodeAudioData_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> filter =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    filter->isDecoderRunning_.store(true);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    filter->inputDataBufferQueue_.push(audioData);
    OH_AVBuffer *codecMem = new OH_AVBuffer(audioData);
    filter->codecBufQueue_.push(codecMem);
    filter->codecIndexQueue_.push(1);
    filter->InputDecodeAudioData();
    EXPECT_TRUE(filter->inputConsumer_ == nullptr);
    delete codecMem;
}

HWTEST_F(AvTransportAudioDecoderFilterTest, ProcessData_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> avAudioDecoderTest_ =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(avAudioDecoderTest_ != nullptr);
    avAudioDecoderTest_->isDecoderRunning_.store(true);
    OH_AVCodec audioDecoder(AVMagic::AVCODEC_MAGIC_AUDIO_DECODER);
    avAudioDecoderTest_->audioDecoder_ = &audioDecoder;
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status status = avAudioDecoderTest_->ProcessData(audioData, 0, nullptr);
    avAudioDecoderTest_->isDecoderRunning_.store(false);
    avAudioDecoderTest_->audioDecoder_ = nullptr;
    EXPECT_EQ(Status::ERROR_NULL_POINTER, status);
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnDecError_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> filter =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    int32_t normalErrorCode = 100;
    filter->OnDecError(normalErrorCode);
    EXPECT_FALSE(filter->isDecoderRunning_.load());
    int32_t boundaryErrorCode = -1;
    filter->OnDecError(boundaryErrorCode);
    EXPECT_FALSE(filter->isDecoderRunning_.load());
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnDecOutputFormatChanged, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> filter =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(filter != nullptr);
    OH_AVFormat *nullFormat = nullptr;
    filter->OnDecOutputFormatChanged(nullFormat);
    OH_AVFormat *validFormat = new (std::nothrow) OH_AVFormat();
    filter->OnDecOutputFormatChanged(validFormat);
    delete validFormat;
}

HWTEST_F(AvTransportAudioDecoderFilterTest, OnDecInputBufferAvailable_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioDecoderFilter> filter =
        std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_ADEC);
    ASSERT_TRUE(filter != nullptr);
    OH_AVBuffer *nullBuffer = nullptr;
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    filter->OnDecInputBufferAvailable(0, nullBuffer);
    for (int i = 0; i < 10; i++)
    {
        OH_AVBuffer *buffer = new OH_AVBuffer(audioData);
        filter->OnDecInputBufferAvailable(i, buffer);
        delete buffer;
    }
    OH_AVBuffer *normalBuffer = new OH_AVBuffer(audioData);
    filter->OnDecInputBufferAvailable(10, normalBuffer);
    delete normalBuffer;
}
} // namespace DistributedHardware
} // namespace OHOS