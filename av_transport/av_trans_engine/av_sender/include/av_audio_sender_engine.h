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

#ifndef OHOS_AV_AUDIO_SENDER_ENGINE_H
#define OHOS_AV_AUDIO_SENDER_ENGINE_H

#include "av_trans_buffer.h"
#include "av_trans_constants.h"
#include "av_trans_control_center_callback.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "av_trans_message.h"

#include "distributed_hardware_fwk_kit.h"

#include "pipeline.h"
#include "pipeline_event.h"
#include "i_av_sender_engine.h"
#include "softbus_channel_adapter.h"
#include "av_trans_audio_input_filter.h"
#include "av_trans_audio_encoder_filter.h"
#include "dsoftbus_output_filter.h"


namespace OHOS {
namespace DistributedHardware {
class AVAudioSenderEngine : public IAVSenderEngine,
                         public ISoftbusChannelListener,
                         public std::enable_shared_from_this<AVAudioSenderEngine>  {
public:
    AVAudioSenderEngine(const std::string &ownerName, const std::string &peerDevId);
    ~AVAudioSenderEngine() override;
    AVAudioSenderEngine(const AVAudioSenderEngine &other) = delete;
    AVAudioSenderEngine& operator=(const AVAudioSenderEngine &other) = delete;

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
    bool StartDumpMediaData() override;
    bool StopDumpMediaData() override;
    bool ReStartDumpMediaData() override;

    // interfaces from ISoftbusChannelListener
    void OnChannelEvent(const AVTransEvent &event) override;
    void OnStreamReceived(const StreamData *data, const StreamData *ext) override;

    // interfaces from OHOS::Media::Pipeline::EventReceiver
    Status OnCallback(std::shared_ptr<Pipeline::Filter> filter, const Pipeline::FilterCallBackCommand cmd,
        Pipeline::StreamType outType);
    Status LinkAudioEncoderFilter(const std::shared_ptr<Pipeline::Filter>& preFilter, Pipeline::StreamType type);
    Status LinkAudioSinkFilter(const std::shared_ptr<Pipeline::Filter>& preFilter, Pipeline::StreamType type);
    void OnEvent(const Pipeline::Event &event);

    Status Prepare();

private:
    int32_t InitPipeline();
    int32_t InitControlCenter();
    int32_t PreparePipeline(const std::string &configParam);
    void NotifyStreamChange(EventType type);
    bool ConvertToInt(const std::string& str, int& value);

    void RegRespFunMap();
    void SetVideoWidth(const std::string &value);
    void SetVideoHeight(const std::string &value);
    void SetVideoPixelFormat(const std::string &value);
    void SetVideoFrameRate(const std::string &value);
    void SetAudioBitRate(const std::string &value);
    void SetVideoBitRate(const std::string &value);
    void SetVideoCodecType(const std::string &value);
    void SetAudioCodecType(const std::string &value);
    void SetAudioChannelMask(const std::string &value);
    void SetAudioSampleRate(const std::string &value);
    void SetAudioChannelLayout(const std::string &value);
    void SetAudioSampleFormat(const std::string &value);
    void SetAudioFrameSize(const std::string &value);
    void SetSharedMemoryFd(const std::string &value);
    void SetEngineReady(const std::string &value);
    void SetEnginePause(const std::string &value);
    void SetEngineResume(const std::string &value);
    void SetParameterInner(AVTransTag tag, const std::string &value);

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
    static constexpr uint8_t TIME_OUT_MS = 50;

    std::mutex stateMutex_;
    std::atomic<bool> isInitialized_ = false;
    std::atomic<StateId> currentState_ = StateId::IDLE;

    sptr<AVTransControlCenterCallback> ctlCenCallback_ = nullptr;
    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit_ = nullptr;
    std::shared_ptr<IAVSenderEngineCallback> senderCallback_ = nullptr;
    std::shared_ptr<Pipeline::Pipeline> pipeline_ = nullptr;
    std::shared_ptr<Pipeline::EventReceiver> playEventReceiver_ = nullptr;
    std::shared_ptr<Pipeline::FilterCallback> playFilterCallback_ = nullptr;
    std::string senderId_ = "";
    std::shared_ptr<Pipeline::AVTransAudioInputFilter> avInput_ = nullptr;
    std::shared_ptr<Pipeline::AudioEncoderFilter> encoderFilter_ = nullptr;
    std::shared_ptr<Pipeline::DSoftbusOutputFilter> avOutput_ = nullptr;
    std::shared_ptr<Media::Meta> meta_ = nullptr;

    using SetParaFunc = void (AVAudioSenderEngine::*)(const std::string &value);
    std::map<AVTransTag, SetParaFunc> funcMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_AUDIO_SENDER_ENGINE_H