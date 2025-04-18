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

#ifndef OHOS_AV_PIPELINE_FILTER_BASE_H
#define OHOS_AV_PIPELINE_FILTER_BASE_H
#include <atomic>
#include <functional>
#include <list>
#include <memory>

#include "buffer/avbuffer_queue_producer.h"
#include "meta/meta.h"
#include "osal/task/condition_variable.h"
#include "osal/task/mutex.h"
#include "osal/task/task.h"

#include "pipeline_event.h"
#include "pipeline_status.h"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {

class Filter;

enum class FilterType {
    FILTERTYPE_SOURCE,
    FILTERTYPE_DEMUXER,
    FILTERTYPE_AENC,
    FILTERTYPE_ADEC,
    FILTERTYPE_VENC,
    FILTERTYPE_VDEC,
    FILTERTYPE_VIDEODEC,
    FILTERTYPE_MUXER,
    FILTERTYPE_ASINK,
    FILTERTYPE_FSINK,
    FILTERTYPE_SSINK,
    AUDIO_CAPTURE,
    AUDIO_DATA_SOURCE,
    VIDEO_CAPTURE,
    FILTERTYPE_VIDRESIZE,
    TIMED_METADATA,
    FILTERTYPE_MAX,
};

enum class StreamType {
    STREAMTYPE_PACKED,
    STREAMTYPE_ENCODED_AUDIO,
    STREAMTYPE_ENCODED_VIDEO,
    STREAMTYPE_DECODED_AUDIO,
    STREAMTYPE_DECODED_VIDEO,
    STREAMTYPE_RAW_AUDIO,
    STREAMTYPE_RAW_VIDEO,
    STREAMTYPE_SUBTITLE,
    STREAMTYPE_MAX,
};

enum class FilterState {
    CREATED,     // Filter created
    INITIALIZED, // Init called
    PREPARING,   // Prepare called
    READY,       // Ready Event reported
    RUNNING,     // Start called
    PAUSED,      // Pause called
    STOPPED,     // Stop called
    RELEASED,    // Release called
    ERROR,       // State fail
};

enum class FilterCallBackCommand {
    NEXT_FILTER_NEEDED,
    NEXT_FILTER_REMOVED,
    NEXT_FILTER_UPDATE,
    FILTER_CALLBACK_COMMAND_MAX,
};

class EventReceiver {
public:
    virtual ~EventReceiver() = default;
    virtual void OnEvent(const Event& event) = 0;
};

class FilterCallback {
public:
    virtual ~FilterCallback() = default;
    virtual Status OnCallback(const std::shared_ptr<Filter>& filter, FilterCallBackCommand cmd, StreamType outType) = 0;
};

class FilterLinkCallback {
public:
    virtual ~FilterLinkCallback() = default;
    virtual void OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue,
        std::shared_ptr<Media::Meta>& meta) = 0;
    virtual void OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta) = 0;
    virtual void OnUpdatedResult(std::shared_ptr<Media::Meta>& meta) = 0;
};

class Filter {
public:
    explicit Filter(std::string name, FilterType type, bool asyncMode = false);
    virtual ~Filter();
    virtual void Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback);

    virtual void LinkPipeLine(const std::string& groupId) final;

    virtual Status Prepare() final;

    virtual Status Start() final;

    virtual Status Pause() final;

    virtual Status PauseDragging() final;

    virtual Status Resume() final;

    virtual Status ResumeDragging() final;

    virtual Status Stop() final;

    virtual Status Flush() final;

    virtual Status Release() final;

    virtual Status Preroll() final;

    virtual Status WaitPrerollDone(bool render) final;

    virtual void StartFilterTask() final;

    virtual void PauseFilterTask() final;

    virtual Status SetPlayRange(int64_t start, int64_t end) final;

    virtual Status ProcessInputBuffer(int sendArg = 0, int64_t delayUs = 0) final;

    virtual Status ProcessOutputBuffer(int sendArg = 0, int64_t delayUs = 0, bool byIdx = false, uint32_t idx = 0,
        int64_t renderTime = -1) final;

    virtual Status WaitAllState(FilterState state) final;

    virtual Status DoInitAfterLink();

    virtual Status DoPrepare();

    virtual Status DoStart();

    virtual Status DoPause();

    virtual Status DoPauseDragging();

    virtual Status DoResume();

    virtual Status DoResumeDragging();

    virtual Status DoStop();

    virtual Status DoFlush();

    virtual Status DoRelease();

    virtual Status DoPreroll();

    virtual Status DoWaitPrerollDone(bool render);

    virtual Status DoSetPlayRange(int64_t start, int64_t end);

    virtual Status DoProcessInputBuffer(int recvArg, bool dropFrame);

    virtual Status DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx, uint32_t idx, int64_t renderTime);

    virtual void SetParameter(const std::shared_ptr<Media::Meta>& meta);

    virtual void GetParameter(std::shared_ptr<Media::Meta>& meta);

    virtual Status LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType);

    virtual Status UpdateNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType);

    virtual Status UnLinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType);

    FilterType GetFilterType();

    virtual Status OnLinked(StreamType inType, const std::shared_ptr<Media::Meta>& meta,
                            const std::shared_ptr<FilterLinkCallback>& callback);

    virtual Status OnUpdated(StreamType inType, const std::shared_ptr<Media::Meta>& meta,
                             const std::shared_ptr<FilterLinkCallback>& callback);

    virtual Status OnUnLinked(StreamType inType, const std::shared_ptr<FilterLinkCallback>& callback);

    virtual void ChangeState(FilterState state);

    virtual void SetErrCode(Status errCode);

    virtual Status GetErrCode();

    virtual Status ClearAllNextFilters();

    virtual Status SetMuted(bool isMuted)
    {
        (void)isMuted;
        return Status::OK;
    }

    virtual bool IsDesignatedState(FilterState state);
protected:
    virtual Status PrepareDone() final;

    virtual Status StartDone() final;

    virtual Status PauseDone() final;

    virtual Status ResumeDone() final;

    virtual Status StopDone() final;

    virtual Status ReleaseDone() final;

    std::string name_;

    std::shared_ptr<Media::Meta> meta_;

    FilterType filterType_;
    FilterState curState_;

    std::vector<StreamType> supportedInStreams_;
    std::vector<StreamType> supportedOutStreams_;

    OHOS::Media::Mutex stateMutex_{};
    OHOS::Media::ConditionVariable cond_{};

    std::map<StreamType, std::vector<std::shared_ptr<Filter>>> nextFiltersMap_;

    std::shared_ptr<EventReceiver> receiver_;

    std::shared_ptr<FilterCallback> callback_;

    std::map<StreamType, std::vector<std::shared_ptr<FilterLinkCallback>>> linkCallbackMaps_;

    Status errCode_ = Status::OK;

    std::unique_ptr<Media::Task> filterTask_;

    int64_t jobIdx_ = 0;

    int64_t processIdx_ = 0;

    int64_t jobIdxBase_ = 0;

    std::string groupId_;

    bool isAsyncMode_;
};

enum FilterPlaybackCommand {
    INIT = 0,
    PREPARE,
    START,
    PAUSE,
    RESUME,
    STOP,
    RELEASE,
    FLUSH,
    PROCESS_INPUT_BUFFER,
    PROCESS_OUTPUT_BUFFER,
};
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
#endif
