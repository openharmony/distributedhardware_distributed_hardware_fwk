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

#include "av_receiver_engine.h"

#include "pipeline/factory/filter_factory.h"
#include "plugin_video_tags.h"

namespace OHOS {
namespace DistributedHardware {
AVReceiverEngine::AVReceiverEngine(const std::string &ownerName, const std::string &peerDevId)
    : ownerName_(ownerName), peerDevId_(peerDevId)
{
    AVTRANS_LOGI("AVReceiverEngine ctor.");
    sessionName_ = ownerName_ + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX;
}

AVReceiverEngine::~AVReceiverEngine()
{
    AVTRANS_LOGI("AVReceiverEngine dctor.");
    Release();

    pipeline_ = nullptr;
    avInput_ = nullptr;
    avOutput_ = nullptr;
    audioDecoder_ = nullptr;
    videoDecoder_ = nullptr;
    ctlCenCallback_ = nullptr;
}

int32_t AVReceiverEngine::Initialize()
{
    TRUE_RETURN_V_MSG_E(initialized_.load(), DH_AVT_SUCCESS, "sender engine has been initialized");

    int32_t ret = InitPipeline();
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init pipeline failed");

    ret = InitControlCenter();
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init av control center failed");

    ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "register receiver channel callback failed");

    initialized_ = true;
    SetCurrentState(StateId::INITIALIZED);
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::InitPipeline()
{
    AVTRANS_LOGI("InitPipeline enter.");
    FilterFactory::Instance().Init();
    avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    TRUE_RETURN_V_MSG_E(avInput_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av input filter failed");

    avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    TRUE_RETURN_V_MSG_E(avOutput_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av output filter failed");

    std::shared_ptr<OHOS::Media::Pipeline::CodecMode> vCodecMode = std::make_shared<AsyncMode>("videoDec");
    videoDecoder_ = std::make_shared<VideoDecoderFilter>(VDECODER_NAME, vCodecMode);
    TRUE_RETURN_V_MSG_E(videoDecoder_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av video decoder filter failed");
    
    std::shared_ptr<OHOS::Media::Pipeline::CodecMode> aCodecMode = std::make_shared<AsyncMode>("audioDec");
    audioDecoder_ = std::make_shared<AudioDecoderFilter>(ADECODER_NAME, aCodecMode);
    TRUE_RETURN_V_MSG_E(audioDecoder_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av audio decoder filter failed");

    ErrorCode ret;
    pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    pipeline_->Init(this, nullptr);
    if ((ownerName_ == OWNER_NAME_D_SCREEN) || (ownerName_ == OWNER_NAME_D_CAMERA)) {
        ret = pipeline_->AddFilters({avInput_.get(), videoDecoder_.get(), avOutput_.get()});
        if (ret == ErrorCode::SUCCESS) {
            ret = pipeline_->LinkFilters({avInput_.get(), videoDecoder_.get(), avOutput_.get()});
        }
    } else if ((ownerName_ == OWNER_NAME_D_MIC) || (ownerName_ == OWNER_NAME_D_SPEAKER)) {
        ret = pipeline_->AddFilters({avInput_.get(), audioDecoder_.get(), avOutput_.get()});
        if (ret == ErrorCode::SUCCESS) {
            ret = pipeline_->LinkFilters({avInput_.get(), audioDecoder_.get(), avOutput_.get()});
        }
    } else {
        AVTRANS_LOGI("unsupport ownerName:%s", ownerName_.c_str());
        return ERR_DH_AVT_INVALID_PARAM_VALUE;
    }
    if (ret != ErrorCode::SUCCESS) {
        pipeline_->RemoveFilterChain(avInput_.get());
    }
    return (ret == ErrorCode::SUCCESS) ? DH_AVT_SUCCESS : ERR_DH_AVT_INVALID_OPERATION;
}

int32_t AVReceiverEngine::InitControlCenter()
{
    int32_t ret = AVTransControlCenterKit::GetInstance().Initialize(TransRole::AV_RECEIVER, engineId_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CTRL_CENTER_INIT_FAIL, "init av trans control center failed");

    ctlCenCallback_ = new (std::nothrow) AVTransControlCenterCallback();
    TRUE_RETURN_V_MSG_E(ctlCenCallback_ == nullptr, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "new control center callback failed");

    std::shared_ptr<IAVReceiverEngine> engine = std::shared_ptr<AVReceiverEngine>(shared_from_this());
    ctlCenCallback_->SetReceiverEngine(engine);

    ret = AVTransControlCenterKit::GetInstance().RegisterCtlCenterCallback(engineId_, ctlCenCallback_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "register control center callback failed");

    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::CreateControlChannel(const std::vector<std::string> &dstDevIds,
    const ChannelAttribute &attribution)
{
    (void)attribution;
    AVTRANS_LOGI("CreateControlChannel enter.");
    TRUE_RETURN_V_MSG_E(dstDevIds.empty(), ERR_DH_AVT_NULL_POINTER, "dst deviceId vector is empty");

    peerDevId_ = dstDevIds[0];
    int32_t ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED,
        "register receiver control channel callback failed");

    std::string peerSessName = ownerName_ + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX;
    ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(sessionName_, peerSessName, peerDevId_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED,
        "create receiver control channel failed");

    SetCurrentState(StateId::CH_CREATING);
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::PreparePipeline(const std::string &configParam)
{
    AVTRANS_LOGI("PreparePipeline enter.");

    StateId currentState = GetCurrentState();
    bool isErrState = ((currentState != StateId::INITIALIZED) && (currentState != StateId::CH_CREATED));
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_PREPARE_FAILED,
        "current state=%" PRId32 " is invalid.", currentState);

    TRUE_RETURN_V_MSG_E((avInput_ == nullptr) || (avOutput_ == nullptr), ERR_DH_AVT_PREPARE_FAILED,
        "av input or output filter is null");

    ErrorCode ret = avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_TYPE),
        TransName2MediaType(ownerName_));
    TRUE_RETURN_V(ret != ErrorCode::SUCCESS, ERR_DH_AVT_SET_PARAM_FAILED);

    ret = avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_BIT_STREAM_FORMAT),
        VideoBitStreamFormat::ANNEXB);
    TRUE_RETURN_V(ret != ErrorCode::SUCCESS, ERR_DH_AVT_SET_PARAM_FAILED);

    ret = avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_DESCRIPTION),
        BuildChannelDescription(ownerName_, peerDevId_));
    TRUE_RETURN_V(ret != ErrorCode::SUCCESS, ERR_DH_AVT_SET_PARAM_FAILED);

