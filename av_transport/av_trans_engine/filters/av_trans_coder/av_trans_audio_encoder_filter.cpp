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

#include "av_trans_audio_encoder_filter.h"

#include "av_trans_log.h"
#include "av_sync_utils.h"
#include "filter_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioEncoderFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
static AutoRegisterFilter<AudioEncoderFilter>g_registerAudioEncoderFilter("builtin.recorder.audioencoderfilter",
    FilterType::FILTERTYPE_AENC,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<AudioEncoderFilter>(name, FilterType::FILTERTYPE_AENC);
    });

class AudioEncoderFilterCallback : public FilterLinkCallback {
public:
    explicit AudioEncoderFilterCallback(std::shared_ptr<AudioEncoderFilter> filter)
        : inputFilter_(std::move(filter)) {}
    ~AudioEncoderFilterCallback() = default;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue,
        std::shared_ptr<Media::Meta>& meta) override
    {
        if (auto filter = inputFilter_.lock()) {
            filter->OnLinkedResult(queue, meta);
        } else {
            AVTRANS_LOGE("Invalid inputfilter");
        }
    }
    void OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta) override
    {
        if (auto filter = inputFilter_.lock()) {
            filter->OnUnLinkedResult(meta);
        } else {
            AVTRANS_LOGE("Invalid inputfilter");
        }
    }
    void OnUpdatedResult(std::shared_ptr<Media::Meta>& meta) override
    {
        if (auto filter = inputFilter_.lock()) {
            filter->OnUpdatedResult(meta);
        } else {
            AVTRANS_LOGE("Invalid inputfilter");
        }
    }
private:
    std::weak_ptr<AudioEncoderFilter> inputFilter_ {};
};

class AVBufferAvaliableListener : public Media::IConsumerListener {
public:
    explicit AVBufferAvaliableListener(std::weak_ptr<AudioEncoderFilter> encoder)
    {
        encoder_ = encoder;
    }
    void OnBufferAvailable() override
    {
        auto encoder = encoder_.lock();
        TRUE_RETURN(encoder == nullptr, "encoder is nullptr");
        encoder->ProcessInputBuffer();
    }
private:
    std::weak_ptr<AudioEncoderFilter> encoder_;
};

static void OnError(OH_AVCodec *codec, int32_t errorCode, void *userData)
{
    (void)codec;
    AudioEncoderFilter *encoder = static_cast<AudioEncoderFilter *>(userData);
    TRUE_RETURN(encoder == nullptr, "encoder is nullptr");
    encoder->OnEncError(errorCode);
}

static void OnOutputFormatChanged(OH_AVCodec *codec, OH_AVFormat *format, void *userData)
{
    (void)codec;
    AudioEncoderFilter *encoder = static_cast<AudioEncoderFilter *>(userData);
    TRUE_RETURN(encoder == nullptr, "encoder is nullptr");
    encoder->OnEncOutputFormatChanged(format);
}

static void OnInputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    (void)codec;
    AudioEncoderFilter *encoder = static_cast<AudioEncoderFilter *>(userData);
    TRUE_RETURN(encoder == nullptr, "encoder is nullptr");
    encoder->OnEncInputBufferAvailable(index, buffer);
}

static void OnOutputBufferAvailable(OH_AVCodec *codec, uint32_t index, OH_AVBuffer *buffer, void *userData)
{
    (void)codec;
    AudioEncoderFilter *encoder = static_cast<AudioEncoderFilter *>(userData);
    TRUE_RETURN(encoder == nullptr, "encoder is nullptr");
    encoder->OnEncOutputBufferAvailable(index, buffer);
}

AudioEncoderFilter::AudioEncoderFilter(std::string name, FilterType type) : Filter(name, type)
{
}

AudioEncoderFilter::~AudioEncoderFilter()
{
    AVTRANS_LOGI("enter");
    if (audioEncoder_ != nullptr) {
        AVTRANS_LOGI("release audio encoder");
        ReleaseAudioCodec();
    }
}

void AudioEncoderFilter::Init(const std::shared_ptr<EventReceiver>& receiver,
    const std::shared_ptr<FilterCallback>& callback)
{
    receiver_ = receiver;
    callback_ = callback;
}

