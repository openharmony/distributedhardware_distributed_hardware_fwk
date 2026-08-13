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

#ifndef MEDIA_PIPELINE_DECODER_FILTER_H
#define MEDIA_PIPELINE_DECODER_FILTER_H

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "pipeline_status.h"
#include "filter.h"

#include "avcodec_audio_decoder.h"
#include "avcodec_codec_name.h"
#include "avcodec_common.h"
#include "avcodec_errors.h"
#include "buffer/avbuffer_queue.h"
#include "buffer/avbuffer_queue_consumer.h"
#include "buffer/avbuffer_queue_producer.h"
#include "media_description.h"
#include "common/native_mfmagic.h"
#include "securec.h"
#include "native_avcodec_audiocodec.h"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
typedef struct {
    AudioCodecType codecType;
    int32_t channel;
    int32_t sampleRate;
    int32_t bitRate;
    MediaAVCodec::AudioSampleFormat sampleDepth;
} ADecInitParams;

class AudioDecoderFilter : public Filter, public std::enable_shared_from_this<AudioDecoderFilter> {
public:
    AudioDecoderFilter(std::string name, FilterType type);
    ~AudioDecoderFilter() override;
    void Init(const std::shared_ptr<EventReceiver> &receiver, const std::shared_ptr<FilterCallback> &callback) override;
    Status DoInitAfterLink() override;
    Status DoPrepare() override;
    Status DoStart() override;
    Status DoPause() override;
    Status DoPauseDragging() override;
    Status DoResume() override;
    Status DoResumeDragging() override;
    Status DoStop() override;
    Status DoFlush() override;
    Status DoRelease() override;

    Status DoProcessInputBuffer(int recvArg, bool dropFrame) override;
    Status DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx, uint32_t idx, int64_t renderTime) override;
    void SetParameter(const std::shared_ptr<Media::Meta>& meta) override;
    void GetParameter(std::shared_ptr<Media::Meta>& meta) override;

    Status LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType) override;
    Status UpdateNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType) override;
    Status UnLinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType) override;

    Status OnLinked(StreamType inType, const std::shared_ptr<Media::Meta>& meta,
                            const std::shared_ptr<FilterLinkCallback>& callback) override;
    Status OnUpdated(StreamType inType, const std::shared_ptr<Media::Meta>& meta,
                             const std::shared_ptr<FilterLinkCallback>& callback) override;
    Status OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback>& callback) override;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue, std::shared_ptr<Media::Meta>& meta);
    void OnUnLinkedResult(std::shared_ptr<Media::Meta>& meta);
    void OnUpdatedResult(std::shared_ptr<Media::Meta>& meta);

    Status CreateAudioCodec();
    Status ConfigureAudioCodec(const ADecInitParams &initDecParams);
    Status ReleaseAudioCodec();

    void OnDecError(int32_t errorCode);
    void OnDecOutputFormatChanged(const OH_AVFormat *format);
    void OnDecInputBufferAvailable(uint32_t index, OH_AVBuffer *buffer);
    void OnDecOutputBufferAvailable(uint32_t index, OH_AVBuffer *buffer);

private:
    Status PrepareInputBufferQueue();
    Status SetDecoderFormat(const ADecInitParams &initDecParams);
    Status CheckDecoderFormat(const ADecInitParams &initDecParams);
    Status StartAudioCodec();
    Status StopAudioCodec();
    void StartInputThread();
    void StopInputThread();
    void InputDecodeAudioData();
    Status ProcessData(std::shared_ptr<Media::AVBuffer> audioData, const uint32_t index, OH_AVBuffer *codecMem);

private:
    constexpr static int32_t AUDIO_DECODER_QUEUE_MAX = 100;
    constexpr static int32_t DECODE_WAIT_MILLISECONDS = 50;
    constexpr static int64_t MAX_TIME_OUT_MS = 1;
    constexpr static int32_t CHANNEL_MASK_MIN = 1;
    constexpr static int32_t CHANNEL_MASK_MAX = 2;
    constexpr static int32_t SAMPLE_RATE_MIN = 8000;
    constexpr static int32_t SAMPLE_RATE_MAX = 96000;
    constexpr static int32_t BITRATE_OPUS = 32000;
    constexpr static int32_t INDEX_FLAG = 15;

    std::shared_ptr<Filter> nextFilter_ {nullptr};
    std::shared_ptr<EventReceiver> eventReceiver_ {nullptr};
    std::shared_ptr<FilterCallback> filterCallback_ {nullptr};
    std::shared_ptr<FilterLinkCallback> filterLinkCallback_ {nullptr};
    std::shared_ptr<FilterLinkCallback> onLinkedResultCallback_ {nullptr};
    std::mutex nextFiltersMutex_;
    sptr<Media::AVBufferQueueProducer> inputProducer_ {nullptr};
    sptr<Media::AVBufferQueueProducer> outputProducer_ {nullptr};
    sptr<Media::AVBufferQueueConsumer> inputConsumer_ {nullptr};
    std::shared_ptr<Media::AVBufferQueue> inputBufferQueue_ {nullptr};
    static constexpr int32_t DEFAULT_BUFFER_NUM = 8;
    const std::string INPUT_BUFFERQUEUE_NAME = "AvTransDecoderBufferQueue";
    static constexpr const char *DECODE_THREAD = "decodeFilterThread";

    std::mutex mtxData_;
    std::mutex mtxCnt_;
    std::thread decoderThread_;
    std::condition_variable decodeCond_;
    std::atomic<bool> isDecoderRunning_ = false;
    std::atomic<bool> isStoped_ = false;

    OH_AVFormat outputFormat_ = {};
    ADecInitParams initDecParams_ = {
        initDecParams_.codecType = AudioCodecType::AUDIO_CODEC_AAC,
        initDecParams_.channel = 2,
        initDecParams_.sampleRate = 44100,
        initDecParams_.bitRate = 12800,
        initDecParams_.sampleDepth = MediaAVCodec::SAMPLE_S16LE
    };
    OH_AVCodec *audioDecoder_ = nullptr;
    std::queue<OH_AVBuffer *> codecBufQueue_;
    std::queue<uint32_t> codecIndexQueue_;
    std::queue<std::shared_ptr<Media::AVBuffer>> inputDataBufferQueue_;
    std::map<uint32_t, int64_t> ptsMap_;
    std::mutex ptsMutex_;
    uint64_t frameInIndex_ = 0;
    uint64_t frameOutIndex_ = 0;
};
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
#endif