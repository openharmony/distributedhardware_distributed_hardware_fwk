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

#include "av_sender_engine.h"

#include "pipeline/factory/filter_factory.h"
#include "plugin_video_tags.h"

namespace OHOS {
namespace DistributedHardware {

using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
using namespace OHOS::Media::Pipeline;
using AVBuffer = OHOS::Media::Plugin::Buffer;

#undef DH_LOG_TAG
#define DH_LOG_TAG "AVSenderEngine"

AVSenderEngine::AVSenderEngine(const std::string &ownerName, const std::string &peerDevId)
    : ownerName_(ownerName), peerDevId_(peerDevId)
{
    AVTRANS_LOGI("AVSenderEngine ctor.");
    sessionName_ = ownerName_ + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX;
}

AVSenderEngine::~AVSenderEngine()
{
    AVTRANS_LOGI("AVSenderEngine dctor.");
    Release();

    dhFwkKit_ = nullptr;
    pipeline_ = nullptr;
    avInput_ = nullptr;
    avOutput_ = nullptr;
    audioEncoder_ = nullptr;
    videoEncoder_ = nullptr;
    senderCallback_ = nullptr;
    ctlCenCallback_ = nullptr;
}

int32_t AVSenderEngine::Initialize()
{
    TRUE_RETURN_V_MSG_E(isInitialized_.load(), DH_AVT_SUCCESS, "sender engine has been initialized");

    int32_t ret = InitPipeline();
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init pipeline failed");

    ret = InitControlCenter();
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init av control center failed");

    ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "register sender channel callback failed");
    RegRespFunMap();
    isInitialized_ = true;
    SetCurrentState(StateId::INITIALIZED);
    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::InitPipeline()
{
    AVTRANS_LOGI("InitPipeline enter.");
    FilterFactory::Instance().Init();
    avInput_ = FilterFactory::Instance().CreateFilterWithType<AVInputFilter>(AVINPUT_NAME, "avinput");
    TRUE_RETURN_V_MSG_E(avInput_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av input filter failed");

    avOutput_ = FilterFactory::Instance().CreateFilterWithType<AVOutputFilter>(AVOUTPUT_NAME, "avoutput");
    TRUE_RETURN_V_MSG_E(avOutput_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av output filter failed");

    videoEncoder_ = FilterFactory::Instance().CreateFilterWithType<VideoEncoderFilter>(VENCODER_NAME, "videoencoder");
    TRUE_RETURN_V_MSG_E(videoEncoder_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av video encoder filter failed");

    audioEncoder_ = FilterFactory::Instance().CreateFilterWithType<AudioEncoderFilter>(AENCODER_NAME, "audioencoder");
    TRUE_RETURN_V_MSG_E(audioEncoder_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av audio encoder filter failed");

    ErrorCode ret;
    pipeline_ = std::make_shared<OHOS::Media::Pipeline::PipelineCore>();
    pipeline_->Init(this, nullptr);
    if ((ownerName_ == OWNER_NAME_D_SCREEN) || (ownerName_ == OWNER_NAME_D_CAMERA)) {
        ret = pipeline_->AddFilters({avInput_.get(), videoEncoder_.get(), avOutput_.get()});
        if (ret == ErrorCode::SUCCESS) {
            ret = pipeline_->LinkFilters({avInput_.get(), videoEncoder_.get(), avOutput_.get()});
        }
    } else if ((ownerName_ == OWNER_NAME_D_MIC) || (ownerName_ == OWNER_NAME_D_SPEAKER) ||
               (ownerName_ == OWNER_NAME_D_VIRMODEM_MIC) || (ownerName_ == OWNER_NAME_D_VIRMODEM_SPEAKER)) {
        ret = pipeline_->AddFilters({avInput_.get(), avOutput_.get()});
        if (ret == ErrorCode::SUCCESS) {
            ret = pipeline_->LinkFilters({avInput_.get(), avOutput_.get()});
        }
    } else {
        AVTRANS_LOGI("unsupport ownerName:%{public}s", ownerName_.c_str());
        return ERR_DH_AVT_INVALID_PARAM_VALUE;
    }
    if (ret != ErrorCode::SUCCESS) {
        pipeline_->RemoveFilterChain(avInput_.get());
    }
    return (ret == ErrorCode::SUCCESS) ? DH_AVT_SUCCESS : ERR_DH_AVT_INVALID_OPERATION;
}

int32_t AVSenderEngine::InitControlCenter()
{
    dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = dhFwkKit_->InitializeAVCenter(TransRole::AV_SENDER, engineId_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CTRL_CENTER_INIT_FAIL, "init av trans control center failed");

    ctlCenCallback_ = new (std::nothrow) AVTransControlCenterCallback();
    TRUE_RETURN_V_MSG_E(ctlCenCallback_ == nullptr, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "new control center callback failed");

    std::shared_ptr<IAVSenderEngine> engine = std::shared_ptr<AVSenderEngine>(shared_from_this());
    ctlCenCallback_->SetSenderEngine(engine);

    ret = dhFwkKit_->RegisterCtlCenterCallback(engineId_, ctlCenCallback_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "register control center callback failed");

    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::CreateControlChannel(const std::vector<std::string> &dstDevIds,
    const ChannelAttribute &attribution)
{
    (void)attribution;
    AVTRANS_LOGI("CreateControlChannel enter.");
    TRUE_RETURN_V_MSG_E(dstDevIds.empty(), ERR_DH_AVT_NULL_POINTER, "dst deviceId vector is empty");

    peerDevId_ = dstDevIds[0];
    int32_t ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED,
        "register control channel callback failed");

    std::string peerSessName = ownerName_ + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX;
    ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(sessionName_, peerSessName, peerDevId_);
    TRUE_RETURN_V(ret == ERR_DH_AVT_SESSION_HAS_OPENED, ERR_DH_AVT_CHANNEL_ALREADY_CREATED);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED,
        "create control channel failed");
    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::Start()
{
    AVTRANS_LOGI("Start sender engine enter.");

    bool isErrState = (GetCurrentState() != StateId::CH_CREATED);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_START_FAILED, "current state=%{public}" PRId32 " is invalid.",
        GetCurrentState());

    ErrorCode errCode = pipeline_->Start();
    TRUE_RETURN_V_MSG_E(errCode != ErrorCode::SUCCESS, ERR_DH_AVT_START_FAILED, "start pipeline failed");

    if (dhFwkKit_ != nullptr) {
        int32_t ret = dhFwkKit_->CreateControlChannel(engineId_, peerDevId_);
        TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED,
            "create av control center channel failed");
    }

    SetCurrentState(StateId::STARTED);
    AVTRANS_LOGI("Start sender engine success.");
    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::Stop()
{
    AVTRANS_LOGI("Stop sender engine enter.");
    ErrorCode ret = pipeline_->Stop();
    TRUE_RETURN_V_MSG_E(ret != ErrorCode::SUCCESS, ERR_DH_AVT_STOP_FAILED, "stop pipeline failed");
    SetCurrentState(StateId::STOPPED);
    NotifyStreamChange(EventType::EVENT_REMOVE_STREAM);
    AVTRANS_LOGI("Stop sender engine success.");
    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::Release()
{
    AVTRANS_LOGI("Release sender engine enter.");
    if (pipeline_ != nullptr) {
        pipeline_->Stop();
    }
    if (dhFwkKit_ != nullptr) {
        dhFwkKit_->ReleaseAVCenter(engineId_);
    }
    SoftbusChannelAdapter::GetInstance().CloseSoftbusChannel(sessionName_, peerDevId_);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, peerDevId_);
    isInitialized_ = false;
    pipeline_ = nullptr;
    dhFwkKit_ = nullptr;
    avInput_ = nullptr;
    avOutput_ = nullptr;
    audioEncoder_ = nullptr;
    videoEncoder_ = nullptr;
    senderCallback_ = nullptr;
    ctlCenCallback_ = nullptr;
    SetCurrentState(StateId::IDLE);
    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::SetParameter(AVTransTag tag, const std::string &value)
{
    bool isFilterNull = (avInput_ == nullptr) || (avOutput_ == nullptr) || (pipeline_ == nullptr);
    TRUE_RETURN_V_MSG_E(isFilterNull, ERR_DH_AVT_SETUP_FAILED, "filter or pipeline is null, set parameter failed.");
    auto iter = funcMap_.find(tag);
    if (iter == funcMap_.end()) {
        AVTRANS_LOGE("AVTransTag %{public}u is undefined.", tag);
        return ERR_DH_AVT_INVALID_PARAM;
    }
    SetParaFunc &func = iter->second;
    (this->*func)(value);
    return DH_AVT_SUCCESS;
}

void AVSenderEngine::RegRespFunMap()
{
    funcMap_[AVTransTag::VIDEO_WIDTH] = &AVSenderEngine::SetVideoWidth;
    funcMap_[AVTransTag::VIDEO_HEIGHT] = &AVSenderEngine::SetVideoHeight;
    funcMap_[AVTransTag::VIDEO_PIXEL_FORMAT] = &AVSenderEngine::SetVideoPixelFormat;
    funcMap_[AVTransTag::VIDEO_FRAME_RATE] = &AVSenderEngine::SetVideoFrameRate;
    funcMap_[AVTransTag::AUDIO_BIT_RATE] = &AVSenderEngine::SetAudioBitRate;
    funcMap_[AVTransTag::VIDEO_BIT_RATE] = &AVSenderEngine::SetVideoBitRate;
    funcMap_[AVTransTag::VIDEO_CODEC_TYPE] = &AVSenderEngine::SetVideoCodecType;
    funcMap_[AVTransTag::AUDIO_CODEC_TYPE] = &AVSenderEngine::SetAudioCodecType;
    funcMap_[AVTransTag::AUDIO_CHANNEL_MASK] = &AVSenderEngine::SetAudioChannelMask;
    funcMap_[AVTransTag::AUDIO_SAMPLE_RATE] = &AVSenderEngine::SetAudioSampleRate;
    funcMap_[AVTransTag::AUDIO_CHANNEL_LAYOUT] = &AVSenderEngine::SetAudioChannelLayout;
    funcMap_[AVTransTag::AUDIO_SAMPLE_FORMAT] = &AVSenderEngine::SetAudioSampleFormat;
    funcMap_[AVTransTag::AUDIO_FRAME_SIZE] = &AVSenderEngine::SetAudioFrameSize;
    funcMap_[AVTransTag::SHARED_MEMORY_FD] = &AVSenderEngine::SetSharedMemoryFd;
    funcMap_[AVTransTag::ENGINE_READY] = &AVSenderEngine::SetEngineReady;
    funcMap_[AVTransTag::ENGINE_PAUSE] = &AVSenderEngine::SetEnginePause;
    funcMap_[AVTransTag::ENGINE_RESUME] = &AVSenderEngine::SetEngineResume;
}

void AVSenderEngine::SetVideoWidth(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_WIDTH), std::stoi(value));
    AVTRANS_LOGI("SetParameter VIDEO_WIDTH success, video width = %{public}s", value.c_str());
}

void AVSenderEngine::SetVideoHeight(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_HEIGHT), std::stoi(value));
    AVTRANS_LOGI("SetParameter VIDEO_HEIGHT success, video height = %{public}s", value.c_str());
}

void AVSenderEngine::SetVideoPixelFormat(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_PIXEL_FORMAT), Plugin::VideoPixelFormat::RGBA);
    AVTRANS_LOGI("SetParameter VIDEO_PIXEL_FORMAT success, pixel format = %{public}s", value.c_str());
}