Status AudioEncoderFilter::DoInitAfterLink()
{
    return Status::OK;
}

Status AudioEncoderFilter::PrepareInputBufferQueue()
{
    AVTRANS_LOGI("enter");
    if (inputBufferQueue_ == nullptr) {
        inputBufferQueue_ = Media::AVBufferQueue::Create(DEFAULT_BUFFER_NUM, Media::MemoryType::VIRTUAL_MEMORY,
            INPUT_BUFFERQUEUE_NAME);
    }
    TRUE_RETURN_V_MSG_E(inputBufferQueue_ == nullptr, Status::ERROR_NULL_POINTER, "create bufferqueue failed");
    inputProducer_ = inputBufferQueue_->GetProducer();
    TRUE_RETURN_V_MSG_E(inputProducer_ == nullptr, Status::ERROR_NULL_POINTER, "GetProducer failed");
    inputConsumer_ = inputBufferQueue_->GetConsumer();
    TRUE_RETURN_V_MSG_E(inputConsumer_ == nullptr, Status::ERROR_NULL_POINTER, "GetConsumer failed");

    sptr<Media::IConsumerListener> listener(new AVBufferAvaliableListener(shared_from_this()));
    inputConsumer_->SetBufferAvailableListener(listener);
    return Status::OK;
}

Status AudioEncoderFilter::DoPrepare()
{
    AVTRANS_LOGI("enter");
    auto ret = PrepareInputBufferQueue();
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "PrepareInputBufferQueue failed");

    ret = CreateAudioCodec();
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "Create AudioCodec failed");
    ret = ConfigureAudioCodec(initEncParams_);
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "Configure AudioCodec failed");

    TRUE_RETURN_V_MSG_E(callback_ == nullptr, Status::ERROR_NULL_POINTER, "callback is nullptr");
    callback_->OnCallback(shared_from_this(), FilterCallBackCommand::NEXT_FILTER_NEEDED,
        StreamType::STREAMTYPE_RAW_AUDIO);
    return Status::OK;
}

Status AudioEncoderFilter::DoStart()
{
    AVTRANS_LOGI("enter");
    auto ret = StartAudioCodec();
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "StartAudioCodec failed");
    return Status::OK;
}

Status AudioEncoderFilter::DoPause()
{
    return Status::OK;
}

Status AudioEncoderFilter::DoPauseDragging()
{
    return Status::OK;
}

Status AudioEncoderFilter::DoResume()
{
    return Status::OK;
}

Status AudioEncoderFilter::DoResumeDragging()
{
    return Status::OK;
}

Status AudioEncoderFilter::DoStop()
{
    AVTRANS_LOGI("enter");
    auto ret = StopAudioCodec();
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "StopAudioCodec failed");
    return Status::OK;
}

Status AudioEncoderFilter::DoFlush()
{
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, Status::ERROR_NULL_POINTER, "audioCodec is null");
    auto res = OH_AudioCodec_Flush(audioEncoder_);
    TRUE_RETURN_V_MSG_E(res != AV_ERR_OK, Status::ERROR_INVALID_OPERATION,
        "audioEncoder flush failed: %{public}d", res);
    return Status::OK;
}

Status AudioEncoderFilter::DoRelease()
{
    AVTRANS_LOGI("enter");
    auto ret = ReleaseAudioCodec();
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "StartAudioCodec failed");
    return Status::OK;
}

Status AudioEncoderFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    AVTRANS_LOGD("enter");
    std::shared_ptr<Media::AVBuffer> buffer = nullptr;
    TRUE_RETURN_V_MSG_E(inputConsumer_ == nullptr, Status::ERROR_NULL_POINTER, "inputConsumer is null");
    Media::Status ret = inputConsumer_->AcquireBuffer(buffer);
    TRUE_RETURN_V_MSG_E(ret != Media::Status::OK, Status::ERROR_INVALID_OPERATION, "AcquireBuffer failed");
    {
        std::lock_guard<std::mutex> datalock(mtxData_);
        while (inputDataBufferQueue_.size() > AUDIO_ENCODER_QUEUE_MAX) {
            AVTRANS_LOGE("inputDataBufferQueue_ overflow");
            auto frontBuffer = inputDataBufferQueue_.front();
            inputDataBufferQueue_.pop();
            if (frontBuffer != nullptr) {
                inputConsumer_->ReleaseBuffer(frontBuffer);
            }
        }
        inputDataBufferQueue_.push(buffer);
    }
    encodeCond_.notify_all();
    return Status::OK;
}

