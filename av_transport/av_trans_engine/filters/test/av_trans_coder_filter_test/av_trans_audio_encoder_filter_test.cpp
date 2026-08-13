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
#include "av_trans_audio_decoder_filter.h"
#include "av_trans_log.h"
#include "av_sync_utils.h"
#include "native_avmagic.h"
#include "av_trans_constants.h"
#include <chrono>
#include <thread>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {

const std::string FILTERNAME = "audioEnc";
static constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string INPUT_BUFFERQUEUE_NAME = "AvTransEncoderBufferQueue";

class AudioFilterEncCallback : public Pipeline::FilterLinkCallback {
public:
    explicit AudioFilterEncCallback(std::shared_ptr<Pipeline::AudioEncoderFilter> filter)
        : inputFilter_(std::move(filter)) {}
    ~AudioFilterEncCallback() = default;

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
    std::weak_ptr<Pipeline::AudioEncoderFilter> inputFilter_{};
};

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
    avAudioEncoderTest_->initEncParams_.codecType = Pipeline::AudioCodecType::AUDIO_CODEC_AAC;
    ret = avAudioEncoderTest_->CreateAudioCodec();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, StartAudioCodec_002, testing::ext::TestSize.Level1)
{
    DAudioAccessConfigManager::GetInstance().ClearAccessConfig();
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);

    // Create Audio Codec
    avAudioEncoderTest_->initEncParams_.codecType = Pipeline::AudioCodecType::AUDIO_CODEC_AAC;
    avAudioEncoderTest_->CreateAudioCodec();
    std::string networkId = "0";
    DAudioAccessConfigManager::GetInstance().currentNetworkId_ = networkId;
    int32_t timeOutMs = 3000;
    Status ret = avAudioEncoderTest_->StartAudioCodec();
    std::this_thread::sleep_for(std::chrono::milliseconds(timeOutMs));
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    ret = avAudioEncoderTest_->StartAudioCodec();
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, false);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    ret = avAudioEncoderTest_->StartAudioCodec();
    DAudioAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    DAudioAccessConfigManager::GetInstance().currentNetworkId_ = "";
    ret = avAudioEncoderTest_->StartAudioCodec();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
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
    avAudioEncoderTest_->initEncParams_.codecType = Pipeline::AudioCodecType::AUDIO_CODEC_AAC;
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
    delete codecMem;
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnEncInputBufferAvailable_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    avAudioEncoderTest_->OnEncInputBufferAvailable(1, nullptr);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnEncOutputBufferAvailable_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    avAudioEncoderTest_->OnEncOutputBufferAvailable(1, nullptr);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
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

