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

#include "pipeline/include/pipeline.h"

#include <queue>
#include <stack>

#include "osal/task/autolock.h"
#include "osal/task/jobutils.h"

#include "av_trans_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "Pipeline"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
static std::atomic<uint16_t> pipeLineId = 0;

int32_t Pipeline::GetNextPipelineId()
{
    return pipeLineId++;
}

Pipeline::~Pipeline()
{
}

void Pipeline::Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback,
    const std::string& groupId)
{
    AVTRANS_LOGI("Pipeline::Init");
    eventReceiver_ = receiver;
    filterCallback_ = callback;
    groupId_ = groupId;
}

Status Pipeline::Prepare()
{
    AVTRANS_LOGI("Prepare enter.");
    Status ret = Status::OK;
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            ret = (*it)->Prepare();
            if (ret != Status::OK) {
                return;
            }
        }
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            ret = (*it)->WaitAllState(FilterState::READY);
            if (ret != Status::OK) {
                return;
            }
        }
    });
    AVTRANS_LOGI("Prepare done ret = %{public}d", ret);
    return ret;
}

Status Pipeline::Start()
{
    AVTRANS_LOGI("Start enter.");
    Status ret = Status::OK;
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            ret = (*it)->Start();
            if (ret != Status::OK) {
                return;
            }
        }
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            ret = (*it)->WaitAllState(FilterState::RUNNING);
            if (ret != Status::OK) {
                return;
            }
        }
    });
    AVTRANS_LOGI("Start done ret = %{public}d", ret);
    return ret;
}

Status Pipeline::Pause()
{
    AVTRANS_LOGI("Pause enter.");
    Status ret = Status::OK;
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            auto rtv = (*it)->Pause();
            if (rtv != Status::OK) {
                ret = rtv;
            }
        }
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            auto rtv = (*it)->WaitAllState(FilterState::PAUSED);
            if (rtv != Status::OK) {
                ret = rtv;
            }
        }
    });
    AVTRANS_LOGI("Pause done ret = %{public}d", ret);
    return ret;
}

Status Pipeline::Resume()
{
    AVTRANS_LOGI("Resume enter.");
    Status ret = Status::OK;
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            ret = (*it)->Resume();
            if (ret != Status::OK) {
                return;
            }
        }
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            ret = (*it)->WaitAllState(FilterState::RUNNING);
            if (ret != Status::OK) {
                return;
            }
        }
    });
    AVTRANS_LOGI("Resume done ret = %{public}d", ret);
    return ret;
}

Status Pipeline::Stop()
{
    AVTRANS_LOGI("Stop enter.");
    Status ret = Status::OK;
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                AVTRANS_LOGE("Pipeline error: %{public}zu", filters_.size());
                continue;
            }
            auto rtv = (*it)->Stop();
            if (rtv != Status::OK) {
                ret = rtv;
            }
        }
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            auto rtv = (*it)->WaitAllState(FilterState::STOPPED);
            if (rtv != Status::OK) {
                ret = rtv;
            }
        }
        filters_.clear();
    });
    AVTRANS_LOGI("Stop done ret = %{public}d", ret);
    return ret;
}

Status Pipeline::Flush()
{
    AVTRANS_LOGI("Flush enter.");
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            (*it)->Flush();
        }
    });
    AVTRANS_LOGI("Flush end.");
    return Status::OK;
}

Status Pipeline::Release()
{
    AVTRANS_LOGI("Release enter.");
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            (*it)->Release();
        }
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            (*it)->WaitAllState(FilterState::RELEASED);
        }
        filters_.clear();
    });
    AVTRANS_LOGI("Release done.");
    return Status::OK;
}