Status AudioEncoderFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame,
    bool byIdx, uint32_t idx, int64_t renderTime)
{
    return Status::OK;
}

void AudioEncoderFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void AudioEncoderFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status AudioEncoderFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    AVTRANS_LOGI("enter");
    TRUE_RETURN_V_MSG_E(nextFilter == nullptr, Status::ERROR_NULL_POINTER, "input nextFilter is nullptr");
    nextFilter_ = nextFilter;
    {
        std::lock_guard<std::mutex> lock(nextFiltersMutex_);
        nextFiltersMap_[outType].push_back(nextFilter);
    }
    auto filterLinkCallback = std::make_shared<AudioEncoderFilterCallback>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, meta_, filterLinkCallback);
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "Onlinked failed: %{public}d", ret);
    return Status::OK;
}

Status AudioEncoderFilter::UpdateNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    return Status::OK;
}

Status AudioEncoderFilter::UnLinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    std::lock_guard<std::mutex> lock(nextFiltersMutex_);
    nextFiltersMap_.clear();
    return Status::OK;
}

Status AudioEncoderFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta>& meta,
    const std::shared_ptr<FilterLinkCallback>& callback)
{
    AVTRANS_LOGI("enter");
    TRUE_RETURN_V_MSG_E(callback == nullptr, Status::ERROR_NULL_POINTER, "input callback is nullptr");
    onLinkedResultCallback_ = callback;
    TRUE_RETURN_V_MSG_E(meta == nullptr, Status::ERROR_NULL_POINTER, "input meta is nullptr");
    meta->GetData(Media::Tag::AUDIO_CHANNEL_COUNT, initEncParams_.channel);
    meta->GetData(Media::Tag::AUDIO_SAMPLE_RATE, initEncParams_.sampleRate);
    meta->GetData(Media::Tag::AUDIO_SAMPLE_FORMAT, initEncParams_.sampleDepth);
    meta->GetData(Media::Tag::MEDIA_BITRATE, initEncParams_.bitRate);
    int32_t mimeType = 0;
    meta->GetData(Media::Tag::MIME_TYPE, mimeType);
    initEncParams_.codecType = static_cast<AudioCodecType>(mimeType);
    if (initEncParams_.codecType == AudioCodecType::AUDIO_CODEC_OPUS) {
        initEncParams_.bitRate = BITRATE_OPUS;
    }
    meta_ = meta;
    return Status::OK;
}

Status AudioEncoderFilter::OnUpdated(StreamType inType, const std::shared_ptr<Media::Meta>& meta,
    const std::shared_ptr<FilterLinkCallback>& callback)
{
    return Status::OK;
}

Status AudioEncoderFilter::OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback>& callback)
{
    onLinkedResultCallback_ = nullptr;
    return Status::OK;
}

void AudioEncoderFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue,
    std::shared_ptr<Media::Meta>& meta)
{
    AVTRANS_LOGI("enter");
    TRUE_RETURN(queue == nullptr, "input queue is nullptr");
    outputProducer_ = queue;
    TRUE_RETURN(onLinkedResultCallback_ == nullptr, "onLinkedResultCallback_ is nullptr");
    onLinkedResultCallback_->OnLinkedResult(inputProducer_, meta);
}

void AudioEncoderFilter::OnUnLinkedResult(std::shared_ptr<Media::Meta>& meta)
{
    (void)meta;
}

void AudioEncoderFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
    (void)meta;
}

