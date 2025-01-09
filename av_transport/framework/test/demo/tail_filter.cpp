/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "tail_filter.h"

#include <algorithm>

#include "av_trans_log.h"
#include "filter_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "TailFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
static AutoRegisterFilter<TailFilter> g_registerAudioEncoderFilter("builtin.recorder.tailfilter",
    FilterType::FILTERTYPE_DEMUXER,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<TailFilter>(name, FilterType::FILTERTYPE_DEMUXER);
    });

class TailFilterLinkCallback : public FilterLinkCallback {
public:
    explicit TailFilterLinkCallback(std::shared_ptr<TailFilter> filter)
        : headFilter_(std::move(filter)) {}
    ~TailFilterLinkCallback() = default;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer> &queue, std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = headFilter_.lock()) {
            filter->OnLinkedResult(queue, meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

    void OnUnlinkedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = headFilter_.lock()) {
            filter->OnUnlinkedResult(meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

    void OnUpdatedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = headFilter_.lock()) {
            filter->OnUpdatedResult(meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

private:
    std::weak_ptr<TailFilter> headFilter_ {};
};

TailFilter::TailFilter(std::string name, FilterType type, bool isAsyncMode)
    : Filter(name, type)
{
}

TailFilter::~TailFilter()
{
    nextFiltersMap_.clear();
}

void TailFilter::Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback)
{
    receiver_ = receiver;
    callback_ = callback;
}

Status TailFilter::DoInitAfterLink()
{
    AVTRANS_LOGI("TailFilter::DoInitAfterLink");
    return Status::OK;
}

Status TailFilter::DoPrepare()
{
    AVTRANS_LOGI("cur: TailFilter, notify linkedResult.");
    sptr<Media::AVBufferQueueProducer> producer = nullptr;
    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    if (onLinkedResultCallback_ != nullptr) {
        onLinkedResultCallback_->OnLinkedResult(producer, meta);
    }
    return Status::OK;
}

Status TailFilter::DoStart()
{
    AVTRANS_LOGI("TailFilter::DoStart");
    return Status::OK;
}

Status TailFilter::DoPause()
{
    AVTRANS_LOGI("TailFilter::DoPause");
    return Status::OK;
}

Status TailFilter::DoPauseDragging()
{
    AVTRANS_LOGI("TailFilter::DoPauseDragging");
    return Status::OK;
}

Status TailFilter::DoResume()
{
    AVTRANS_LOGI("TailFilter::DoResume");
    return Status::OK;
}

Status TailFilter::DoResumeDragging()
{
    AVTRANS_LOGI("TailFilter::DoResumeDragging");
    return Status::OK;
}

Status TailFilter::DoStop()
{
    AVTRANS_LOGI("TailFilter::DoStop");
    return Status::OK;
}

Status TailFilter::DoFlush()
{
    AVTRANS_LOGI("TailFilter::DoFlush");
    return Status::OK;
}

Status TailFilter::DoRelease()
{
    AVTRANS_LOGI("TailFilter::DoRelease");
    return Status::OK;
}

Status TailFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    AVTRANS_LOGI("TailFilter::DoProcessInputBuffer");
    return Status::OK;
}

Status TailFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx, uint32_t idx, int64_t renderTimee)
{
    AVTRANS_LOGI("TailFilter::DoProcessOutputBuffer");
    return Status::OK;
}

void TailFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void TailFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status TailFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    AVTRANS_LOGI("cur: TailFilter, link next filter..");
    // 1. 插入下级filter维护map
    nextFilter_ = nextFilter;
    nextFiltersMap_[outType].push_back(nextFilter_);
    // 2. 下级节点的Link初始化
    auto filterLinkCallback = std::make_shared<TailFilterLinkCallback>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, configureParam_, filterLinkCallback);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Onlinked failed, status: %{public}d.", ret);
        return ret;
    }
    return Status::OK;
}

Status TailFilter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status TailFilter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    AVTRANS_LOGI("cur: TailFilter, unlink next filter..");
    return Status::OK;
}

Status TailFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    AVTRANS_LOGI("cur: TailFilter, OnLinked");
    onLinkedResultCallback_ = callback;
    return Status::OK;
};

Status TailFilter::OnUpdated(StreamType, const std::shared_ptr<Media::Meta>&,
    const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status TailFilter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    AVTRANS_LOGI("cur: TailFilter, OnUnLinked.");
    return Status::OK;
}

void TailFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue, std::shared_ptr<Media::Meta>& meta)
{
    // link bufferQueue
}

void TailFilter::OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta)
{
}

void TailFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
