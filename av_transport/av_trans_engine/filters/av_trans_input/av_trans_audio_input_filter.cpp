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

#include "av_trans_audio_input_filter.h"

#include <algorithm>
#include <memory>

#include "av_trans_log.h"
#include "filter_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AudioInputFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
namespace {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
constexpr int32_t MAX_TIME_OUT_MS = 1;
const std::string INPUT_BUFFER_QUEUE_NAME = "AVTransAudioInputBufferQueue";
}

static AutoRegisterFilter<AVTransAudioInputFilter> g_registerAudioEncoderFilter("builtin.avtrans.audio.input",
    FilterType::FILTERTYPE_SOURCE,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<AVTransAudioInputFilter>(name, FilterType::FILTERTYPE_SOURCE);
    });

class HeadFilterLinkCallback : public FilterLinkCallback {
public:
    explicit HeadFilterLinkCallback(std::shared_ptr<AVTransAudioInputFilter> filter)
        : filter_(std::move(filter)) {}
    ~HeadFilterLinkCallback() = default;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer> &queue, std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = filter_.lock()) {
            filter->OnLinkedResult(queue, meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

    void OnUnlinkedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = filter_.lock()) {
            filter->OnUnlinkedResult(meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

    void OnUpdatedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = filter_.lock()) {
            filter->OnUpdatedResult(meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

private:
    std::weak_ptr<AVTransAudioInputFilter> filter_ {};
};

class InputBufAvailableListener : public Media::IConsumerListener {
public:
    explicit InputBufAvailableListener(const std::weak_ptr<AVTransAudioInputFilter> inputFilter)
    {
        inputFilter_ = inputFilter;
    }

    void OnBufferAvailable() override
    {
        auto inputFilter = inputFilter_.lock();
        if (inputFilter != nullptr) {
            inputFilter->ProcessInputBuffer();
        }
    }

private:
    std::weak_ptr<AVTransAudioInputFilter> inputFilter_;
};

AVTransAudioInputFilter::AVTransAudioInputFilter(std::string name, FilterType type, bool isAsyncMode)
    : Filter(name, type)
{
}

AVTransAudioInputFilter::~AVTransAudioInputFilter()
{
    nextFiltersMap_.clear();
}

void AVTransAudioInputFilter::Init(const std::shared_ptr<EventReceiver>& receiver,
    const std::shared_ptr<FilterCallback>& callback)
{
    AVTRANS_LOGI("AVTransAudioInputFilter::Init");
    receiver_ = receiver;
    callback_ = callback;
    AVTRANS_LOGI("AVTransAudioInputFilter::Init Done");
}

Status AVTransAudioInputFilter::DoInitAfterLink()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoInitAfterLink");
    return Status::OK;
}

void AVTransAudioInputFilter::PrepareInputBuffer()
{
    AVTRANS_LOGI("Preparing input buffer.");
    int32_t inputBufNum = DEFAULT_BUFFER_NUM;
    Media::MemoryType memoryType = Media::MemoryType::VIRTUAL_MEMORY;
    if (inputBufQue_ == nullptr) {
        inputBufQue_ = Media::AVBufferQueue::Create(inputBufNum, memoryType, INPUT_BUFFER_QUEUE_NAME);
    }
    if (inputBufQue_ == nullptr) {
        AVTRANS_LOGE("Create buffer queue failed.");
        return;
    }
    inputBufQueProducer_ = inputBufQue_->GetProducer();
    TRUE_RETURN((inputBufQueProducer_ == nullptr), "Get producer failed");

    inputBufQueConsumer_ = inputBufQue_->GetConsumer();
    TRUE_RETURN((inputBufQueConsumer_ == nullptr), "Get consumer failed");

    sptr<Media::IConsumerListener> listener(new InputBufAvailableListener(shared_from_this()));
    inputBufQueConsumer_->SetBufferAvailableListener(listener);
}

Status AVTransAudioInputFilter::DoPrepare()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoPrepare");
    PrepareInputBuffer();
    frameNumber_.store(0);
    if (callback_ == nullptr) {
        AVTRANS_LOGE("filter callback is null");
        return Status::ERROR_NULL_POINTER;
    }
    TRUE_RETURN_V_MSG_E(meta_ == nullptr, Status::ERROR_NULL_POINTER, "meta_ is nullptr");
    int32_t mimeType = 0;
    auto filterType = StreamType::STREAMTYPE_ENCODED_AUDIO;
    meta_->GetData(Media::Tag::MIME_TYPE, mimeType);
    if (static_cast<AudioCodecType>(mimeType) == AudioCodecType::AUDIO_CODEC_AAC) {
        filterType = StreamType::STREAMTYPE_RAW_AUDIO;
    }
    AVTRANS_LOGD("mimeType: %{public}d.", mimeType);
    callback_->OnCallback(shared_from_this(), FilterCallBackCommand::NEXT_FILTER_NEEDED, filterType);
    AVTRANS_LOGI("AVTransAudioInputFilter::DoPrepare done");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoStart()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoStart");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoPause()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoPause");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoPauseDragging()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoPauseDragging");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoResume()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoResume");
    frameNumber_.store(0);
    return Status::OK;
}