Status AudioEncoderFilter::CreateAudioCodec()
{
    if (initEncParams_.codecType == AudioCodecType::AUDIO_CODEC_OPUS) {
        AVTRANS_LOGI("encoderType::opus");
        audioEncoder_ = OH_AudioCodec_CreateByName((MediaAVCodec::AVCodecCodecName::AUDIO_ENCODER_OPUS_NAME).data());
    } else {
        AVTRANS_LOGI("encoderType::aac");
        audioEncoder_ = OH_AudioCodec_CreateByName((MediaAVCodec::AVCodecCodecName::AUDIO_ENCODER_AAC_NAME).data());
    }
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, Status::ERROR_NULL_POINTER, "Create AudioCodec failed");
    OH_AVCodecCallback cb = {&OnError, &OnOutputFormatChanged, &OnInputBufferAvailable, &OnOutputBufferAvailable};
    int32_t ret = OH_AudioCodec_RegisterCallback(audioEncoder_, cb, this);
    TRUE_RETURN_V_MSG_E(ret != AV_ERR_OK, Status::ERROR_INVALID_OPERATION, "Codec setCallback failed");
    return Status::OK;
}

Status AudioEncoderFilter::ConfigureAudioCodec(const AEncInitParams &initEncParams)
{
    AVTRANS_LOGI("enter");
    auto ret = SetEncoderFormat(initEncParams);
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "input meta is invalid");
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, Status::ERROR_NULL_POINTER, "audioCodec is null");
    auto res = OH_AudioCodec_Prepare(audioEncoder_);
    TRUE_RETURN_V_MSG_E(res != AV_ERR_OK, Status::ERROR_INVALID_OPERATION,
        "audioEncoder prepare failed: %{public}d", res);
    return Status::OK;
}

Status AudioEncoderFilter::ReleaseAudioCodec()
{
    AVTRANS_LOGI("enter");
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, Status::ERROR_NULL_POINTER, "audioCodec is null");
    bool isSuccess = true;
    if (!isStoped_.load()) {
        auto ret = StopAudioCodec();
        if (ret != Status::OK) {
            AVTRANS_LOGE("Stop before Release AudioCodec failed");
            isSuccess = false;
        }
    }
    auto res = OH_AudioCodec_Destroy(audioEncoder_);
    if (res != AV_ERR_OK) {
        AVTRANS_LOGE("Release AudioCodec failed");
        isSuccess = false;
    }
    audioEncoder_ = nullptr;
    return isSuccess ? Status::OK : Status::ERROR_INVALID_OPERATION;
}

Status AudioEncoderFilter::SetEncoderFormat(const AEncInitParams &initEncParams)
{
    AVTRANS_LOGI("enter");
    auto ret = CheckEncoderFormat(initEncParams);
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ret, "input meta is invalid");
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, Status::ERROR_NULL_POINTER, "audioCodec is null");
    AVTRANS_LOGI("initEncParams channel: %{public}d, sampleRate: %{public}d, sampleDepth: %{public}d",
        initEncParams.channel, initEncParams.sampleRate, initEncParams.sampleDepth);
    OH_AVFormat *format = OH_AVFormat_Create();
    OH_AVFormat_SetIntValue(format, MediaAVCodec::MediaDescriptionKey::MD_KEY_CHANNEL_COUNT.data(),
        initEncParams.channel);
    OH_AVFormat_SetIntValue(format, MediaAVCodec::MediaDescriptionKey::MD_KEY_SAMPLE_RATE.data(),
        initEncParams.sampleRate);
    OH_AVFormat_SetIntValue(format, MediaAVCodec::MediaDescriptionKey::MD_KEY_AUDIO_SAMPLE_FORMAT.data(),
        initEncParams.sampleDepth);
    if (initEncParams.codecType == AudioCodecType::AUDIO_CODEC_OPUS) {
        OH_AVFormat_SetLongValue(format, MediaAVCodec::MediaDescriptionKey::MD_KEY_BITRATE.data(),
            initEncParams.bitRate);
    }
    int32_t res = OH_AudioCodec_Configure(audioEncoder_, format);
    if (res != AV_ERR_OK) {
        AVTRANS_LOGE("configure encoder failed: %{public}d", res);
        OH_AVFormat_Destroy(format);
        return Status::ERROR_INVALID_OPERATION;
    }
    OH_AVFormat_Destroy(format);
    return Status::OK;
}