void AVSenderEngine::SetVideoFrameRate(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_FRAME_RATE), std::stoi(value));
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_FRAME_RATE), std::stoi(value));
    AVTRANS_LOGI("SetParameter VIDEO_FRAME_RATE success, frame rate = %{public}s", value.c_str());
}

void AVSenderEngine::SetAudioBitRate(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_BITRATE), std::stoi(value));
    AVTRANS_LOGI("SetParameter MEDIA_BITRATE success, bit rate = %{public}s", value.c_str());
}

void AVSenderEngine::SetVideoBitRate(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_BITRATE), std::stoi(value));
    AVTRANS_LOGI("SetParameter MEDIA_BITRATE success, bit rate = %{public}s", value.c_str());
}

void AVSenderEngine::SetVideoCodecType(const std::string &value)
{
    if (value == MIME_VIDEO_H264) {
        Plugin::Meta encoderMeta;
        encoderMeta.Set<Plugin::Tag::MIME>(MEDIA_MIME_VIDEO_H264);
        encoderMeta.Set<Plugin::Tag::VIDEO_H264_PROFILE>(Plugin::VideoH264Profile::BASELINE);
        encoderMeta.Set<Plugin::Tag::VIDEO_H264_LEVEL>(VIDEO_H264_LEVEL);
        videoEncoder_->SetVideoEncoder(0, std::make_shared<Plugin::Meta>(encoderMeta));

        std::string mime = MEDIA_MIME_VIDEO_RAW;
        avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
        mime = MEDIA_MIME_VIDEO_H264;
        avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
        AVTRANS_LOGI("SetParameter VIDEO_CODEC_TYPE = H264 success");
    } else if (value == MIME_VIDEO_H265) {
        Plugin::Meta encoderMeta;
        encoderMeta.Set<Plugin::Tag::MIME>(MEDIA_MIME_VIDEO_H265);
        videoEncoder_->SetVideoEncoder(0, std::make_shared<Plugin::Meta>(encoderMeta));

        std::string mime = MEDIA_MIME_VIDEO_RAW;
        avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
        mime = MEDIA_MIME_VIDEO_H265;
        avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
        AVTRANS_LOGI("SetParameter VIDEO_CODEC_TYPE = H265 success");
    } else {
        AVTRANS_LOGE("SetParameter VIDEO_CODEC_TYPE failed, input value invalid.");
    }
}