Status AVTransAudioInputFilter::DoResumeDragging()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoResumeDragging");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoStop()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoStop");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoFlush()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoFlush");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoRelease()
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoRelease");
    return Status::OK;
}

Status AVTransAudioInputFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    AVTRANS_LOGD("DoProcessInputBuffer");
    (void)recvArg;
    (void)dropFrame;
    std::shared_ptr<Media::AVBuffer> filledBuffer = nullptr;
    TRUE_RETURN_V_MSG_E((inputBufQueConsumer_ == nullptr), Status::ERROR_WRONG_STATE, "InputBufQueConsumer is null");
    Media::Status ret = inputBufQueConsumer_->AcquireBuffer(filledBuffer);
    if (ret != Media::Status::OK) {
        AVTRANS_LOGE("Acquire buffer err.");
        return Status::ERROR_INVALID_OPERATION;
    }
    if (curState_ != FilterState::RUNNING) {
        inputBufQueConsumer_->ReleaseBuffer(filledBuffer);
        AVTRANS_LOGE("Current status ia not running.");
        return Status::ERROR_WRONG_STATE;
    }
    ProcessAndSendBuffer(filledBuffer);
    inputBufQueConsumer_->ReleaseBuffer(filledBuffer);
    return Status::OK;
}

Status AVTransAudioInputFilter::ProcessAndSendBuffer(const std::shared_ptr<Media::AVBuffer> buffer)
{
    if (buffer == nullptr || buffer->memory_ == nullptr) {
        AVTRANS_LOGE("AVBuffer is null");
        return Status::ERROR_NULL_POINTER;
    }

    TRUE_RETURN_V_MSG_E((outputBufQueProducer_ == nullptr), Status::ERROR_NULL_POINTER, "Producer is null");
    Media::AVBufferConfig config(buffer->GetConfig());
    AVTRANS_LOGD("outPut config, size: %{public}u, capacity: %{public}u, memtype: %{public}hhu",
        config.size, config.capacity, config.memoryType);
    std::shared_ptr<Media::AVBuffer> outBuffer = nullptr;
    outputBufQueProducer_->RequestBuffer(outBuffer, config, MAX_TIME_OUT_MS);
    TRUE_RETURN_V_MSG_E((outBuffer == nullptr || outBuffer->memory_ == nullptr), Status::ERROR_NULL_POINTER,
        "OutBuffer or memory is null");
    auto meta = outBuffer->meta_;
    if (meta == nullptr) {
        AVTRANS_LOGE("Meta of AVBuffer is null");
        outputBufQueProducer_->PushBuffer(outBuffer, true);
        return Status::ERROR_NULL_POINTER;
    }
    ++frameNumber_;
    outBuffer->pts_ = buffer->pts_;
    meta->SetData(Media::Tag::USER_FRAME_PTS, outBuffer->pts_);
    outBuffer->memory_->Write(buffer->memory_->GetAddr(), buffer->memory_->GetSize(), 0);
    outputBufQueProducer_->PushBuffer(outBuffer, true);
    return Status::OK;
}

Status AVTransAudioInputFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx,
    uint32_t idx, int64_t renderTimee)
{
    AVTRANS_LOGI("AVTransAudioInputFilter::DoProcessOutputBuffer");
    return Status::OK;
}

void AVTransAudioInputFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void AVTransAudioInputFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status AVTransAudioInputFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    AVTRANS_LOGI("cur: AVTransAudioInputFilter, link next filter..");
    nextFilter_ = nextFilter;
    nextFiltersMap_[outType].push_back(nextFilter_);
    auto filterLinkCallback = std::make_shared<HeadFilterLinkCallback>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, meta_, filterLinkCallback);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Onlinked failed, status: %{public}d.", ret);
        return ret;
    }
    return Status::OK;
}

Status AVTransAudioInputFilter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status AVTransAudioInputFilter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    AVTRANS_LOGI("cur: AVTransAudioInputFilter, unlink next filter..");
    return Status::OK;
}

Status AVTransAudioInputFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    AVTRANS_LOGI("cur: AVTransAudioInputFilter, OnLinked");
    return Status::OK;
};

Status AVTransAudioInputFilter::OnUpdated(StreamType, const std::shared_ptr<Media::Meta>&,
    const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status AVTransAudioInputFilter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    AVTRANS_LOGI("cur: AVTransAudioInputFilter, OnUnLinked.");
    return Status::OK;
}

void AVTransAudioInputFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue,
    std::shared_ptr<Media::Meta>& meta)
{
    AVTRANS_LOGI("cur: AVTransAudioInputFilter, OnLinkedResult");
    outputBufQueProducer_ = queue;
}

void AVTransAudioInputFilter::OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta)
{
    (void)meta;
}

void AVTransAudioInputFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
    (void)meta;
}

sptr<Media::AVBufferQueueProducer> AVTransAudioInputFilter::GetInputBufQueProducer()
{
    return inputBufQueProducer_;
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