Status AudioEncoderFilter::CheckEncoderFormat(const AEncInitParams &initEncParams)
{
    AVTRANS_LOGI("enter");
    if (initEncParams.channel >= CHANNEL_MASK_MIN && initEncParams.channel <= CHANNEL_MASK_MAX &&
        initEncParams.sampleRate >= SAMPLE_RATE_MIN && initEncParams.sampleRate <= SAMPLE_RATE_MAX &&
        initEncParams.sampleDepth >= MediaAVCodec::AudioSampleFormat::SAMPLE_U8 &&
        initEncParams.sampleDepth <= MediaAVCodec::AudioSampleFormat::SAMPLE_F32LE) {
            return Status::OK;
    }
    AVTRANS_LOGE("initEncParams from meta err, channel: %{public}d, sampleRate: %{public}d, sampleDepth: %{public}d",
        initEncParams.channel, initEncParams.sampleRate, initEncParams.sampleDepth);
    return Status::ERROR_INVALID_PARAMETER;
}

Status AudioEncoderFilter::StartAudioCodec()
{
    AVTRANS_LOGI("enter");
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, Status::ERROR_NULL_POINTER, "audioCodec is null");
    std::string networkId = DAudioAccessConfigManager::GetInstance().GetCurrentNetworkId();
    if (!networkId.empty()) {
        if (!DAudioAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId)) {
            int32_t timeOut = DAudioAccessConfigManager::GetInstance().GetAccessTimeOut();
            bool gotResult = DAudioAccessConfigManager::GetInstance().WaitForAuthorizationResult(networkId, timeOut);
            if (!gotResult) {
                AVTRANS_LOGE("Authorization timeout, cannot start encoder");
                return Status::ERROR_INVALID_OPERATION;
            }
        }

        if (!DAudioAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId)) {
            AVTRANS_LOGE("Authorization denied, cannot start encoder");
            return Status::ERROR_INVALID_OPERATION;
        }
    }

    auto res = OH_AudioCodec_Start(audioEncoder_);
    TRUE_RETURN_V_MSG_E(res != AV_ERR_OK, Status::ERROR_INVALID_OPERATION,
        "audioEncoder start failed: %{public}d", res);
    StartInputThread();
    return Status::OK;
}

Status AudioEncoderFilter::StopAudioCodec()
{
    AVTRANS_LOGI("enter");
    StopInputThread();
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, Status::ERROR_NULL_POINTER, "audioCodec is null");
    bool isSuccess = true;
    auto ret = OH_AudioCodec_Flush(audioEncoder_);
    if (ret != AV_ERR_OK) {
        AVTRANS_LOGE("OH_AudioCodec_Flush failed");
        isSuccess = false;
    }
    TRUE_RETURN_V_MSG_E(OH_AudioCodec_Stop(audioEncoder_)!= AV_ERR_OK || !isSuccess,
        Status::ERROR_INVALID_OPERATION, "StopAudioCodec failed");
    isStoped_.store(true);
    return Status::OK;
}

void AudioEncoderFilter::StartInputThread()
{
    AVTRANS_LOGI("enter");
    isEncoderRunning_.store(true);
    encoderThread_ = std::thread(&AudioEncoderFilter::InputEncodeAudioData, this);
    if (pthread_setname_np(encoderThread_.native_handle(), ENCODE_THREAD) != AV_ERR_OK) {
        AVTRANS_LOGD("Encode thread setname failed");
    }
}

void AudioEncoderFilter::StopInputThread()
{
    AVTRANS_LOGI("enter");
    isEncoderRunning_.store(false);
    if (encoderThread_.joinable()) {
        encoderThread_.join();
    }
    std::lock_guard<std::mutex> dataLock(mtxData_);
    std::queue<uint32_t>().swap(codecIndexQueue_);
    std::queue<OH_AVBuffer *>().swap(codecBufQueue_);
    TRUE_RETURN(inputConsumer_ == nullptr, "inputConsumer is null");
    while (!inputDataBufferQueue_.empty()) {
        auto audioData = inputDataBufferQueue_.front();
        inputDataBufferQueue_.pop();
        inputConsumer_->ReleaseBuffer(audioData);
    }
}