void AVSenderEngine::SetAudioCodecType(const std::string &value)
{
    Plugin::Meta encoderMeta;
    encoderMeta.Set<Plugin::Tag::MIME>(MEDIA_MIME_AUDIO_AAC);
    encoderMeta.Set<Plugin::Tag::AUDIO_AAC_PROFILE>(Plugin::AudioAacProfile::LC);
    audioEncoder_->SetAudioEncoder(0, std::make_shared<Plugin::Meta>(encoderMeta));

    std::string mime = MEDIA_MIME_AUDIO_RAW;
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
    mime = MEDIA_MIME_AUDIO_AAC;
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MIME), mime);
    AVTRANS_LOGI("SetParameter AUDIO_CODEC_TYPE = ACC success");
}

void AVSenderEngine::SetAudioChannelMask(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNELS), std::stoi(value));
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNELS), std::stoi(value));
    AVTRANS_LOGI("SetParameter AUDIO_CHANNELS success, audio channels = %{public}s", value.c_str());
}

void AVSenderEngine::SetAudioSampleRate(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_RATE), std::stoi(value));
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_RATE), std::stoi(value));
    AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_RATE success, audio sample rate = %{public}s", value.c_str());
}

void AVSenderEngine::SetAudioChannelLayout(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNEL_LAYOUT), std::stoi(value));
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_CHANNEL_LAYOUT), std::stoi(value));
    AVTRANS_LOGI("SetParameter AUDIO_CHANNEL_LAYOUT success, audio channel layout = %{public}s", value.c_str());
}