    ret = pipeline_->Prepare();
    TRUE_RETURN_V(ret != ErrorCode::SUCCESS, ERR_DH_AVT_PREPARE_FAILED);

    SetCurrentState(StateId::CH_CREATED);
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::Start()
{
    AVTRANS_LOGI("Start enter.");

    bool isErrState = (GetCurrentState() != StateId::CH_CREATED);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_START_FAILED, "current state=%" PRId32 " is invalid.",
        GetCurrentState());

    ErrorCode ret = pipeline_->Start();
    TRUE_RETURN_V(ret != ErrorCode::SUCCESS, ERR_DH_AVT_START_FAILED);
    SetCurrentState(StateId::STARTED);
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::Stop()
{
    AVTRANS_LOGI("Stop enter.");
    ErrorCode ret = pipeline_->Pause();
    TRUE_RETURN_V(ret != ErrorCode::SUCCESS, ERR_DH_AVT_STOP_FAILED);
    SetCurrentState(StateId::STOPPED);
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::Release()
{
    AVTRANS_LOGI("Release enter.");
    TRUE_RETURN_V(GetCurrentState() == StateId::IDLE, DH_AVT_SUCCESS);
    AVTransControlCenterKit::GetInstance().Release(engineId_);
    pipeline_->Stop();
    SoftbusChannelAdapter::GetInstance().CloseSoftbusChannel(sessionName_, peerDevId_);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, peerDevId_);
    initialized_ = false;
    SetCurrentState(StateId::IDLE);
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::SetParameter(AVTransTag tag, const std::string &value)
{
    AVTRANS_LOGI("SetParameter enter. tag:%" PRId32, tag);
    switch (tag) {
        case AVTransTag::VIDEO_WIDTH: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_WIDTH), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter VIDEO_WIDTH success, video width = %s", value.c_str());
            break;
        }
        case AVTransTag::VIDEO_HEIGHT: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_HEIGHT), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter VIDEO_HEIGHT success, video height = %s", value.c_str());
            break;
        }
        case AVTransTag::VIDEO_FRAME_RATE: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_FRAME_RATE), std::atoi(value.c_str()));
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_FRAME_RATE), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter VIDEO_FRAME_RATE success, frame rate = %s", value.c_str());
            break;
        }
        case AVTransTag::AUDIO_BIT_RATE:
        case AVTransTag::VIDEO_BIT_RATE: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_BITRATE), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter MEDIA_BITRATE success, bit rate = %s", value.c_str());
            break;
        }
        case AVTransTag::VIDEO_CODEC_TYPE: {
            if (value == MIME_VIDEO_H264) {
                std::string mime = MEDIA_MIME_VIDEO_H264;
                avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
                mime = MEDIA_MIME_VIDEO_RAW;
                avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
                AVTRANS_LOGI("SetParameter VIDEO_CODEC_TYPE = H264 success");
            } else if (value == MIME_VIDEO_H265) {
                std::string mime = MEDIA_MIME_VIDEO_H265;
                avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
                mime = MEDIA_MIME_VIDEO_RAW;
                avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
                AVTRANS_LOGI("SetParameter VIDEO_CODEC_TYPE = H265 success");
            } else {
                AVTRANS_LOGE("SetParameter VIDEO_CODEC_TYPE failed, input value invalid.");
            }
            break;
        }
        case AVTransTag::AUDIO_CODEC_TYPE: {
            std::string mime = MEDIA_MIME_AUDIO_AAC;
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
            mime = MEDIA_MIME_AUDIO_RAW;
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
            AVTRANS_LOGI("SetParameter AUDIO_CODEC_TYPE = AAC success");
            break;
        }
        case AVTransTag::AUDIO_CHANNEL_MASK: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNELS), std::atoi(value.c_str()));
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNELS), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter AUDIO_CHANNELS success, audio channels = %s", value.c_str());
            break;
        }
        case AVTransTag::AUDIO_SAMPLE_RATE: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_RATE), std::atoi(value.c_str()));
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_RATE), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_RATE success, audio sample rate = %s", value.c_str());
            break;
        }
        case AVTransTag::AUDIO_CHANNEL_LAYOUT: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNEL_LAYOUT), std::atoi(value.c_str()));
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNEL_LAYOUT), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter AUDIO_CHANNEL_LAYOUT success, audio channel layout = %s", value.c_str());
            break;
        }
        case AVTransTag::AUDIO_SAMPLE_FORMAT: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_FORMAT), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_FORMAT success, audio sample format = %s", value.c_str());
            break;
        }
        case AVTransTag::AUDIO_FRAME_SIZE: {
            avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_PER_FRAME), std::atoi(value.c_str()));
            AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_PER_FRAME success, audio sample per frame = %s", value.c_str());
            break;
        }
        case AVTransTag::TIME_SYNC_RESULT: {
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::USER_TIME_SYNC_RESULT), value);
            AVTRANS_LOGI("SetParameter TIME_SYNC_RESULT success, time sync result = %s", value.c_str());
            break;
        }
        case AVTransTag::START_AV_SYNC: {
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::USER_AV_SYNC_GROUP_INFO), value);
            AVTRANS_LOGI("SetParameter START_AV_SYNC success, av sync group info = %s", value.c_str());
            break;
        }
        case AVTransTag::SHARED_MEMORY_FD: {
            avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::USER_SHARED_MEMORY_FD), value);
            AVTRANS_LOGI("SetParameter SHARED_MEMORY_FD success, shared memory info = %s", value.c_str());
            break;
        }
        case AVTransTag::ENGINE_READY: {
            int32_t ret = PreparePipeline(value);
            TRUE_RETURN_V(ret != DH_AVT_SUCCESS, ERR_DH_AVT_SETUP_FAILED);
            break;
        }
        default:
            AVTRANS_LOGE("Invalid tag.");
    }
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::SendMessage(const std::shared_ptr<AVTransMessage> &message)
{
    AVTRANS_LOGI("SendMessage enter.");
    TRUE_RETURN_V_MSG_E(message == nullptr, ERR_DH_AVT_INVALID_PARAM, "input message is nullptr.");
    std::string msgData = message->MarshalMessage();
    return SoftbusChannelAdapter::GetInstance().SendBytesData(sessionName_, message->dstDevId_, msgData);
}