void AudioEncoderFilter::InputEncodeAudioData()
{
    AVTRANS_LOGD("enter");
    while (isEncoderRunning_.load()) {
        std::shared_ptr<Media::AVBuffer> audioData;
        uint32_t index = 0;
        OH_AVBuffer *codecMem;
        {
            std::unique_lock<std::mutex> datalock(mtxData_);
            encodeCond_.wait_for(datalock, std::chrono::milliseconds(ENCODE_WAIT_MILLISECONDS),
                [this]() {
                    return (!inputDataBufferQueue_.empty() && !codecBufQueue_.empty());
                });
            if (inputDataBufferQueue_.empty() || codecBufQueue_.empty()) {
                continue;
            }
            index = codecIndexQueue_.front();
            codecIndexQueue_.pop();
            codecMem = codecBufQueue_.front();
            codecBufQueue_.pop();
            audioData = inputDataBufferQueue_.front();
            inputDataBufferQueue_.pop();
        }
        auto ret = ProcessData(audioData, index, codecMem);
        if (ret ==  Status::ERROR_INVALID_OPERATION) {
            AVTRANS_LOGE("Encoder is not runnnig");
            return;
        } else if (ret != Status::OK) {
            AVTRANS_LOGE("Process data fail. Error type: %d.", ret);
            continue;
        }
        TRUE_RETURN(inputConsumer_ == nullptr, "inputConsumer is null");
        inputConsumer_->ReleaseBuffer(audioData);
    }
}

Status AudioEncoderFilter::ProcessData(std::shared_ptr<Media::AVBuffer> audioData, const uint32_t index,
    OH_AVBuffer *codecMem)
{
    AVTRANS_LOGD("enter");
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr || !isEncoderRunning_.load(), Status::ERROR_INVALID_OPERATION,
        "Encoder is not runnnig, isEncoderRunning_: %{public}d", isEncoderRunning_.load());
    TRUE_RETURN_V_MSG_E(codecMem == nullptr || codecMem->buffer_ == nullptr || codecMem->buffer_->memory_ == nullptr ||
        codecMem->buffer_->meta_ == nullptr, Status::ERROR_NULL_POINTER, "codecMem is invaild");
    TRUE_RETURN_V_MSG_E(audioData == nullptr || audioData->memory_ == nullptr || audioData->meta_ == nullptr,
        Status::ERROR_NULL_POINTER, "audioData is invaild");

    auto memSize = audioData->memory_->GetSize();
    codecMem->buffer_->memory_->SetSize(memSize);
    errno_t err = memcpy_s(OH_AVBuffer_GetAddr(codecMem), memSize, audioData->memory_->GetAddr(), memSize);
    TRUE_RETURN_V_MSG_E(err != EOK, Status::ERROR_INVALID_OPERATION,
        "memcpy_s err: %{public}d, memSize: %{public}d", err, memSize);
    int64_t pts = 0;
    audioData->meta_->GetData(Media::Tag::USER_FRAME_PTS, pts);
    {
        std::lock_guard<std::mutex> lock(ptsMutex_);
        ptsMap_[frameInIndex_] = pts;
        AVTRANS_LOGI("ProcessData frameInIndex_ %{public}" PRId64", pts: %{public}" PRId64, frameInIndex_, pts);
        frameInIndex_++;
        if (frameInIndex_ % FRAME_OUTINDEX_FLAG == 0) {
            frameInIndex_ = 0;
        }
    }
    codecMem->buffer_->meta_->SetData(Media::Tag::USER_FRAME_PTS, pts);
    AVTRANS_LOGI("before AudioEncoderFilter index %{public}u, pts: %{public}" PRId64, index, pts);
    codecMem->buffer_->flag_ = MediaAVCodec::AVCODEC_BUFFER_FLAG_NONE;
    auto ret = OH_AudioCodec_PushInputBuffer(audioEncoder_, index);
    TRUE_RETURN_V_MSG_E(ret != AV_ERR_OK, Status::ERROR_INVALID_OPERATION,
        "OH_AudioCodec_PushInputBuffer err: %{public}d", ret);
    return Status::OK;
}

void AudioEncoderFilter::OnEncError(int32_t errorCode)
{
    AVTRANS_LOGE("Encoder err: %{public}d", errorCode);
    isEncoderRunning_.store(false);
}