Status Pipeline::Preroll(bool render)
{
    AVTRANS_LOGI("Preroll enter.");
    Status ret = Status::OK;
    Media::AutoLock lock(mutex_);
    for (auto it = filters_.begin(); it != filters_.end(); ++it) {
        if (*it == nullptr) {
            continue;
        }
        auto rtv = (*it)->Preroll();
        if (rtv != Status::OK) {
            ret = rtv;
            AVTRANS_LOGI("Preroll done ret = %{public}d", ret);
            return ret;
        }
    }
    for (auto it = filters_.begin(); it != filters_.end(); ++it) {
        if (*it == nullptr) {
            continue;
        }
        auto rtv = (*it)->WaitPrerollDone(render);
        if (rtv != Status::OK) {
            ret = rtv;
            AVTRANS_LOGI("Preroll done ret = %{public}d", ret);
            return ret;
        }
    }
    AVTRANS_LOGI("Preroll done ret = %{public}d", ret);
    return ret;
}

Status Pipeline::SetPlayRange(int64_t start, int64_t end)
{
    AVTRANS_LOGI("SetPlayRange enter.");
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        for (auto it = filters_.begin(); it != filters_.end(); ++it) {
            if (*it == nullptr) {
                continue;
            }
            (*it)->SetPlayRange(start, end);
        }
    });
    AVTRANS_LOGI("SetPlayRange done.");
    return Status::OK;
}

Status Pipeline::AddHeadFilters(std::vector<std::shared_ptr<Filter>> filtersIn)
{
    AVTRANS_LOGI("AddHeadFilters enter.");
    std::vector<std::shared_ptr<Filter>> filtersToAdd;
    for (auto& filterIn : filtersIn) {
        if (filterIn == nullptr) {
            continue;
        }
        bool matched = false;
        for (const auto& filter : filters_) {
            if (filterIn == filter) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            filtersToAdd.push_back(filterIn);
            filterIn->LinkPipeLine(groupId_);
        }
    }
    if (filtersToAdd.empty()) {
        AVTRANS_LOGI("filter already exists");
        return Status::OK;
    }
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        this->filters_.insert(this->filters_.end(), filtersToAdd.begin(), filtersToAdd.end());
    });
    AVTRANS_LOGI("AddHeadFilters done.");
    return Status::OK;
}

Status Pipeline::RemoveHeadFilter(const std::shared_ptr<Filter>& filter)
{
    Media::SubmitJobOnce([&] {
        Media::AutoLock lock(mutex_);
        auto it = std::find_if(filters_.begin(), filters_.end(),
                               [&filter](const std::shared_ptr<Filter>& filterPtr) { return filterPtr == filter; });
        if (it != filters_.end()) {
            filters_.erase(it);
        }
        if (filter != nullptr) {
            filter->Release();
            filter->WaitAllState(FilterState::RELEASED);
            filter->ClearAllNextFilters();
        }
        return Status::OK;
    });
    return Status::OK;
}

Status Pipeline::LinkFilters(const std::shared_ptr<Filter> &preFilter,
                             const std::vector<std::shared_ptr<Filter>> &nextFilters,
                             StreamType type)
{
    TRUE_RETURN_V(preFilter == nullptr, Status::ERROR_NULL_POINTER);
    for (auto nextFilter : nextFilters) {
        TRUE_RETURN_V(nextFilter == nullptr, Status::ERROR_NULL_POINTER);
        auto ret = preFilter->LinkNext(nextFilter, type);
        nextFilter->LinkPipeLine(groupId_);
        TRUE_RETURN_V(ret != Status::OK, ret);
    }
    return Status::OK;
}

Status Pipeline::UpdateFilters(const std::shared_ptr<Filter> &preFilter,
                               const std::vector<std::shared_ptr<Filter>> &nextFilters,
                               StreamType type)
{
    TRUE_RETURN_V(preFilter == nullptr, Status::ERROR_NULL_POINTER);
    for (auto nextFilter : nextFilters) {
        preFilter->UpdateNext(nextFilter, type);
    }
    return Status::OK;
}

Status Pipeline::UnLinkFilters(const std::shared_ptr<Filter> &preFilter,
                               const std::vector<std::shared_ptr<Filter>> &nextFilters,
                               StreamType type)
{
    TRUE_RETURN_V(preFilter == nullptr, Status::ERROR_NULL_POINTER);
    for (auto nextFilter : nextFilters) {
        preFilter->UnLinkNext(nextFilter, type);
    }
    return Status::OK;
}

void Pipeline::OnEvent(const Event& event)
{
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
