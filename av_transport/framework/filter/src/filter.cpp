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

#include "filter.h"

#include <algorithm>

#include "osal/utils/util.h"

#include "av_trans_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "Filter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
Filter::Filter(std::string name, FilterType type, bool isAsyncMode)
    : name_(std::move(name)), filterType_(type), curState_(FilterState::ERROR), isAsyncMode_(isAsyncMode)
{
}

Filter::~Filter()
{
    nextFiltersMap_.clear();
}

void Filter::Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback)
{
    receiver_ = receiver;
    callback_ = callback;
}

void Filter::LinkPipeLine(const std::string& groupId)
{
    groupId_ = groupId;
    if (isAsyncMode_) {
        Media::TaskType taskType;
        switch (filterType_) {
            case FilterType::FILTERTYPE_VENC:
            case FilterType::FILTERTYPE_VDEC:
            case FilterType::VIDEO_CAPTURE:
                taskType = Media::TaskType::SINGLETON;
                break;
            case FilterType::FILTERTYPE_ASINK:
            case FilterType::AUDIO_CAPTURE:
                taskType = Media::TaskType::AUDIO;
                break;
            default:
                taskType = Media::TaskType::SINGLETON;
                break;
        }
        filterTask_ = std::make_unique<Media::Task>(name_, groupId_, taskType, Media::TaskPriority::HIGH, false);
        filterTask_->SubmitJobOnce([this] {
           DoInitAfterLink();
           ChangeState(FilterState::INITIALIZED);
        });
    } else {
        DoInitAfterLink();
        ChangeState(FilterState::INITIALIZED);
    }
}

Status Filter::Prepare()
{
    AVTRANS_LOGD("Prepare %{public}s, pState:%{public}d", name_.c_str(), curState_);
    if (filterTask_) {
        filterTask_->SubmitJobOnce([this] {
            PrepareDone();
        });
    } else {
        return PrepareDone();
    }
    return Status::OK;
}

Status Filter::PrepareDone()
{
    AVTRANS_LOGI("Prepare in %{public}s", name_.c_str());
    // next filters maybe added in DoPrepare, so we must DoPrepare first
    Status ret = DoPrepare();
    SetErrCode(ret);
    if (ret != Status::OK) {
        return ret;
    }
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->Prepare();
            }
        }
    }
    ChangeState(FilterState::READY);
    return ret;
}

Status Filter::Start()
{
    AVTRANS_LOGD("Start %{public}s, pState:%{public}d", name_.c_str(), curState_);
    if (filterTask_) {
        filterTask_->SubmitJobOnce([this] {
            StartDone();
            filterTask_->Start();
        });
        for (auto iter : nextFiltersMap_) {
            for (auto filter : iter.second) {
                filter->Start();
            }
        }
        return Status::OK;
    }

    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->Start();
            }
        }
    }
    return StartDone();
}

Status Filter::StartDone()
{
    AVTRANS_LOGI("Start in %{public}s", name_.c_str());
    Status ret = DoStart();
    SetErrCode(ret);
    ChangeState(ret == Status::OK ? FilterState::RUNNING : FilterState::ERROR);
    return ret;
}

Status Filter::Pause()
{
    AVTRANS_LOGD("Pause %{public}s, pState:%{public}d", name_.c_str(), curState_);
    // In offload case, we need pause to interrupt audio_sink_plugin write function,  so do not use filterTask_
    auto ret = PauseDone();
    if (filterTask_) {
        filterTask_->Pause();
    }
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->Pause();
            }
        }
    }
    return ret;
}

Status Filter::PauseDragging()
{
    AVTRANS_LOGD("PauseDragging %{public}s, pState:%{public}d", name_.c_str(), curState_);
    auto ret = DoPauseDragging();
    if (filterTask_) {
        filterTask_->Pause();
    }
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->PauseDragging();
            }
        }
    }
    return ret;
}

Status Filter::PauseDone()
{
    AVTRANS_LOGI("Pause in %{public}s", name_.c_str());
    Status ret = DoPause();
    SetErrCode(ret);
    ChangeState(ret == Status::OK ? FilterState::PAUSED : FilterState::ERROR);
    return ret;
}