int32_t AVReceiverEngine::RegisterReceiverCallback(const std::shared_ptr<IAVReceiverEngineCallback> &callback)
{
    AVTRANS_LOGI("RegisterReceiverCallback enter.");
    if (callback == nullptr) {
        AVTRANS_LOGE("RegisterReceiverCallback failed, receiver engine callback is nullptr.");
        return ERR_DH_AVT_INVALID_PARAM;
    }
    receiverCallback_ = callback;
    return DH_AVT_SUCCESS;
}

int32_t AVReceiverEngine::HandleOutputBuffer(std::shared_ptr<AVBuffer> &hisBuffer)
{
    AVTRANS_LOGI("HandleOutputBuffer enter.");

    StateId currentState = GetCurrentState();
    bool isErrState = (currentState != StateId::STARTED) && (currentState != StateId::PLAYING);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_OUTPUT_DATA_FAILED,
        "current state=%" PRId32 " is invalid.", currentState);

    std::shared_ptr<AVTransBuffer> transBuffer = HiSBuffer2TransBuffer(hisBuffer);
    TRUE_RETURN_V(transBuffer == nullptr, ERR_DH_AVT_OUTPUT_DATA_FAILED);

    SetCurrentState(StateId::PLAYING);
    TRUE_RETURN_V(receiverCallback_ == nullptr, ERR_DH_AVT_OUTPUT_DATA_FAILED);
    return receiverCallback_->OnDataAvailable(transBuffer);
}

