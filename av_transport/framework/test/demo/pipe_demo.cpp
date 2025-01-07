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

#include "pipe_demo.h"

#include <iostream>
#include <string>

#include "av_trans_log.h"
#include "filter_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "PipeDemo"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
class PlayerEventReceiver : public EventReceiver {
public:
    explicit PlayerEventReceiver(PipeDemo* pipeDemo, std::string playerId)
    {
        AVTRANS_LOGI("PlayerEventReceiver ctor called.");
        pipeDemo_ = pipeDemo;
    }

    void OnEvent(const Event &event)
    {
        AVTRANS_LOGD("PlayerEventReceiver OnEvent.");
    }

private:
    PipeDemo* pipeDemo_;
};

class PlayerFilterCallback : public FilterCallback {
public:
    explicit PlayerFilterCallback(PipeDemo* pipeDemo)
    {
        AVTRANS_LOGI("PlayerFilterCallback ctor called.");
        pipeDemo_ = pipeDemo;
    }

    Status OnCallback(const std::shared_ptr<Filter>& filter, FilterCallBackCommand cmd, StreamType outType) override
    {
        AVTRANS_LOGI("PlayerFilterCallback OnCallback.");
        if (pipeDemo_ == nullptr) {
            return Status::ERROR_NULL_POINTER;
        }
        return pipeDemo_->OnCallback(filter, cmd, outType);
    }

private:
    PipeDemo* pipeDemo_;
};


PipeDemo::PipeDemo() {}

PipeDemo::~PipeDemo() {}

int32_t PipeDemo::InitPipe()
{
    // 初始化pipeline
    demoId_ = std::string("PipeDemo") + std::to_string(Pipeline::GetNextPipelineId());
    pipeline_ = std::make_shared<Pipeline>();
    playerEventReceiver_ = std::make_shared<PlayerEventReceiver>(this, demoId_);
    playerFilterCallback_ = std::make_shared<PlayerFilterCallback>(this);

    AVTRANS_LOGI("pipeline Init in");
    pipeline_->Init(playerEventReceiver_, playerFilterCallback_, demoId_);
    AVTRANS_LOGI("pipeline Init out");
    // 添加头Filter
    headFilter_ = FilterFactory::Instance().CreateFilter<HeadFilter>("builtin.recorder.test1",
        FilterType::FILTERTYPE_MUXER);
    if (headFilter_ == nullptr) {
        return -1;
    }

    headFilter_->Init(playerEventReceiver_, playerFilterCallback_);
    pipeline_->AddHeadFilters({headFilter_});
    return 0;
}

int32_t PipeDemo::Prepare()
{
    if (pipeline_ != nullptr) {
        pipeline_->Prepare();
    }
    return 0;
}

int32_t PipeDemo::Start()
{
    if (pipeline_ != nullptr) {
        pipeline_->Start();
    }
    return 0;
}

int32_t PipeDemo::Pause()
{
    if (pipeline_ != nullptr) {
        pipeline_->Pause();
    }
    return 0;
}

int32_t PipeDemo::Stop()
{
    if (pipeline_ != nullptr) {
        pipeline_->Stop();
    }
    return 0;
}

int32_t PipeDemo::Release()
{
    if (pipeline_ != nullptr) {
        pipeline_->Release();
    }
    return 0;
}

Status PipeDemo::OnCallback(std::shared_ptr<Filter> filter, const FilterCallBackCommand cmd,
    StreamType outType)
{
    AVTRANS_LOGI("PipeDemo::OnCallback filter, outType: %{public}d", outType);
    if (cmd == FilterCallBackCommand::NEXT_FILTER_NEEDED) {
        switch (outType) {
            case StreamType::STREAMTYPE_RAW_AUDIO:
                return LinkAudioSinkFilter(filter, outType);
            case StreamType::STREAMTYPE_ENCODED_AUDIO:
                return LinkAudioDecoderFilter(filter, outType);
            default:
                break;
        }
    }
    return Status::OK;
}

Status PipeDemo::LinkAudioDecoderFilter(const std::shared_ptr<Filter>& preFilter, StreamType type)
{
    AVTRANS_LOGI("PipeDemo::LinkAudioDecoderFilter");
    TRUE_RETURN_V(midFilter_ != nullptr, Status::OK);
    midFilter_ = FilterFactory::Instance().CreateFilter<MidFilter>("builtin.recorder.midfilter",
        FilterType::FILTERTYPE_VIDEODEC);
    TRUE_RETURN_V(midFilter_ == nullptr, Status::ERROR_NULL_POINTER);
    midFilter_->Init(playerEventReceiver_, playerFilterCallback_);

    return pipeline_->LinkFilters(preFilter, {midFilter_}, type);
}

Status PipeDemo::LinkAudioSinkFilter(const std::shared_ptr<Filter>& preFilter, StreamType type)
{
    AVTRANS_LOGI("PipeDemo::LinkAudioSinkFilter");
    TRUE_RETURN_V(tailFilter_ != nullptr, Status::OK);
    tailFilter_ = FilterFactory::Instance().CreateFilter<TailFilter>("builtin.recorder.tailfilter",
        FilterType::FILTERTYPE_DEMUXER);
    TRUE_RETURN_V(tailFilter_ == nullptr, Status::ERROR_NULL_POINTER);
    tailFilter_->Init(playerEventReceiver_, playerFilterCallback_);

    return pipeline_->LinkFilters(preFilter, {tailFilter_}, type);
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS


int main(int argc, char *argv[])
{
    auto pipeDemo = std::make_shared<OHOS::DistributedHardware::Pipeline::PipeDemo>();
    std::cout << "Initing..." << std::endl;
    pipeDemo->InitPipe();
    std::cout << "Init piep success" << std::endl;

    std::cout << "Preparing..." << std::endl;
    pipeDemo->Prepare();
    std::cout << "Prepare success" << std::endl;

    std::cout << "Starting..." << std::endl;
    pipeDemo->Start();
    std::cout << "Start success" << std::endl;

    std::cout << "Pausing..." << std::endl;
    pipeDemo->Pause();
    std::cout << "Pause success" << std::endl;

    std::cout << "Stoping..." << std::endl;
    pipeDemo->Stop();
    std::cout << "Stop success" << std::endl;

    std::cout << "Releasing..." << std::endl;
    pipeDemo->Release();
    std::cout << "Release success" << std::endl;

    return 0;
}
