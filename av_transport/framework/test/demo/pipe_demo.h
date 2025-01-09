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

#ifndef OHOS_AV_PIPELINE_DEMO_H
#define OHOS_AV_PIPELINE_DEMO_H

#include <string>
#include <vector>
#include <memory>

#include "osal/task/mutex.h"

#include "filter.h"
#include "head_filter.h"
#include "mid_filter.h"
#include "pipeline_status.h"
#include "pipeline/include/pipeline.h"
#include "tail_filter.h"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
class PipeDemo {
public:
    PipeDemo();
    ~PipeDemo();

    int32_t InitPipe();

    int32_t Prepare();

    int32_t Start();

    int32_t Pause();

    int32_t Stop();

    int32_t Release();

    Status OnCallback(std::shared_ptr<Filter> filter, const FilterCallBackCommand cmd,
        StreamType outType);
    Status LinkAudioDecoderFilter(const std::shared_ptr<Filter>& preFilter, StreamType type);
    Status LinkAudioSinkFilter(const std::shared_ptr<Filter>& preFilter, StreamType type);

private:
    std::string demoId_ {-1};
    std::shared_ptr<EventReceiver> playerEventReceiver_ {nullptr};
    std::shared_ptr<FilterCallback> playerFilterCallback_ {nullptr};
    std::shared_ptr<Pipeline> pipeline_ {nullptr};
    std::shared_ptr<HeadFilter> headFilter_ {nullptr};
    std::shared_ptr<MidFilter> midFilter_ {nullptr};
    std::shared_ptr<TailFilter> tailFilter_ {nullptr};
};
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_PIPELINE_DEMO_H
