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

#include "daudio_output_filter.h"

#include <algorithm>

#include "av_trans_log.h"
#include "filter_factory.h"

#include "cJSON.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "av_trans_constants.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DAudioOutputFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string OUTPUT_BUFFER_QUEUE_NAME = "AVTransAudioOutputBufferQueue";

static AutoRegisterFilter<DAudioOutputFilter> g_registerAudioEncoderFilter("builtin.daudio.output",
    FilterType::FILTERTYPE_SSINK,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<DAudioOutputFilter>(name, FilterType::FILTERTYPE_SSINK);
    });

class DAudioOutputFilterLinkCallback : public FilterLinkCallback {
public:
    explicit DAudioOutputFilterLinkCallback(std::shared_ptr<DAudioOutputFilter> filter)
        : outFilter_(std::move(filter)) {}
    ~DAudioOutputFilterLinkCallback() = default;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer> &queue, std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = outFilter_.lock()) {
            filter->OnLinkedResult(queue, meta);
        } else {
            AVTRANS_LOGI("invalid dAudioOutputFilter");
        }
    }

    void OnUnlinkedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = outFilter_.lock()) {
            filter->OnUnlinkedResult(meta);
        } else {
            AVTRANS_LOGI("invalid dAudioOutputFilter");
        }
    }

    void OnUpdatedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = outFilter_.lock()) {
            filter->OnUpdatedResult(meta);
        } else {
            AVTRANS_LOGI("invalid dAudioOutputFilter");
        }
    }

private:
    std::weak_ptr<DAudioOutputFilter> outFilter_ {};
};

class AVBufferAvailableListener : public Media::IConsumerListener {
public:
    explicit AVBufferAvailableListener(std::weak_ptr<DAudioOutputFilter> outputFilter)
    {
        outputFilter_ = outputFilter;
    }

    void OnBufferAvailable() override
    {
        auto outputFilter = outputFilter_.lock();
        if (outputFilter != nullptr) {
            outputFilter->ProcessInputBuffer();
        }
    }

private:
    std::weak_ptr<DAudioOutputFilter> outputFilter_;
};

DAudioOutputFilter::DAudioOutputFilter(std::string name, FilterType type)
    : Filter(name, type)
{
}

DAudioOutputFilter::~DAudioOutputFilter()
{
    nextFiltersMap_.clear();
}

void DAudioOutputFilter::Init(const std::shared_ptr<EventReceiver>& receiver,
    const std::shared_ptr<FilterCallback>& callback)
{
    eventReceiver_ = receiver;
    filterCallback_ = callback;
}

Status DAudioOutputFilter::DoInitAfterLink()
{
    return Status::OK;
}

void DAudioOutputFilter::PrepareInputBuffer()
{
    AVTRANS_LOGI("Preparing input buffer.");
    int32_t outputBufNum = DEFAULT_BUFFER_NUM;
    Media::MemoryType memoryType = Media::MemoryType::VIRTUAL_MEMORY;
    if (outputBufQue_ == nullptr) {
        outputBufQue_ = Media::AVBufferQueue::Create(outputBufNum, memoryType, OUTPUT_BUFFER_QUEUE_NAME);
    }
    if (outputBufQue_ == nullptr) {
        AVTRANS_LOGE("Create buffer queue failed.");
        return;
    }
    inputBufQueProducer_ = outputBufQue_->GetProducer();
    TRUE_RETURN((inputBufQueProducer_ == nullptr), "Get producer failed");

    inputBufQueConsumer_ = outputBufQue_->GetConsumer();
    TRUE_RETURN((inputBufQueConsumer_ == nullptr), "Get consumer failed");

    sptr<Media::IConsumerListener> listener(new AVBufferAvailableListener(shared_from_this()));
    inputBufQueConsumer_->SetBufferAvailableListener(listener);
}

