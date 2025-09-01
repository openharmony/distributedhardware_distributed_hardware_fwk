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

#ifndef OHOS_DSOFTFBUS_OUTPUT_AUDIO_FILTER_H
#define OHOS_DSOFTFBUS_OUTPUT_AUDIO_FILTER_H

#include <string>
#include <vector>
#include <memory>

#include "buffer/avbuffer_queue.h"
#include "buffer/avbuffer_queue_consumer.h"
#include "buffer/avbuffer_queue_producer.h"
#include "softbus_channel_adapter.h"

#include "pipeline_status.h"
#include "filter.h"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
class DSoftbusOutputFilter : public Filter,
                             public ISoftbusChannelListener,
                             public std::enable_shared_from_this<DSoftbusOutputFilter> {
public:
    DSoftbusOutputFilter(std::string name, FilterType type);
    virtual ~DSoftbusOutputFilter();

    void Init(const std::shared_ptr<EventReceiver>& receiver, const std::shared_ptr<FilterCallback>& callback) override;
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
    void OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta);
    void OnUpdatedResult(std::shared_ptr<Media::Meta>& meta);
    void OnChannelEvent(const AVTransEvent &event) override;
    void OnStreamReceived(const StreamData *data, const StreamData *ext) override;

private:
    void PrepareInputBuffer();
    Status ProcessAndSendBuffer(const std::shared_ptr<Media::AVBuffer> buffer);
    std::string MarshalAudioMeta(BufferDataType dataType, int64_t pts, int64_t ptsSpecail, uint32_t frameNumber);
    std::shared_ptr<Media::Meta> meta_ {nullptr};
    std::shared_ptr<Filter> nextFilter_ {nullptr};

    std::shared_ptr<EventReceiver> eventReceiver_ {nullptr};
    std::shared_ptr<FilterCallback> filterCallback_ {nullptr};
    std::shared_ptr<FilterLinkCallback> onLinkedResultCallback_ {nullptr};

    std::shared_ptr<Media::AVBufferQueue> outputBufQue_ {nullptr};
    sptr<Media::AVBufferQueueProducer> inputBufQueProducer_ {nullptr};
    sptr<Media::AVBufferQueueConsumer> inputBufQueConsumer_ {nullptr};
    sptr<Media::AVBufferQueueProducer> outputBufQueProducer_ {nullptr};
    std::mutex paramsMapMutex_;
    std::string ownerName_;
    std::string sessionName_;
    std::string peerDevId_;
};
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
#endif