void AVSenderEngine::SetAudioSampleFormat(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_FORMAT), std::stoi(value));
    AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_FORMAT success, audio sample format = %{public}s", value.c_str());
}

void AVSenderEngine::SetAudioFrameSize(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::AUDIO_SAMPLE_PER_FRAME), std::stoi(value));
    AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_PER_FRAME success, audio sample per frame = %{public}s", value.c_str());
}

void AVSenderEngine::SetSharedMemoryFd(const std::string &value)
{
    avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::USER_SHARED_MEMORY_FD), value);
    AVTRANS_LOGI("SetParameter USER_SHARED_MEMORY_FD success, shared memory info = %{public}s", value.c_str());
}

void AVSenderEngine::SetEngineReady(const std::string &value)
{
    int32_t ret = PreparePipeline(value);
    TRUE_LOG_MSG(ret != DH_AVT_SUCCESS, "SetParameter ENGINE_READY failed");
}

void AVSenderEngine::SetEnginePause(const std::string &value)
{
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline is null, need init first.");
        return;
    }

    ErrorCode ret = pipeline_->Pause();
    TRUE_LOG_MSG(ret != ErrorCode::SUCCESS, "pipeline pause failed");
}

void AVSenderEngine::SetEngineResume(const std::string &value)
{
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline is null, need init first.");
        return;
    }

    ErrorCode ret = pipeline_->Resume();
    TRUE_LOG_MSG(ret != ErrorCode::SUCCESS, "pipeline resume failed");
}