HWTEST_F(AvTransportAudioEncoderFilterTest, DoInitAfterLink_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioEncoderTest_->DoInitAfterLink();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoPause_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioEncoderTest_->DoPause();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoPauseDragging_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioEncoderTest_->DoPauseDragging();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoResume_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioEncoderTest_->DoResume();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoResumeDragging_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioEncoderTest_->DoResumeDragging();
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, DoProcessOutputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    EXPECT_EQ(avAudioEncoderTest_->DoProcessOutputBuffer(0, false, false, 0, 0), Status::OK);
    EXPECT_EQ(avAudioEncoderTest_->DoProcessOutputBuffer(-1, false, false, 0, 0), Status::OK);
    EXPECT_EQ(avAudioEncoderTest_->DoProcessOutputBuffer(0, false, false,
                                                         std::numeric_limits<uint32_t>::max(), 0),
              Status::OK);
    EXPECT_EQ(avAudioEncoderTest_->DoProcessOutputBuffer(0, false, false,
                                                         0, std::numeric_limits<int64_t>::max()),
              Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, SetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    avAudioEncoderTest_->SetParameter(meta);
    EXPECT_EQ(avAudioEncoderTest_->meta_, meta);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, LinkNext_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    std::shared_ptr<Pipeline::AudioEncoderFilter> nextFilter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = avAudioEncoderTest_->LinkNext(nextFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(ret, Status::ERROR_NULL_POINTER);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, UpdateNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    std::shared_ptr<Pipeline::AudioEncoderFilter> nextFilter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioEncoderTest_->UpdateNext(nextFilter, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, UnLinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Status status = avAudioEncoderTest_->UnLinkNext(nullptr, Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnLinked_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Pipeline::StreamType inType = Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO;
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<AudioFilterEncCallback> callback = std::make_shared<AudioFilterEncCallback>(avAudioEncoderTest_);
    Status status = avAudioEncoderTest_->OnLinked(inType, meta, callback);
    EXPECT_EQ(status, Status::OK);
    status = avAudioEncoderTest_->OnLinked(inType, meta, nullptr);
    EXPECT_EQ(status, Status::ERROR_NULL_POINTER);
    status = avAudioEncoderTest_->OnLinked(inType, nullptr, callback);
    EXPECT_EQ(status, Status::ERROR_NULL_POINTER);
    meta->SetData(Media::Tag::MIME_TYPE, Pipeline::AudioCodecType::AUDIO_CODEC_OPUS);
    status = avAudioEncoderTest_->OnLinked(inType, meta, callback);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnUpdated_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<AudioFilterEncCallback> callback = std::make_shared<AudioFilterEncCallback>(avAudioEncoderTest_);
    Status status = avAudioEncoderTest_->OnUpdated(Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO, meta, callback);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnUnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    auto status = avAudioEncoderTest_->OnUnLinked(Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO, nullptr);
    EXPECT_EQ(status, Status::OK);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnLinkedResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    sptr<Media::AVBufferQueueProducer> queue = nullptr;
    std::shared_ptr<Media::Meta> meta = nullptr;
    avAudioEncoderTest_->OnLinkedResult(queue, meta);
    EXPECT_EQ(avAudioEncoderTest_->outputProducer_, nullptr);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnUnLinkedResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    avAudioEncoderTest_->OnUnLinkedResult(meta);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnUpdatedResult_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    avAudioEncoderTest_->OnUpdatedResult(meta);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, ConfigureAudioCodec_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Pipeline::AEncInitParams initParams;
    Status status = avAudioEncoderTest_->ConfigureAudioCodec(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
    initParams = {};
    status = avAudioEncoderTest_->ConfigureAudioCodec(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
    avAudioEncoderTest_->audioEncoder_ = nullptr;
    status = avAudioEncoderTest_->ConfigureAudioCodec(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, SetEncoderFormat_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Pipeline::AEncInitParams encInitParams;
    encInitParams.channel = 2;
    encInitParams.sampleRate = 44100;
    encInitParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    Status status = avAudioEncoderTest_->SetEncoderFormat(encInitParams);
    EXPECT_EQ(status, Status::ERROR_NULL_POINTER);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, SetEncoderFormat_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    Pipeline::AEncInitParams encInitParams;
    encInitParams.channel = 2;
    encInitParams.sampleRate = 44100;
    encInitParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    encInitParams.codecType = Pipeline::AudioCodecType::AUDIO_CODEC_OPUS;
    OH_AVCodec audioEncoder(AVMagic::AVCODEC_MAGIC_AUDIO_ENCODER);
    avAudioEncoderTest_->audioEncoder_ = &audioEncoder;
    Status status = avAudioEncoderTest_->SetEncoderFormat(encInitParams);
    avAudioEncoderTest_->audioEncoder_ = nullptr;
    EXPECT_EQ(status, Status::ERROR_INVALID_OPERATION);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, CheckEncoderFormat_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    Pipeline::AEncInitParams initParams;
    initParams.channel = 2;
    initParams.sampleRate = 44100;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    Status status = avAudioEncoderTest_->CheckEncoderFormat(initParams);
    EXPECT_EQ(status, Status::OK);

    initParams.channel = 0;
    initParams.sampleRate = 44100;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    status = avAudioEncoderTest_->CheckEncoderFormat(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);

    initParams.channel = 2;
    initParams.sampleRate = 1000;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::SAMPLE_U8;
    status = avAudioEncoderTest_->CheckEncoderFormat(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);

    initParams.channel = 2;
    initParams.sampleRate = 1000;
    initParams.sampleDepth = MediaAVCodec::AudioSampleFormat::INVALID_WIDTH;
    status = avAudioEncoderTest_->CheckEncoderFormat(initParams);
    EXPECT_EQ(status, Status::ERROR_INVALID_PARAMETER);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, InputEncodeAudioData_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    filter->isEncoderRunning_.store(true);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    filter->inputDataBufferQueue_.push(audioData);
    OH_AVBuffer *codecMem = new OH_AVBuffer(audioData);
    filter->codecBufQueue_.push(codecMem);
    filter->codecIndexQueue_.push(1);
    filter->InputEncodeAudioData();
    EXPECT_TRUE(filter->inputConsumer_ == nullptr);
    delete codecMem;
}

HWTEST_F(AvTransportAudioEncoderFilterTest, ProcessData_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(avAudioEncoderTest_ != nullptr);
    avAudioEncoderTest_->isEncoderRunning_.store(true);
    OH_AVCodec audioDecoder(AVMagic::AVCODEC_MAGIC_AUDIO_ENCODER);
    avAudioEncoderTest_->audioEncoder_ = &audioDecoder;
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status status = avAudioEncoderTest_->ProcessData(audioData, 0, nullptr);
    avAudioEncoderTest_->isEncoderRunning_.store(false);
    avAudioEncoderTest_->audioEncoder_ = nullptr;
    EXPECT_EQ(Status::ERROR_NULL_POINTER, status);
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnEncError_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    int32_t normalErrorCode = 100;
    filter->OnEncError(normalErrorCode);
    EXPECT_FALSE(filter->isEncoderRunning_.load());
    int32_t boundaryErrorCode = -1;
    filter->OnEncError(boundaryErrorCode);
    EXPECT_FALSE(filter->isEncoderRunning_.load());
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnEncOutputFormatChanged, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(filter != nullptr);
    OH_AVFormat *nullFormat = nullptr;
    filter->OnEncOutputFormatChanged(nullFormat);
    OH_AVFormat *validFormat = new (std::nothrow) OH_AVFormat();
    filter->OnEncOutputFormatChanged(validFormat);
    delete validFormat;
}

HWTEST_F(AvTransportAudioEncoderFilterTest, OnEncInputBufferAvailable_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
                                                       Pipeline::FilterType::FILTERTYPE_AENC);
    ASSERT_TRUE(filter != nullptr);
    OH_AVBuffer *nullBuffer = nullptr;
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    filter->OnEncInputBufferAvailable(0, nullBuffer);
    for (int i = 0; i < 10; i++)
    {
        OH_AVBuffer *buffer = new OH_AVBuffer(audioData);
        filter->OnEncInputBufferAvailable(i, buffer);
        delete buffer;
    }
    OH_AVBuffer *normalBuffer = new OH_AVBuffer(audioData);
    filter->OnEncInputBufferAvailable(10, normalBuffer);
    delete normalBuffer;
}
} // namespace DistributedHardware
} // namespace OHOS