Status DAudioOutputFilter::DoPrepare()
{
    PrepareInputBuffer();
    int32_t inputBufNum = DEFAULT_BUFFER_NUM;
    sptr<Media::AVBufferQueueProducer> producer = nullptr;
    Media::MemoryType memoryType = Media::MemoryType::UNKNOWN_MEMORY;
    if (outputBufQue_ == nullptr) {
        outputBufQue_ = Media::AVBufferQueue::Create(inputBufNum, memoryType, OUTPUT_BUFFER_QUEUE_NAME);
    }
    if (outputBufQue_ == nullptr) {
        AVTRANS_LOGE("Create buffer queue failed.");
        return Status::ERROR_NULL_POINTER;
    }
    producer = outputBufQue_->GetProducer();
    TRUE_RETURN_V_MSG_E((producer == nullptr), Status::ERROR_NULL_POINTER, "Get producer failed");
    sptr<Media::AVBufferQueueConsumer> consumer = outputBufQue_->GetConsumer();
    sptr<Media::IConsumerListener> listener(new AVBufferAvailableListener(shared_from_this()));
    consumer->SetBufferAvailableListener(listener);

    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    if (onLinkedResultCallback_ != nullptr) {
        onLinkedResultCallback_->OnLinkedResult(producer, meta);
    }
    return Status::OK;
}

Status DAudioOutputFilter::DoStart()
{
    AVTRANS_LOGI("Do Start");
    return Status::OK;
}

Status DAudioOutputFilter::DoPause()
{
    return Status::OK;
}

Status DAudioOutputFilter::DoPauseDragging()
{
    return Status::OK;
}

Status DAudioOutputFilter::DoResume()
{
    return Status::OK;
}

Status DAudioOutputFilter::DoResumeDragging()
{
    return Status::OK;
}

Status DAudioOutputFilter::DoStop()
{
    return Status::OK;
}

Status DAudioOutputFilter::DoFlush()
{
    return Status::OK;
}

Status DAudioOutputFilter::DoRelease()
{
    return Status::OK;
}

Status DAudioOutputFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    (void)recvArg;
    (void)dropFrame;
    std::shared_ptr<Media::AVBuffer> filledBuffer = nullptr;
    Media::Status ret = inputBufQueConsumer_->AcquireBuffer(filledBuffer);
    if (ret != Media::Status::OK) {
        AVTRANS_LOGE("Acquire buffer err: %{public}d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    ProcessAndSendBuffer(filledBuffer);
    inputBufQueConsumer_->ReleaseBuffer(filledBuffer);
    return Status::OK;
}

Status DAudioOutputFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx,
    uint32_t idx, int64_t renderTimee)
{
    return Status::OK;
}

Status DAudioOutputFilter::ProcessAndSendBuffer(const std::shared_ptr<Media::AVBuffer> buffer)
{
    if (buffer == nullptr) {
        AVTRANS_LOGE("ProcessAndSendBuffer buffer is nullptr");
        return Status::ERROR_INVALID_OPERATION;
    }
    TRUE_RETURN_V_MSG_E(eventReceiver_ == nullptr, Status::ERROR_NULL_POINTER, "receiver_ is nullptr");
    Event event;
    event.type = EventType::EVENT_BUFFER_PROGRESS;
    event.param = buffer;
    eventReceiver_->OnEvent(event);
    return Status::OK;
}

void DAudioOutputFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void DAudioOutputFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status DAudioOutputFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    nextFilter_ = nextFilter;
    nextFiltersMap_[outType].push_back(nextFilter_);
    auto filterLinkCallback = std::make_shared<DAudioOutputFilterLinkCallback>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, meta_, filterLinkCallback);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Onlinked failed, status: %{public}d.", ret);
        return ret;
    }
    return Status::OK;
}

Status DAudioOutputFilter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status DAudioOutputFilter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    AVTRANS_LOGI("cur: DAudioOutputFilter, unlink next filter..");
    return Status::OK;
}

Status DAudioOutputFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    AVTRANS_LOGI("cur: DAudioOutputFilter, OnLinked");
    onLinkedResultCallback_ = callback;
    SetParameter(meta);
    return Status::OK;
};

Status DAudioOutputFilter::OnUpdated(StreamType, const std::shared_ptr<Media::Meta>&,
    const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status DAudioOutputFilter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    AVTRANS_LOGI("cur: DAudioOutputFilter, OnUnLinked.");
    return Status::OK;
}

void DAudioOutputFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue,
    std::shared_ptr<Media::Meta>& meta)
{
    outputBufQueProducer_ = queue;
}

void DAudioOutputFilter::OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta)
{
}

void DAudioOutputFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