int32_t AVSenderEngine::PushData(const std::shared_ptr<AVTransBuffer> &buffer)
{
    StateId currentState = GetCurrentState();
    bool isErrState = (currentState != StateId::STARTED) && (currentState != StateId::PLAYING);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_PUSH_DATA_FAILED,
        "current state=%{public}" PRId32 " is invalid.", currentState);

    if (currentState == StateId::STARTED) {
        NotifyStreamChange(EventType::EVENT_ADD_STREAM);
    }
    TRUE_RETURN_V_MSG_E(avInput_ == nullptr, ERR_DH_AVT_PUSH_DATA_FAILED, "av input filter is null");

    std::shared_ptr<AVBuffer> hisBuffer = TransBuffer2HiSBuffer(buffer);
    TRUE_RETURN_V(hisBuffer == nullptr, ERR_DH_AVT_PUSH_DATA_FAILED);

    ErrorCode ret = avInput_->PushData(avInput_->GetName(), hisBuffer, -1);
    TRUE_RETURN_V(ret != ErrorCode::SUCCESS, ERR_DH_AVT_PUSH_DATA_FAILED);

    SetCurrentState(StateId::PLAYING);
    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::PreparePipeline(const std::string &configParam)
{
    AVTRANS_LOGI("PreparePipeline enter.");

    StateId currentState = GetCurrentState();
    bool isErrState = ((currentState != StateId::INITIALIZED) && (currentState != StateId::CH_CREATED));
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_PREPARE_FAILED,
        "current state=%{public}" PRId32 " is invalid.", currentState);

    TRUE_RETURN_V_MSG_E((avInput_ == nullptr) || (avOutput_ == nullptr), ERR_DH_AVT_PREPARE_FAILED,
        "av input or output filter is null");

    // First: config av input filter
    ErrorCode ret = avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_TYPE),
        TransName2MediaType(ownerName_));
    TRUE_RETURN_V_MSG_E(ret != ErrorCode::SUCCESS, ERR_DH_AVT_SET_PARAM_FAILED, "set media_type failed");

    ret = avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_BIT_STREAM_FORMAT),
        VideoBitStreamFormat::ANNEXB);
    TRUE_RETURN_V_MSG_E(ret != ErrorCode::SUCCESS, ERR_DH_AVT_SET_PARAM_FAILED, "set video_bit_stream_format failed");

    ret = avInput_->SetParameter(static_cast<int32_t>(Plugin::Tag::INPUT_MEMORY_TYPE),
        OHOS::Media::Plugin::MemoryType::VIRTUAL_ADDR);
    TRUE_RETURN_V_MSG_E(ret != ErrorCode::SUCCESS, ERR_DH_AVT_SET_PARAM_FAILED, "set input_memory_type failed");

    // Second: config av output filter
    ret = avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::MEDIA_DESCRIPTION),
        BuildChannelDescription(ownerName_, peerDevId_));
    TRUE_RETURN_V_MSG_E(ret != ErrorCode::SUCCESS, ERR_DH_AVT_SET_PARAM_FAILED, "set media_description failed");

    ret = pipeline_->Prepare();
    TRUE_RETURN_V_MSG_E(ret != ErrorCode::SUCCESS, ERR_DH_AVT_PREPARE_FAILED, "pipeline prepare failed");

    SetCurrentState(StateId::CH_CREATED);
    return DH_AVT_SUCCESS;
}

int32_t AVSenderEngine::SendMessage(const std::shared_ptr<AVTransMessage> &message)
{
    TRUE_RETURN_V_MSG_E(message == nullptr, ERR_DH_AVT_INVALID_PARAM, "input message is nullptr.");
    std::string msgData = message->MarshalMessage();
    return SoftbusChannelAdapter::GetInstance().SendBytesData(sessionName_, message->dstDevId_, msgData);
}

int32_t AVSenderEngine::RegisterSenderCallback(const std::shared_ptr<IAVSenderEngineCallback> &callback)
{
    AVTRANS_LOGI("RegisterSenderCallback enter.");
    TRUE_RETURN_V_MSG_E(callback == nullptr, ERR_DH_AVT_INVALID_PARAM, "input sender engine callback is nullptr.");

    senderCallback_ = callback;
    return DH_AVT_SUCCESS;
}

bool AVSenderEngine::StartDumpMediaData()
{
    if (avOutput_ == nullptr) {
        AVTRANS_LOGE("avOutput_ is nullptr.");
        return false;
    }
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::SECTION_USER_SPECIFIC_START), true);
    return true;
}

