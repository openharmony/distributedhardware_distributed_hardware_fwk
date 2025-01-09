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

#include "mid_filter.h"

#include <algorithm>

#include "av_trans_log.h"
#include "filter_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "MidFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
static AutoRegisterFilter<MidFilter> g_registerAudioEncoderFilter("builtin.recorder.midfilter",
    FilterType::FILTERTYPE_VIDEODEC,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<MidFilter>(name, FilterType::FILTERTYPE_VIDEODEC);
    });

class MidFilterLinkCallback : public FilterLinkCallback {
public:
    explicit MidFilterLinkCallback(std::shared_ptr<MidFilter> filter)
        : headFilter_(std::move(filter)) {}
    ~MidFilterLinkCallback() = default;

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
    std::weak_ptr<MidFilter> headFilter_ {};
};

MidFilter::MidFilter(std::string name, FilterType type, bool isAsyncMode)
    : Filter(name, type)
{
}

MidFilter::~MidFilter()
{
    nextFiltersMap_.clear();
}

void MidFilter::Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback)
{
    receiver_ = receiver;
    callback_ = callback;
}

Status MidFilter::DoInitAfterLink()
{
    AVTRANS_LOGI("MidFilter::DoInitAfterLink");
    return Status::OK;
}

Status MidFilter::DoPrepare()
{
    // 1. 本Filter的准备
    AVTRANS_LOGI("MidFilter::DoPrepare");

    // 2. 回调申请下一级filter
    if (callback_ == nullptr) {
        AVTRANS_LOGE("filter callback is null");
        return Status::ERROR_NULL_POINTER;
    }
    callback_->OnCallback(shared_from_this(), FilterCallBackCommand::NEXT_FILTER_NEEDED,
        StreamType::STREAMTYPE_RAW_AUDIO);
    AVTRANS_LOGI("MidFilter::DoPrepare done");
    return Status::OK;
}

Status MidFilter::DoStart()
{
    AVTRANS_LOGI("MidFilter::DoStart");
    return Status::OK;
}

Status MidFilter::DoPause()
{
    AVTRANS_LOGI("MidFilter::DoPause");
    return Status::OK;
}

Status MidFilter::DoPauseDragging()
{
    AVTRANS_LOGI("MidFilter::DoPauseDragging");
    return Status::OK;
}

Status MidFilter::DoResume()
{
    AVTRANS_LOGI("MidFilter::DoResume");
    return Status::OK;
}

Status MidFilter::DoResumeDragging()
{
    AVTRANS_LOGI("MidFilter::DoResumeDragging");
    return Status::OK;
}

Status MidFilter::DoStop()
{
    AVTRANS_LOGI("MidFilter::DoStop");
    return Status::OK;
}

Status MidFilter::DoFlush()
{
    AVTRANS_LOGI("MidFilter::DoFlush");
    return Status::OK;
}

Status MidFilter::DoRelease()
{
    AVTRANS_LOGI("MidFilter::DoRelease");
    return Status::OK;
}

Status MidFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    AVTRANS_LOGI("MidFilter::DoProcessInputBuffer");
    return Status::OK;
}

Status MidFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx, uint32_t idx, int64_t renderTimee)
{
    AVTRANS_LOGI("MidFilter::DoProcessOutputBuffer");
    return Status::OK;
}

void MidFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void MidFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status MidFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    AVTRANS_LOGI("cur: MidFilter, link next filter..");
    // 1. 插入下级filter维护map
    nextFilter_ = nextFilter;
    nextFiltersMap_[outType].push_back(nextFilter_);
    // 2. 下级节点的Link初始化
    auto filterLinkCallback = std::make_shared<MidFilterLinkCallback>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, configureParam_, filterLinkCallback);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Onlinked failed, status: %{public}d.", ret);
        return ret;
    }
    return Status::OK;
}

Status MidFilter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status MidFilter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    AVTRANS_LOGI("cur: MidFilter, unlink next filter..");
    return Status::OK;
}

Status MidFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    AVTRANS_LOGI("cur: MidFilter, OnLinked");
    onLinkedResultCallback_ = callback;
    return Status::OK;
};

Status MidFilter::OnUpdated(StreamType, const std::shared_ptr<Media::Meta>&,
    const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status MidFilter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    AVTRANS_LOGI("cur: MidFilter, OnUnLinked.");
    return Status::OK;
}

void MidFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue, std::shared_ptr<Media::Meta>& meta)
{
    // link bufferQueue
    AVTRANS_LOGI("cur: MidFilter, OnLinkedResult.");
    sptr<Media::AVBufferQueueProducer> producer = nullptr;
    std::shared_ptr<Media::Meta> midMeta = std::make_shared<Media::Meta>();
    if (onLinkedResultCallback_ != nullptr) {
        onLinkedResultCallback_->OnLinkedResult(producer, midMeta);
    }
}

void MidFilter::OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta)
{
}

void MidFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