void AudioEncoderFilter::OnEncOutputFormatChanged(const OH_AVFormat *format)
{
    TRUE_RETURN(format == nullptr, "input format is nullptr");
    outputFormat_.format_ = format->format_;
}

void AudioEncoderFilter::OnEncInputBufferAvailable(uint32_t index, OH_AVBuffer *buffer)
{
    AVTRANS_LOGD("enter");
    TRUE_RETURN(buffer == nullptr, "input buffer is nullptr");
    {
        std::lock_guard<std::mutex> datalock(mtxData_);
        while (codecBufQueue_.size() > AUDIO_ENCODER_QUEUE_MAX || codecIndexQueue_.size() > AUDIO_ENCODER_QUEUE_MAX) {
            AVTRANS_LOGE("codecBufQueue_ or codecIndexQueue_ overflow");
            codecIndexQueue_.pop();
            codecBufQueue_.pop();
        }
        codecIndexQueue_.push(index);
        codecBufQueue_.push(buffer);
    }
    encodeCond_.notify_all();
}

void AudioEncoderFilter::OnEncOutputBufferAvailable(uint32_t index, OH_AVBuffer *buffer)
{
    TRUE_RETURN(audioEncoder_ == nullptr || !isEncoderRunning_.load(), "running:%{public}d", isEncoderRunning_.load());
    TRUE_RETURN(buffer == nullptr || buffer->buffer_ == nullptr || buffer->buffer_->memory_ == nullptr ||
        buffer->buffer_->memory_->GetSize() <= 0 || buffer->buffer_->meta_ == nullptr, "audioData is invaild");
    TRUE_RETURN(outputProducer_ == nullptr, "input queue is nullptr");

    Media::AVBufferConfig config;
    config.size = buffer->buffer_->memory_->GetSize();
    config.memoryType = Media::MemoryType::VIRTUAL_MEMORY;
    config.memoryFlag = Media::MemoryFlag::MEMORY_READ_WRITE;
    std::shared_ptr<Media::AVBuffer> outBuffer = nullptr;
    outputProducer_->RequestBuffer(outBuffer, config, MAX_TIME_OUT_MS);
    TRUE_RETURN(outBuffer == nullptr || outBuffer->memory_ == nullptr, "RequestBuffer failed");
    auto meta = outBuffer->meta_;
    if (meta == nullptr) {
        AVTRANS_LOGE("outBuffer->meta_ is null");
        outputProducer_->PushBuffer(outBuffer, true);
        return;
    }
    int64_t pts = 0;
    {
        std::lock_guard<std::mutex> lock(ptsMutex_);
        auto iter = ptsMap_.find(frameOutIndex_);
        if (iter != ptsMap_.end()) {
            pts = iter->second;
            ptsMap_.erase(iter);
        }
        frameOutIndex_++;
    }
    outBuffer->pts_ = pts;
    meta->SetData(Media::Tag::USER_FRAME_PTS, pts);
    AVTRANS_LOGD("index:%{public}u,outIndex_:%{public}" PRIu64",pts:%{public}" PRId64, index, frameOutIndex_, pts);
    if (frameOutIndex_ == INDEX_FLAG) {
        {
            std::lock_guard<std::mutex> lock(ptsMutex_);
            auto iter = ptsMap_.find(frameOutIndex_);
            if (iter != ptsMap_.end()) {
                pts = iter->second;
                ptsMap_.erase(iter);
            }
        }
        AVTRANS_LOGD("the fifth special process index %{public}" PRIu64", pts: %{public}" PRId64, frameOutIndex_, pts);
        meta->SetData(Media::Tag::USER_FRAME_PTS, pts);
        frameOutIndex_ = 0;
    }
    meta->SetData(Media::Tag::AUDIO_OBJECT_NUMBER, index);
    outBuffer->memory_->Write(buffer->buffer_->memory_->GetAddr(), buffer->buffer_->memory_->GetSize(), 0);
    outputProducer_->PushBuffer(outBuffer, true);
    auto ret = OH_AudioCodec_FreeOutputBuffer(audioEncoder_, index);
    TRUE_RETURN(ret != AV_ERR_OK, "OH_AudioCodec_FreeOutputBuffer err: %{public}d", ret);
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
