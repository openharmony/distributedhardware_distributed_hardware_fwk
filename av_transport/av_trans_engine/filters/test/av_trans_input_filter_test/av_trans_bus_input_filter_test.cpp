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

#include "av_trans_bus_input_filter_test.h"
#include "av_trans_constants.h"
#include "av_trans_audio_encoder_filter.h"
#include "av_trans_types.h"
using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string INPUT_BUFFER_QUEUE_NAME = "AVTransBusInputBufferQueue";

void AvTransportBusInputFilterTest::SetUp()
{
}

void AvTransportBusInputFilterTest::TearDown()
{
}

void AvTransportBusInputFilterTest::SetUpTestCase()
{
}

void AvTransportBusInputFilterTest::TearDownTestCase()
{
}

HWTEST_F(AvTransportBusInputFilterTest, Init_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    avBusInputTest_->Init(nullptr, nullptr);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
}

HWTEST_F(AvTransportBusInputFilterTest, DoInitAfterLink_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoInitAfterLink();
    EXPECT_EQ(ret, Status::OK);
}

HWTEST_F(AvTransportBusInputFilterTest, PrepareInputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    avBusInputTest_->PrepareInputBuffer();
    ASSERT_TRUE(avBusInputTest_ != nullptr);
}

HWTEST_F(AvTransportBusInputFilterTest, ProcessAndSendBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status ret = avBusInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}


HWTEST_F(AvTransportBusInputFilterTest, ProcessAndSendBuffer_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status ret = avBusInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    // Create memory
    if (audioData->memory_ == nullptr) {
        audioData->memory_ = std::make_shared<Media::AVMemory>();
        ASSERT_TRUE(audioData->memory_ != nullptr);
    }
    ret = avBusInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    // Create audioData meta
    if (audioData->meta_ == nullptr) {
        audioData->meta_ = std::make_shared<Media::Meta>();
        ASSERT_TRUE(audioData->meta_ != nullptr);
    }
    ret = avBusInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    // set meta parameter
    if (avBusInputTest_->meta_ == nullptr) {
        avBusInputTest_->meta_ = std::make_shared<Media::Meta>();
        ASSERT_TRUE(avBusInputTest_->meta_ != nullptr);
    }
    uint64_t pts = 8434234234;
    uint32_t frameNumber = 20;
    BufferDataType dataType = BufferDataType::AUDIO;
    audioData->meta_->SetData(Media::Tag::MEDIA_STREAM_TYPE, dataType);
    audioData->meta_->SetData(Media::Tag::AUDIO_OBJECT_NUMBER, frameNumber);
    audioData->meta_->SetData(Media::Tag::USER_FRAME_PTS, pts);
    ret = avBusInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, ProcessAndSendBuffer_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    std::shared_ptr<Media::AVBufferQueue> outputBufQue = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
        Media::MemoryType::VIRTUAL_MEMORY, INPUT_BUFFER_QUEUE_NAME);
    avBusInputTest_->outputBufQueProducer_ = outputBufQue->GetProducer();
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status ret = avBusInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, ProcessAndSendBuffer_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.audio.input",
            Pipeline::FilterType::FILTERTYPE_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    // Create memory
    if (audioData->memory_ == nullptr) {
        audioData->memory_ = std::make_shared<Media::AVMemory>();
        ASSERT_TRUE(audioData->memory_ != nullptr);
    }
    std::shared_ptr<Media::AVBufferQueue> outputBufQue = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
        Media::MemoryType::VIRTUAL_MEMORY, INPUT_BUFFER_QUEUE_NAME);
    avBusInputTest_->outputBufQueProducer_ = outputBufQue->GetProducer();
    // Create audioData meta
    if (audioData->meta_ == nullptr) {
        audioData->meta_ = std::make_shared<Media::Meta>();
        ASSERT_TRUE(audioData->meta_ != nullptr);
    }
    // set meta parameter
    if (avBusInputTest_->meta_ == nullptr) {
        avBusInputTest_->meta_ = std::make_shared<Media::Meta>();
        ASSERT_TRUE(avBusInputTest_->meta_ != nullptr);
    }
    uint64_t pts = 8434234234;
    uint32_t frameNumber = 20;
    BufferDataType dataType = BufferDataType::AUDIO;
    audioData->meta_->SetData(Media::Tag::MEDIA_STREAM_TYPE, dataType);
    audioData->meta_->SetData(Media::Tag::AUDIO_OBJECT_NUMBER, frameNumber);
    audioData->meta_->SetData(Media::Tag::USER_FRAME_PTS, pts);
    Status ret = avBusInputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoProcessOutputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoProcessOutputBuffer(0, true, true, 0, 0);
    EXPECT_EQ(ret, Status::OK);
}

