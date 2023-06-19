/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_AV_SENDER_ENGINE_H
#define OHOS_AV_SENDER_ENGINE_H

#include "av_trans_buffer.h"
#include "av_trans_errno.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_message.h"
#include "av_trans_types.h"
#include "av_trans_utils.h"
#include "i_av_sender_engine.h"
#include "softbus_channel_adapter.h"
#include "av_trans_control_center_kit.h"
#include "av_trans_control_center_callback.h"

// follwing head files depends on histreamer
#include "error_code.h"
#include "event.h"
#include "pipeline/core/filter.h"
#include "pipeline_core.h"
#include "audio_encoder_filter.h"
#include "video_encoder_filter.h"
#include "av_transport_input_filter.h"
#include "av_transport_output_filter.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Pipeline;
using AVBuffer = OHOS::Media::Plugin::Buffer;

class AVSenderEngine : public IAVSenderEngine,
                       public ISoftbusChannelListener,
                       public OHOS::Media::Pipeline::EventReceiver,
                       public std::enable_shared_from_this<AVSenderEngine> {
public:
    AVSenderEngine(const std::string &ownerName, const std::string &peerDevId);
    ~AVSenderEngine() override;
    AVSenderEngine(const AVSenderEngine &other) = delete;
    AVSenderEngine& operator=(const AVSenderEngine &other) = delete;

    // interfaces from IAVSenderEngine
    int32_t Initialize() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Release() override;
    int32_t SetParameter(AVTransTag tag, const std::string &value) override;
    int32_t PushData(const std::shared_ptr<AVTransBuffer> &buffer) override;
    int32_t SendMessage(const std::shared_ptr<AVTransMessage> &message) override;
    int32_t CreateControlChannel(const std::vector<std::string> &dstDevIds,
        const ChannelAttribute &attribution) override;
    int32_t RegisterSenderCallback(const std::shared_ptr<IAVSenderEngineCallback> &callback) override;

    // interfaces from ISoftbusChannelListener
    void OnChannelEvent(const AVTransEvent &event) override;
    void OnStreamReceived(const StreamData *data, const StreamData *ext) override;

    // interfaces from OHOS::Media::Pipeline::EventReceiver
    void OnEvent(const OHOS::Media::Event &event) override;

private:
    int32_t InitPipeline();
    int32_t InitControlCenter();
    int32_t PreparePipeline(const std::string &configParam);
    void NotifyStreamChange(EventType type);

    StateId GetCurrentState()
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        return currentState_;
    }

    void SetCurrentState(StateId stateId)
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        currentState_ = stateId;
    }

private:
    int32_t engineId_ = 0;
    std::string ownerName_;
    std::string sessionName_;
    std::string peerDevId_;

    std::mutex stateMutex_;
    std::atomic<bool> initialized_ {false};
    std::atomic<StateId> currentState_ = StateId::IDLE;

    sptr<AVTransControlCenterCallback> ctlCenCallback_ = nullptr;
    std::shared_ptr<IAVSenderEngineCallback> senderCallback_ = nullptr;
    std::shared_ptr<OHOS::Media::Pipeline::PipelineCore> pipeline_ = nullptr;

    std::shared_ptr<OHOS::Media::Pipeline::AVInputFilter> avInput_ = nullptr;
    std::shared_ptr<OHOS::Media::Pipeline::AVOutputFilter> avOutput_ = nullptr;
    std::shared_ptr<OHOS::Media::Pipeline::AudioEncoderFilter> audioEncoder_ = nullptr;
    std::shared_ptr<OHOS::Media::Pipeline::VideoEncoderFilter> videoEncoder_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_SENDER_ENGINE_H