Status Filter::Resume()
{
    AVTRANS_LOGD("Resume %{public}s, pState:%{public}d", name_.c_str(), curState_);
    if (filterTask_) {
        filterTask_->SubmitJobOnce([this]() {
            ResumeDone();
            filterTask_->Start();
        });
        for (auto iter : nextFiltersMap_) {
            for (auto filter : iter.second) {
                filter->Resume();
            }
        }
        return Status::OK;
    }

    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->Resume();
            }
        }
    }
    return ResumeDone();
}

Status Filter::ResumeDone()
{
    AVTRANS_LOGI("Resume in %{public}s", name_.c_str());
    Status ret = DoResume();
    SetErrCode(ret);
    ChangeState(ret == Status::OK ? FilterState::RUNNING : FilterState::ERROR);
    return ret;
}

Status Filter::ResumeDragging()
{
    AVTRANS_LOGD("ResumeDragging %{public}s, pState:%{public}d", name_.c_str(), curState_);
    if (filterTask_) {
        filterTask_->SubmitJobOnce([this]() {
            DoResumeDragging();
            filterTask_->Start();
        });
        for (auto iter : nextFiltersMap_) {
            for (auto filter : iter.second) {
                filter->ResumeDragging();
            }
        }
        return Status::OK;
    }

    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->ResumeDragging();
            }
        }
    }
    return DoResumeDragging();
}

Status Filter::Stop()
{
    AVTRANS_LOGD("Stop %{public}s, pState:%{public}d", name_.c_str(), curState_);
    // In offload case, we need stop to interrupt audio_sink_plugin write function,  so do not use filterTask_
    auto ret = StopDone();
    if (filterTask_) {
        filterTask_->Stop();
    }
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->Stop();
            }
        }
    }
    return ret;
}

Status Filter::StopDone()
{
    AVTRANS_LOGI("Stop in %{public}s", name_.c_str());
    Status ret = DoStop();
    SetErrCode(ret);
    ChangeState(ret == Status::OK ? FilterState::STOPPED : FilterState::ERROR);
    return ret;
}

Status Filter::Flush()
{
    AVTRANS_LOGD("Flush %{public}s, pState:%{public}d", name_.c_str(), curState_);
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->Flush();
            }
        }
    }
    jobIdxBase_ = jobIdx_;
    return DoFlush();
}

Status Filter::Release()
{
    AVTRANS_LOGD("Release %{public}s, pState:%{public}d", name_.c_str(), curState_);
    if (filterTask_) {
        filterTask_->SubmitJobOnce([this]() {
            ReleaseDone();
        });

        for (auto iter : nextFiltersMap_) {
            for (auto filter : iter.second) {
                filter->Release();
            }
        }
        return Status::OK;
    }

    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->Release();
            }
        }
    }
    return ReleaseDone();
}

Status Filter::ReleaseDone()
{
    AVTRANS_LOGI("Release in %{public}s", name_.c_str());
    Status ret = DoRelease();
    SetErrCode(ret);
    ChangeState(ret == Status::OK ? FilterState::RELEASED : FilterState::ERROR);
    return ret;
}

Status Filter::SetPlayRange(int64_t start, int64_t end)
{
    AVTRANS_LOGD("SetPlayRange %{public}s, pState:%{public}d", name_.c_str(), curState_);
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter != nullptr) {
                filter->SetPlayRange(start, end);
            }
        }
    }
    return DoSetPlayRange(start, end);
}

Status Filter::Preroll()
{
    Status ret = DoPreroll();
    if (ret != Status::OK) {
        return ret;
    }
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter == nullptr) {
                continue;
            }
            ret = filter->Preroll();
            if (ret != Status::OK) {
                return ret;
            }
        }
    }
    return Status::OK;
}

Status Filter::WaitPrerollDone(bool render)
{
    Status ret = Status::OK;
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter == nullptr) {
                continue;
            }
            auto curRet = filter->WaitPrerollDone(render);
            if (curRet != Status::OK) {
                ret = curRet;
            }
        }
    }
    auto curRet = DoWaitPrerollDone(render);
    if (curRet != Status::OK) {
        ret = curRet;
    }
    return ret;
}

void Filter::StartFilterTask()
{
    if (filterTask_) {
        filterTask_->Start();
    }
}

void Filter::PauseFilterTask()
{
    if (filterTask_) {
        filterTask_->Pause();
    }
}

Status Filter::ClearAllNextFilters()
{
    nextFiltersMap_.clear();
    return Status::OK;
}