HWTEST_F(AvTransportBusInputFilterTest, DoPrepare_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoPrepare();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoPrepare_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    avBusInputTest_->meta_ = std::make_shared<Media::Meta>();
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, "ownerName_111", "ownerName_111");
    auto str = cJSON_PrintUnformatted(jsonObj);
    std::string jsonStr = std::string(str);
    cJSON_free(str);
    cJSON_Delete(jsonObj);
    avBusInputTest_->meta_->SetData(Media::Tag::MEDIA_DESCRIPTION, jsonStr);
    Status ret = avBusInputTest_->DoPrepare();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoPrepare_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    avBusInputTest_->meta_ = std::make_shared<Media::Meta>();
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, KEY_ONWER_NAME.c_str(), "ownerName_111");
    auto str = cJSON_PrintUnformatted(jsonObj);
    std::string jsonStr = std::string(str);
    cJSON_free(str);
    cJSON_Delete(jsonObj);
    avBusInputTest_->meta_->SetData(Media::Tag::MEDIA_DESCRIPTION, jsonStr);
    Status ret = avBusInputTest_->DoPrepare();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoPrepare_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    avBusInputTest_->meta_ = std::make_shared<Media::Meta>();
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    cJSON_AddStringToObject(jsonObj, KEY_ONWER_NAME.c_str(), "ownerName_111");
    cJSON_AddStringToObject(jsonObj, KEY_PEERDEVID_NAME.c_str(), "peerDevId_111");
    auto str = cJSON_PrintUnformatted(jsonObj);
    std::string jsonStr = std::string(str);
    cJSON_free(str);
    cJSON_Delete(jsonObj);
    avBusInputTest_->meta_->SetData(Media::Tag::MEDIA_DESCRIPTION, jsonStr);
    Status ret = avBusInputTest_->DoPrepare();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoStart_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoStart();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoStop_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoStop();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoPause_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoPause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoPauseDragging_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoPauseDragging();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoResume_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoResume();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoResumeDragging_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoResumeDragging();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoFlush_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoFlush();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoRelease_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->DoRelease();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoProcessInputBuffer_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);

    Status ret = avBusInputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);

    avBusInputTest_->inputBufQue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
                                                                Media::MemoryType::VIRTUAL_MEMORY,
                                                                INPUT_BUFFER_QUEUE_NAME);
    ret = avBusInputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_WRONG_STATE, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, DoProcessInputBuffer_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    avBusInputTest_->PrepareInputBuffer();
    avBusInputTest_->curState_ = Pipeline::FilterState::RUNNING;
    Status ret = avBusInputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, LinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = avBusInputTest_->LinkNext(avAudioEncoderTest_, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, OnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->OnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, UpdateNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->UpdateNext(nullptr, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, UnLinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->UnLinkNext(nullptr, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, OnUpdated_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->OnUpdated(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, OnUnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    Status ret = avBusInputTest_->OnUnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts= 0;
    int64_t ptsSpecial= 0;
    bool ret = avBusInputTest_->UnmarshalAudioMeta("", pts, ptsSpecial);
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts= 1;
    int64_t ptsSpecial= 1;
    bool ret = avBusInputTest_->UnmarshalAudioMeta("AUDIO", pts, ptsSpecial);
    EXPECT_EQ(false, ret);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_003, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "123456");
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "789012");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);

    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);

    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_004, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddStringToObject(jsonObj, "meta_timestamp", "not_a_number");
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "123456");
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "789012");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);

    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);

    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_005, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 123456);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "789012");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);

    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);

    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_006, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 123456);
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp_string", 123456);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "789012");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);

    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);

    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_007, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 123456);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "");
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "789012");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);

    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);

    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_008, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 123456);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "123456");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);

    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);
    
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}


HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_009, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 123456);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "123456");
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp_special", 789012);
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);

    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);
    
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_010, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 123456);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "123456");
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);
    
    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(false, ret);
    
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_011, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 123456);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "123456");
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "789012");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);
    
    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(123456, pts);
    EXPECT_EQ(789012, ptsSpecial);
    
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, UnmarshalAudioMeta_012, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    int64_t pts = 0;
    int64_t ptsSpecial = 0;

    cJSON *jsonObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(jsonObj, "meta_timestamp", 9876543210LL);
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_string", "9876543210");
    cJSON_AddStringToObject(jsonObj, "meta_timestamp_special", "1234567890");
    char *jsonStr = cJSON_PrintUnformatted(jsonObj);
    
    bool ret = avBusInputTest_->UnmarshalAudioMeta(std::string(jsonStr), pts, ptsSpecial);
    EXPECT_EQ(true, ret);
    EXPECT_EQ(9876543210LL, pts);
    EXPECT_EQ(1234567890LL, ptsSpecial);
    
    cJSON_free(jsonStr);
    cJSON_Delete(jsonObj);
}

HWTEST_F(AvTransportBusInputFilterTest, TransName2PkgName_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    std::string test = "TEST";
    std::string result = avBusInputTest_->TransName2PkgName(test);
    EXPECT_EQ(EMPTY_STRING, result);
}

HWTEST_F(AvTransportBusInputFilterTest, TransName2PkgName_002, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    std::string result = avBusInputTest_->TransName2PkgName(OWNER_NAME_D_SCREEN);
    EXPECT_EQ(PKG_NAME_D_SCREEN, result);
}

HWTEST_F(AvTransportBusInputFilterTest, OnChannelEvent_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avBusInputTest_ =
        std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
            Pipeline::FilterType::AUDIO_DATA_SOURCE);
    ASSERT_TRUE(avBusInputTest_ != nullptr);
    avBusInputTest_->receiver_ = std::make_shared<EventReceiverTest>();
    std::shared_ptr<EventReceiverTest> receiverTest_ = std::static_pointer_cast<EventReceiverTest>(
        avBusInputTest_->receiver_);

    receiverTest_->type_ = OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY;
    AVTransEvent event;
    event.type = EventType::EVENT_CHANNEL_OPENED;
    avBusInputTest_->OnChannelEvent(event);
    EXPECT_EQ(receiverTest_->type_, OHOS::DistributedHardware::Pipeline::EventType::EVENT_AUDIO_PROGRESS);

    receiverTest_->type_ = OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY;
    event.type = EventType::EVENT_CHANNEL_OPEN_FAIL;
    avBusInputTest_->OnChannelEvent(event);
    EXPECT_EQ(receiverTest_->type_, OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY);

    receiverTest_->type_ = OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY;
    event.type = EventType::EVENT_CHANNEL_CLOSED;
    avBusInputTest_->OnChannelEvent(event);
    EXPECT_EQ(receiverTest_->type_, OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY);

    receiverTest_->type_ = OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY;
    event.type = EventType::EVENT_REMOVE_STREAM;
    avBusInputTest_->OnChannelEvent(event);
    EXPECT_EQ(receiverTest_->type_, OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY);
}
} // namespace DistributedHardware
} // namespace OHOS