void AVReceiverEngine::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter. event type:%" PRId32, event.type);
    TRUE_RETURN(receiverCallback_ == nullptr, "receiver callback is nullptr.");

    switch (event.type) {
        case EventType::EVENT_CHANNEL_OPENED: {
            receiverCallback_->OnReceiverEvent(event);
            SetCurrentState(StateId::CH_CREATED);
            break;
        }
        case EventType::EVENT_CHANNEL_OPEN_FAIL: {
            receiverCallback_->OnReceiverEvent(event);
            SetCurrentState(StateId::INITIALIZED);
            break;
        }
        case EventType::EVENT_CHANNEL_CLOSED: {
            StateId currentState = GetCurrentState();
            if ((currentState != StateId::IDLE) && (currentState != StateId::INITIALIZED)) {
                receiverCallback_->OnReceiverEvent(event);
            }
            SetCurrentState(StateId::INITIALIZED);
            break;
        }
        case EventType::EVENT_DATA_RECEIVED: {
            auto avMessage = std::make_shared<AVTransMessage>();
            TRUE_RETURN(!avMessage->UnmarshalMessage(event.content), "unmarshal message failed");
            receiverCallback_->OnMessageReceived(avMessage);
            break;
        }
        default:
            AVTRANS_LOGE("Invalid event type.");
    }
}

void AVReceiverEngine::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

void AVReceiverEngine::OnEvent(const OHOS::Media::Event &event)
{
    AVTRANS_LOGI("OnEvent enter. event type:%s", GetEventName(event.type));
    TRUE_RETURN(receiverCallback_ == nullptr, "receiver callback is nullptr.");

    switch (event.type) {
        case OHOS::Media::EventType::EVENT_BUFFER_PROGRESS: {
            if (event.param.SameTypeWith(typeid(std::shared_ptr<AVBuffer>))) {
                auto hisBuffer = Plugin::AnyCast<std::shared_ptr<AVBuffer>>(event.param);
                TRUE_RETURN(hisBuffer == nullptr, "hisBuffer is null");
                HandleOutputBuffer(hisBuffer);
            }
            break;
        }
        case OHOS::Media::EventType::EVENT_PLUGIN_EVENT: {
            Plugin::PluginEvent pluginEvent = Plugin::AnyCast<Plugin::PluginEvent>(event.param);
            receiverCallback_->OnReceiverEvent(AVTransEvent{ CastEventType(pluginEvent.type), "", peerDevId_ });
            break;
        }
        default:
            AVTRANS_LOGE("Invalid event type.");
    }
}
} // namespace DistributedHardware
} // namespace OHOS