Status Filter::ProcessInputBuffer(int sendArg, int64_t delayUs)
{
    AVTRANS_LOGD("Filter::ProcessInputBuffer  %{public}s", name_.c_str());
    if (filterTask_) {
        jobIdx_++;
        filterTask_->SubmitJob([this, sendArg]() {
            processIdx_++;
            DoProcessInputBuffer(sendArg, processIdx_ <= jobIdxBase_);  // drop frame after flush
            }, delayUs, false);
    } else {
        Media::Task::SleepInTask(delayUs / 1000); // 1000 convert to ms
        DoProcessInputBuffer(sendArg, false);
    }
    return Status::OK;
}

Status Filter::ProcessOutputBuffer(int sendArg, int64_t delayUs, bool byIdx, uint32_t idx, int64_t renderTime)
{
    AVTRANS_LOGD("Filter::ProcessOutputBuffer  %{public}s", name_.c_str());
    if (filterTask_) {
        jobIdx_++;
        int64_t processIdx = jobIdx_;
        filterTask_->SubmitJob([this, sendArg, processIdx, byIdx, idx, renderTime]() {
            processIdx_++;
            // drop frame after flush
            DoProcessOutputBuffer(sendArg, processIdx <= jobIdxBase_, byIdx, idx, renderTime);
            }, delayUs, false);
    } else {
        Media::Task::SleepInTask(delayUs / 1000); // 1000 convert to ms
        DoProcessOutputBuffer(sendArg, false, false, idx, renderTime);
    }
    return Status::OK;
}

Status Filter::DoInitAfterLink()
{
    AVTRANS_LOGI("Filter::DoInitAfterLink");
    return Status::OK;
}

Status Filter::DoPrepare()
{
    return Status::OK;
}

Status Filter::DoStart()
{
    return Status::OK;
}

Status Filter::DoPause()
{
    return Status::OK;
}

Status Filter::DoPauseDragging()
{
    return Status::OK;
}

Status Filter::DoResume()
{
    return Status::OK;
}

Status Filter::DoResumeDragging()
{
    return Status::OK;
}

Status Filter::DoStop()
{
    return Status::OK;
}

Status Filter::DoFlush()
{
    return Status::OK;
}

Status Filter::DoRelease()
{
    return Status::OK;
}

Status Filter::DoPreroll()
{
    return Status::OK;
}

Status Filter::DoWaitPrerollDone(bool render)
{
    return Status::OK;
}

Status Filter::DoSetPlayRange(int64_t start, int64_t end)
{
    return Status::OK;
}

Status Filter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    return Status::OK;
}

Status Filter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx, uint32_t idx, int64_t renderTimee)
{
    return Status::OK;
}

// should only call in this cpp
void Filter::ChangeState(FilterState state)
{
    AVTRANS_LOGI("%{public}s > %{public}d", name_.c_str(), state);
    Media::AutoLock lock(stateMutex_);
    curState_ = state;
    cond_.NotifyOne();
}

Status Filter::WaitAllState(FilterState state)
{
    Media::AutoLock lock(stateMutex_);
    if (curState_ != state) {
        cond_.WaitFor(lock, 30000, [this, state] { // 30000 ms timeout
            return curState_ == state || curState_ == FilterState::ERROR;
        });
        if (curState_ != state) {
            AVTRANS_LOGE("Filter(%{public}s) wait state %{public}d fail, curState %{public}d",
                name_.c_str(), state, curState_);
            return GetErrCode();
        }
    }

    Status res = Status::OK;
    for (auto iter : nextFiltersMap_) {
        for (auto filter : iter.second) {
            if (filter == nullptr) {
                continue;
            }
            if (filter->WaitAllState(state) != Status::OK) {
                res = filter->GetErrCode();
            }
        }
    }
    return res;
}

bool Filter::IsDesignatedState(FilterState state)
{
    return curState_ == state;
}

void Filter::SetErrCode(Status errCode)
{
    errCode_ = errCode;
}

Status Filter::GetErrCode()
{
    return errCode_;
}

void Filter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void Filter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status Filter::LinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status Filter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status Filter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

FilterType Filter::GetFilterType()
{
    return filterType_;
};

Status Filter::OnLinked(StreamType, const std::shared_ptr<Media::Meta>&, const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
};

Status Filter::OnUpdated(StreamType, const std::shared_ptr<Media::Meta>&, const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status Filter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