bool AVSenderEngine::StopDumpMediaData()
{
    if (avOutput_ == nullptr) {
        AVTRANS_LOGE("avOutput_ is nullptr.");
        return false;
    }
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::SECTION_USER_SPECIFIC_START), false);
    return true;
}

bool AVSenderEngine::ReStartDumpMediaData()
{
    if (avOutput_ == nullptr) {
        AVTRANS_LOGE("avOutput_ is nullptr.");
        return false;
    }
    avOutput_->SetParameter(static_cast<int32_t>(Plugin::Tag::SECTION_VIDEO_SPECIFIC_START), true);
    return true;
}

void AVSenderEngine::NotifyStreamChange(EventType type)
{
    AVTRANS_LOGI("NotifyStreamChange enter, change type=%{public}" PRId32, type);

    std::string sceneType = "";
    if (ownerName_ == OWNER_NAME_D_MIC) {
        sceneType = SCENE_TYPE_D_MIC;
    } else if (ownerName_ == OWNER_NAME_D_SPEAKER) {
        sceneType = SCENE_TYPE_D_SPEAKER;
    } else if (ownerName_ == OWNER_NAME_D_SCREEN) {
        sceneType = SCENE_TYPE_D_SCREEN;
    } else if (ownerName_ == OWNER_NAME_D_CAMERA) {
        TRUE_RETURN(avInput_ == nullptr, "av input filter is null");
        Plugin::Any value;
        avInput_->GetParameter(static_cast<int32_t>(Plugin::Tag::VIDEO_PIXEL_FORMAT), value);
        std::string videoFormat = Plugin::AnyCast<std::string>(value);
        sceneType = (videoFormat == VIDEO_FORMAT_JEPG) ? SCENE_TYPE_D_CAMERA_PIC : SCENE_TYPE_D_CAMERA_STR;
    } else {
        AVTRANS_LOGE("Unknown owner name=%{public}s", ownerName_.c_str());
        return;
    }

    TRUE_RETURN(dhFwkKit_ == nullptr, "dh fwk kit is nullptr.");
    dhFwkKit_->NotifyAVCenter(engineId_, { type, sceneType, peerDevId_ });
}

void AVSenderEngine::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter. event type:%{public}" PRId32, event.type);
    TRUE_RETURN(senderCallback_ == nullptr, "sender callback is nullptr");

    switch (event.type) {
        case EventType::EVENT_CHANNEL_OPENED: {
            SetCurrentState(StateId::CH_CREATED);
            senderCallback_->OnSenderEvent(event);
            break;
        }
        case EventType::EVENT_CHANNEL_OPEN_FAIL: {
            SetCurrentState(StateId::INITIALIZED);
            senderCallback_->OnSenderEvent(event);
            break;
        }
        case EventType::EVENT_CHANNEL_CLOSED: {
            StateId currentState = GetCurrentState();
            if ((currentState != StateId::IDLE) && (currentState != StateId::INITIALIZED)) {
                SetCurrentState(StateId::INITIALIZED);
                senderCallback_->OnSenderEvent(event);
            }
            break;
        }
        case EventType::EVENT_DATA_RECEIVED: {
            auto avMessage = std::make_shared<AVTransMessage>();
            TRUE_RETURN(!avMessage->UnmarshalMessage(event.content, event.peerDevId), "unmarshal message failed");
            senderCallback_->OnMessageReceived(avMessage);
            break;
        }
        default:
            AVTRANS_LOGE("Invalid event type.");
    }
}

void AVSenderEngine::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

void AVSenderEngine::OnEvent(const Event &event)
{
    AVTRANS_LOGI("OnEvent enter. event type:%{public}s", GetEventName(event.type));
    TRUE_RETURN(senderCallback_ == nullptr, "sender callback is nullptr");

    switch (event.type) {
        case OHOS::Media::EventType::EVENT_PLUGIN_EVENT: {
            Plugin::PluginEvent plugEvt = Plugin::AnyCast<Plugin::PluginEvent>(event.param);
            bool isPlaying = (GetCurrentState() == StateId::PLAYING);
            senderCallback_->OnSenderEvent(AVTransEvent{CastEventType(plugEvt.type, isPlaying), "", peerDevId_});
            break;
        }
        default:
            AVTRANS_LOGE("Invalid event type.");
    }
}
} // namespace DistributedHardware
} // namespace OHOS