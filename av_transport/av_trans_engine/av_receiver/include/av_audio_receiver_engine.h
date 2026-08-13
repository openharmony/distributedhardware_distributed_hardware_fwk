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

#ifndef OHOS_AV_AUDIO_RECEIVER_ENGINE_H
#define OHOS_AV_AUDIO_RECEIVER_ENGINE_H

#include "av_trans_audio_decoder_filter.h"
#include "av_trans_buffer.h"
#include "av_trans_bus_input_filter.h"
#include "av_trans_constants.h"
#include "av_trans_control_center_callback.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "av_trans_message.h"
#include "av_trans_types.h"
#include "daudio_output_filter.h"
#include "distributed_hardware_fwk_kit.h"
#include "i_av_receiver_engine.h"
#include "pipeline.h"
#include "pipeline_event.h"
#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {
class AVAudioReceiverEngine : public IAVReceiverEngine,
                         public ISoftbusChannelListener,
                         public std::enable_shared_from_this<AVAudioReceiverEngine>  {
public:
    AVAudioReceiverEngine(const std::string &ownerName, const std::string &peerDevId);
    ~AVAudioReceiverEngine() override;
    AVAudioReceiverEngine(const AVAudioReceiverEngine &other) = delete;
    AVAudioReceiverEngine& operator=(const AVAudioReceiverEngine &other) = delete;

    // interfaces from IAVReceiverEngine
    int32_t Initialize() override;
    int32_t Start() override;
    int32_t Stop() override;
    int32_t Release() override;
    int32_t SetParameter(AVTransTag tag, const std::string &value) override;
    int32_t SendMessage(const std::shared_ptr<AVTransMessage> &message) override;
    int32_t CreateControlChannel(const std::vector<std::string> &dstDevIds,
        const ChannelAttribute &attribution) override;
    int32_t RegisterReceiverCallback(const std::shared_ptr<IAVReceiverEngineCallback> &callback) override;
    bool StartDumpMediaData() override;
    bool StopDumpMediaData() override;
    bool ReStartDumpMediaData() override;

    // interfaces from ISoftbusChannelListener
    void OnChannelEvent(const AVTransEvent &event) override;
    void OnStreamReceived(const StreamData *data, const StreamData *ext) override;

    void OnEvent(const Pipeline::Event &event);
    Status OnCallback(const std::shared_ptr<Pipeline::Filter>& filter, const Pipeline::FilterCallBackCommand cmd,
        Pipeline::StreamType outType);
    Status LinkAudioDecoderFilter(const std::shared_ptr<Pipeline::Filter>& preFilter, Pipeline::StreamType type);
    Status LinkAudioSinkFilter(const std::shared_ptr<Pipeline::Filter>& preFilter, Pipeline::StreamType type);

    Status Prepare();

private:
    int32_t InitPipeline();
    int32_t InitControlCenter();
    int32_t PreparePipeline(const std::string &configParam);
    int32_t HandleOutputBuffer(std::shared_ptr<Media::AVBuffer> &hisBuffer);
    bool ConvertToInt(const std::string& str, int& value);

    void RegRespFunMap();
    void SetVideoWidth(const std::string &value);
    void SetVideoHeight(const std::string &value);
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
    void SetSyncResult(const std::string &value);
    void SetStartAvSync(const std::string &value);
    void SetStopAvSync(const std::string &value);
    void SetSharedMemoryFd(const std::string &value);
    void SetEngineReady(const std::string &value);
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
    static constexpr uint8_t TIME_OUT_MS = 50;
    int32_t engineId_ = 0;
    std::string ownerName_;
    std::string sessionName_;
    std::string peerDevId_;
    std::mutex stateMutex_;
    std::atomic<bool> isInitialized_ = false;
    std::atomic<StateId> currentState_ = StateId::IDLE;

    sptr<AVTransControlCenterCallback> ctlCtrCallback_ = nullptr;
    std::shared_ptr<DistributedHardwareFwkKit> dhFwkKit_ = nullptr;
    std::shared_ptr<IAVReceiverEngineCallback> receiverCallback_ = nullptr;
    std::shared_ptr<Pipeline::Pipeline> pipeline_ = nullptr;
    std::shared_ptr<Pipeline::EventReceiver> playEventReceiver_ = nullptr;
    std::shared_ptr<Pipeline::FilterCallback> playFilterCallback_ = nullptr;
    std::string receiverId_ = "";
    std::shared_ptr<Pipeline::AVTransBusInputFilter> avInput_ = nullptr;
    std::shared_ptr<Pipeline::AudioDecoderFilter> audioDecoder_ = nullptr;
    std::shared_ptr<Pipeline::DAudioOutputFilter> avOutput_ = nullptr;
    std::shared_ptr<Media::Meta> meta_ = nullptr;

    using SetParaFunc = void (AVAudioReceiverEngine::*)(const std::string &value);
    std::map<AVTransTag, SetParaFunc> funcMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_AUDIO_RECEIVER_ENGINE_H