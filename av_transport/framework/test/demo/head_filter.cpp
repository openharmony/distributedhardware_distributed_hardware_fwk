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

#include "head_filter.h"

#include <algorithm>

#include "av_trans_log.h"
#include "filter_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "HeadFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
static AutoRegisterFilter<HeadFilter> g_registerAudioEncoderFilter("builtin.recorder.test1",
    FilterType::FILTERTYPE_MUXER,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<HeadFilter>(name, FilterType::FILTERTYPE_MUXER);
    });

class HeadFilterLinkCallback : public FilterLinkCallback {
public:
    explicit HeadFilterLinkCallback(std::shared_ptr<HeadFilter> filter)
        : headFilter_(std::move(filter)) {}
    ~HeadFilterLinkCallback() = default;

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
    std::weak_ptr<HeadFilter> headFilter_ {};
};

HeadFilter::HeadFilter(std::string name, FilterType type, bool isAsyncMode)
    : Filter(name, type)
{
}

HeadFilter::~HeadFilter()
{
    nextFiltersMap_.clear();
}

void HeadFilter::Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback)
{
    AVTRANS_LOGI("HeadFilter::Init");
    receiver_ = receiver;
    callback_ = callback;
    AVTRANS_LOGI("HeadFilter::Init Done");
}

Status HeadFilter::DoInitAfterLink()
{
    AVTRANS_LOGI("HeadFilter::DoInitAfterLink");
    return Status::OK;
}

Status HeadFilter::DoPrepare()
{
    // 1. 本Filter的准备
    AVTRANS_LOGI("HeadFilter::DoPrepare");

    // 2. 回调申请下一级filter
    if (callback_ == nullptr) {
        AVTRANS_LOGE("filter callback is null");
        return Status::ERROR_NULL_POINTER;
    }
    callback_->OnCallback(shared_from_this(), FilterCallBackCommand::NEXT_FILTER_NEEDED,
        StreamType::STREAMTYPE_ENCODED_AUDIO);
    AVTRANS_LOGI("HeadFilter::DoPrepare done");
    return Status::OK;
}

Status HeadFilter::DoStart()
{
    AVTRANS_LOGI("HeadFilter::DoStart");
    return Status::OK;
}

Status HeadFilter::DoPause()
{
    AVTRANS_LOGI("HeadFilter::DoPause");
    return Status::OK;
}

Status HeadFilter::DoPauseDragging()
{
    AVTRANS_LOGI("HeadFilter::DoPauseDragging");
    return Status::OK;
}

Status HeadFilter::DoResume()
{
    AVTRANS_LOGI("HeadFilter::DoResume");
    return Status::OK;
}

Status HeadFilter::DoResumeDragging()
{
    AVTRANS_LOGI("HeadFilter::DoResumeDragging");
    return Status::OK;
}

Status HeadFilter::DoStop()
{
    AVTRANS_LOGI("HeadFilter::DoStop");
    return Status::OK;
}

Status HeadFilter::DoFlush()
{
    AVTRANS_LOGI("HeadFilter::DoFlush");
    return Status::OK;
}

Status HeadFilter::DoRelease()
{
    AVTRANS_LOGI("HeadFilter::DoRelease");
    return Status::OK;
}

Status HeadFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    AVTRANS_LOGI("HeadFilter::DoProcessInputBuffer");
    return Status::OK;
}

Status HeadFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx, uint32_t idx, int64_t renderTimee)
{
    AVTRANS_LOGI("HeadFilter::DoProcessOutputBuffer");
    return Status::OK;
}

void HeadFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void HeadFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status HeadFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    AVTRANS_LOGI("cur: HeadFilter, link next filter..");
    // 1. 插入下级filter维护map
    nextFilter_ = nextFilter;
    nextFiltersMap_[outType].push_back(nextFilter_);
    // 2. 下级节点的Link初始化
    auto filterLinkCallback = std::make_shared<HeadFilterLinkCallback>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, configureParam_, filterLinkCallback);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Onlinked failed, status: %{public}d.", ret);
        return ret;
    }
    return Status::OK;
}

Status HeadFilter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status HeadFilter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    AVTRANS_LOGI("cur: HeadFilter, unlink next filter..");
    return Status::OK;
}

Status HeadFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    AVTRANS_LOGI("cur: HeadFilter, OnLinked");
    return Status::OK;
};

Status HeadFilter::OnUpdated(StreamType, const std::shared_ptr<Media::Meta>&,
    const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status HeadFilter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    AVTRANS_LOGI("cur: HeadFilter, OnUnLinked.");
    return Status::OK;
}

void HeadFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue, std::shared_ptr<Media::Meta>& meta)
{
    // link bufferQueue
    AVTRANS_LOGI("cur: HeadFilter, OnLinkedResult");
}

void HeadFilter::OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta)
{
}

void HeadFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
