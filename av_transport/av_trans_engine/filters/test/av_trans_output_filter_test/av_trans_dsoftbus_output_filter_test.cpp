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

#include "av_trans_dsoftbus_output_filter_test.h"
#include "av_trans_audio_encoder_filter.h"
#include "av_trans_constants.h"
#include "av_trans_types.h"
#include "cJSON.h"
using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string INPUT_BUFFER_QUEUE_NAME = "AVTransAudioInputBufferQueue";

void AvTransportAudioOutputFilterTest::SetUp()
{
    dSoftbusOutputTest_ = std::make_shared<Pipeline::DSoftbusOutputFilter>("builtin.daudio.output",
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
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::string receiverId = std::string("AVreceiverEngineTest");
    dSoftbusOutputTest_->Init(nullptr, nullptr);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoInitAfterLink_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoInitAfterLink();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, PrepareInputBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    dSoftbusOutputTest_->PrepareInputBuffer();
    EXPECT_NE(dSoftbusOutputTest_->outputBufQue_, nullptr);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoPrepare_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoPrepare();
    EXPECT_NE(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoStart_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoStart();
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoPause_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoPause();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoPauseDragging_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoPauseDragging();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoResume_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoResume();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoResumeDragging_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoResumeDragging();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoStop_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoStop();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoFlush_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoFlush();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoRelease_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    Status ret = dSoftbusOutputTest_->DoRelease();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoProcessInputBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    dSoftbusOutputTest_->PrepareInputBuffer();
    Status ret = dSoftbusOutputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
    
    dSoftbusOutputTest_->outputBufQue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM,
        Media::MemoryType::VIRTUAL_MEMORY, INPUT_BUFFER_QUEUE_NAME);
    ret = dSoftbusOutputTest_->DoProcessInputBuffer(1, true);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoProcessOutputBuffer_001, testing::ext::TestSize.Level1)
{
    Status ret = dSoftbusOutputTest_->DoProcessOutputBuffer(1, true, true, 2, 3);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, ProcessAndSendBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status ret = dSoftbusOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, SetParameterAndGetParameter_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Media::Meta> meta = nullptr;
    std::shared_ptr<Media::Meta> meta2 = std::make_shared<Media::Meta>();
    dSoftbusOutputTest_->SetParameter(meta2);
    dSoftbusOutputTest_->GetParameter(meta);
    EXPECT_EQ(meta, meta2);
}

HWTEST_F(AvTransportAudioOutputFilterTest, LinkNext_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dSoftbusOutputTest_->LinkNext(avAudioEncoderTest_, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, UpdateNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter2",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dSoftbusOutputTest_->UpdateNext(filter, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, UnLinkNext_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter2",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dSoftbusOutputTest_->UnLinkNext(filter, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, OnLinked_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    Status ret = dSoftbusOutputTest_->OnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, meta, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, OnUpdated_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    Status ret = dSoftbusOutputTest_->OnUpdated(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, meta, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, OnUnLinked_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<Pipeline::AudioEncoderFilter> filter =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter2",
            Pipeline::FilterType::FILTERTYPE_AENC);
    Status ret = dSoftbusOutputTest_->OnUnLinked(Pipeline::StreamType::STREAMTYPE_RAW_AUDIO, nullptr);
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, DoPrepare_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    ASSERT_TRUE(meta != nullptr);
    dSoftbusOutputTest_->SetParameter(meta);
    Status ret = dSoftbusOutputTest_->DoPrepare();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    cJSON *jsonObj = cJSON_CreateObject();
    ASSERT_TRUE(jsonObj != nullptr);
    ret = dSoftbusOutputTest_->DoPrepare();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    std::string ownerName = "audiotest";
    cJSON_AddStringToObject(jsonObj, KEY_ONWER_NAME.c_str(), ownerName.c_str());
    auto str = cJSON_PrintUnformatted(jsonObj);
    ASSERT_TRUE(str != nullptr);
    std::string jsonStr = std::string(str);
    cJSON_free(str);
    meta->SetData(Media::Tag::MEDIA_DESCRIPTION, jsonStr);
    dSoftbusOutputTest_->SetParameter(meta);
    ret = dSoftbusOutputTest_->DoPrepare();
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    std::string peerDevId = "2";
    cJSON_AddStringToObject(jsonObj, KEY_PEERDEVID_NAME.c_str(), peerDevId.c_str());
    str = cJSON_PrintUnformatted(jsonObj);
    ASSERT_TRUE(str != nullptr);
    jsonStr = std::string(str);
    cJSON_free(str);
    cJSON_Delete(jsonObj);
    meta->SetData(Media::Tag::MEDIA_DESCRIPTION, jsonStr);
    dSoftbusOutputTest_->SetParameter(meta);
    std::shared_ptr<Pipeline::AudioEncoderFilter> avAudioEncoderTest_ =
        std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.audioencoderfilter",
            Pipeline::FilterType::FILTERTYPE_AENC);
    ret = dSoftbusOutputTest_->LinkNext(avAudioEncoderTest_, Pipeline::StreamType::STREAMTYPE_RAW_AUDIO);
    ASSERT_TRUE(ret == Status::OK);
    ret = dSoftbusOutputTest_->DoPrepare();
    EXPECT_EQ(Status::OK, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, MarshalAudioMeta_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::string json = dSoftbusOutputTest_->MarshalAudioMeta(BufferDataType::AUDIO, 1000, 2000, 1);
    std::string json1 = std::string(R"({"meta_data_type":0,)") +
                        std::string(R"("meta_timestamp":1000,)") +
                        std::string(R"("meta_frame_number":1,)") +
                        std::string(R"("meta_timestamp_string":"1000",)") +
                        std::string(R"("meta_timestamp_special":"2000"})");
    EXPECT_EQ(json, json1);
}

HWTEST_F(AvTransportAudioOutputFilterTest, ProcessAndSendBuffer_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(dSoftbusOutputTest_ != nullptr);
    std::shared_ptr<Media::AVBuffer> audioData = std::make_shared<Media::AVBuffer>();
    Status ret = dSoftbusOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    // Create memory
    if (audioData->memory_ == nullptr) {
        audioData->memory_ = std::make_shared<Media::AVMemory>();
        ASSERT_TRUE(audioData->memory_ != nullptr);
    }
    ret = dSoftbusOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    // Create audioData meta
    if (audioData->meta_ == nullptr) {
        audioData->meta_ = std::make_shared<Media::Meta>();
        ASSERT_TRUE(audioData->meta_ != nullptr);
    }
    ret = dSoftbusOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_NULL_POINTER, ret);
    // set meta parameter
    if (dSoftbusOutputTest_->meta_ == nullptr) {
        dSoftbusOutputTest_->meta_ = std::make_shared<Media::Meta>();
        ASSERT_TRUE(dSoftbusOutputTest_->meta_ != nullptr);
    }
    uint64_t pts = 8434234234;
    uint32_t frameNumber = 20;
    BufferDataType dataType = BufferDataType::AUDIO;

    audioData->meta_->SetData(Media::Tag::MEDIA_STREAM_TYPE, dataType);
    audioData->meta_->SetData(Media::Tag::AUDIO_OBJECT_NUMBER, frameNumber);
    audioData->meta_->SetData(Media::Tag::USER_FRAME_PTS, pts);

    ret = dSoftbusOutputTest_->ProcessAndSendBuffer(audioData);
    EXPECT_EQ(Status::ERROR_INVALID_OPERATION, ret);
}

HWTEST_F(AvTransportAudioOutputFilterTest, OnChannelEvent_001, testing::ext::TestSize.Level1)
{
    AVTransEvent event;
    event.type = EventType::EVENT_REMOVE_STREAM;
    dSoftbusOutputTest_->OnChannelEvent(event);
    dSoftbusOutputTest_->eventReceiver_ = std::make_shared<EventReceiverTest>();
    std::shared_ptr<EventReceiverTest> eventReceiverTest_ = std::static_pointer_cast<EventReceiverTest>(
        dSoftbusOutputTest_->eventReceiver_);
    eventReceiverTest_->type_ = OHOS::DistributedHardware::Pipeline::EventType::EVENT_READY;
    dSoftbusOutputTest_->OnChannelEvent(event);
    EXPECT_EQ(OHOS::DistributedHardware::Pipeline::EventType::EVENT_AUDIO_PROGRESS, eventReceiverTest_->type_);
}
} // namespace DistributedHardware
} // namespace OHOS