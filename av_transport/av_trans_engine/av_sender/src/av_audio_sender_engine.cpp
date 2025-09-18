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

#include "av_audio_sender_engine.h"

#include <charconv>

namespace OHOS {
namespace DistributedHardware {

#undef DH_LOG_TAG
#define DH_LOG_TAG "AVAudioSenderEngine"
class PlayerEventCbSen : public Pipeline::EventReceiver {
public:
    explicit PlayerEventCbSen(std::shared_ptr<AVAudioSenderEngine> sender, std::string playerId)
    {
        AVTRANS_LOGI("PlayerEventCbSen ctor called.");
        sender_ = sender;
    }

    void OnEvent(const Pipeline::Event &event) override
    {
        AVTRANS_LOGD("PlayerEventCbSen OnEvent.");
        if (auto sender = sender_.lock()) {
            sender->OnEvent(event);
        } else {
            AVTRANS_LOGI("invalid sender_");
        }
    }

private:
    std::weak_ptr<AVAudioSenderEngine> sender_;
};

class PlayerFilterCbSen : public Pipeline::FilterCallback {
public:
    explicit PlayerFilterCbSen(std::shared_ptr<AVAudioSenderEngine> sender) :sender_(sender)
    {
        AVTRANS_LOGI("PlayerFilterCbSen ctor called.");
        sender_ = sender;
    }

    Status OnCallback(const std::shared_ptr<Pipeline::Filter>& filter, Pipeline::FilterCallBackCommand cmd,
        Pipeline::StreamType outType) override
    {
        AVTRANS_LOGD("PlayerFilterCbSen OnCallback.");
        if (auto lockedSender_ = sender_.lock()) {
            return lockedSender_->OnCallback(filter, cmd, outType);
        } else {
            AVTRANS_LOGI("invalid receiver_");
            return Status::ERROR_NULL_POINTER;
        }
    }

private:
    std::weak_ptr<AVAudioSenderEngine> sender_;
};

AVAudioSenderEngine::AVAudioSenderEngine(const std::string &ownerName, const std::string &peerDevId)
    : ownerName_(ownerName), peerDevId_(peerDevId)
{
    AVTRANS_LOGI("AVAudioSenderEngine ctor().");
    sessionName_ = ownerName_ + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX;
}

AVAudioSenderEngine::~AVAudioSenderEngine()
{
    AVTRANS_LOGI("AVAudioSenderEngine dctor().");
    Release();

    dhFwkKit_ = nullptr;
    pipeline_ = nullptr;
    avInput_ = nullptr;
    encoderFilter_ = nullptr;
    avOutput_ = nullptr;
    senderCallback_ = nullptr;
    ctlCenCallback_ = nullptr;
}

int32_t AVAudioSenderEngine::Initialize()
{
    TRUE_RETURN_V_MSG_E(isInitialized_.load(), DH_AVT_SUCCESS, "sender engine has been initialized");

    int32_t ret = InitPipeline();
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init pipeline failed");

    int32_t retCenter = InitControlCenter();
    TRUE_RETURN_V_MSG_E(retCenter != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init av control center failed");

    RegRespFunMap();
    isInitialized_ = true;
    SetCurrentState(StateId::INITIALIZED);
    return DH_AVT_SUCCESS;
}

int32_t AVAudioSenderEngine::InitPipeline()
{
    AVTRANS_LOGI("InitPipeline enter.");
    senderId_ = std::string("AVAudioSenderEngine") + std::to_string(Pipeline::Pipeline::GetNextPipelineId());
    playEventReceiver_ = std::make_shared<PlayerEventCbSen>(shared_from_this(), senderId_);
    playFilterCallback_ = std::make_shared<PlayerFilterCbSen>(shared_from_this());
    pipeline_ = std::make_shared<Pipeline::Pipeline>();
    pipeline_->Init(playEventReceiver_, playFilterCallback_, senderId_);

    avInput_ = std::make_shared<Pipeline::AVTransAudioInputFilter>("builtin.avtrans.audio.input",
        Pipeline::FilterType::FILTERTYPE_SOURCE);
    TRUE_RETURN_V_MSG_E(avInput_ == nullptr, ERR_DH_AVT_NULL_POINTER, "create av input filter failed");
    avInput_->Init(playEventReceiver_, playFilterCallback_);
    meta_ = std::make_shared<Media::Meta>();

    Status ret = pipeline_->AddHeadFilters({avInput_});
    if (ret == Status::OK) {
        AVTRANS_LOGI("Add head filters success.");
        return DH_AVT_SUCCESS;
    } else {
        AVTRANS_LOGI("add head filter failed.");
        return ERR_DH_AVT_INVALID_PARAM_VALUE;
    }
}

int32_t AVAudioSenderEngine::InitControlCenter()
{
    dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = dhFwkKit_->InitializeAVCenter(TransRole::AV_SENDER, engineId_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CTRL_CENTER_INIT_FAIL, "init av trans control center failed");

    ctlCenCallback_ = sptr<AVTransControlCenterCallback>(new (std::nothrow) AVTransControlCenterCallback());
    TRUE_RETURN_V_MSG_E(ctlCenCallback_ == nullptr, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "new control center callback failed");

    std::shared_ptr<IAVSenderEngine> engine = std::shared_ptr<AVAudioSenderEngine>(shared_from_this());
    ctlCenCallback_->SetSenderEngine(engine);

    ret = dhFwkKit_->RegisterCtlCenterCallback(engineId_, ctlCenCallback_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "register control center callback failed");

    return DH_AVT_SUCCESS;
}

int32_t AVAudioSenderEngine::CreateControlChannel(const std::vector<std::string> &dstDevIds,
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

int32_t AVAudioSenderEngine::PreparePipeline(const std::string &configParam)
{
    AVTRANS_LOGI("PreparePipeline enter.");

    StateId currentState = GetCurrentState();
    bool isErrState = ((currentState != StateId::INITIALIZED) && (currentState != StateId::CH_CREATED));
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_PREPARE_FAILED,
        "current state=%{public}" PRId32 " is invalid.", currentState);

    cJSON *jsonObj = cJSON_CreateObject();
    if (jsonObj == nullptr) {
        return ERR_DH_AVT_NULL_POINTER;
    }
    cJSON_AddStringToObject(jsonObj, KEY_ONWER_NAME.c_str(), ownerName_.c_str());
    cJSON_AddStringToObject(jsonObj, KEY_PEERDEVID_NAME.c_str(), peerDevId_.c_str());
    auto str = cJSON_PrintUnformatted(jsonObj);
    if (str == nullptr) {
        cJSON_Delete(jsonObj);
        return ERR_DH_AVT_NULL_POINTER;
    }
    std::string jsonStr = std::string(str);
    cJSON_free(str);
    cJSON_Delete(jsonObj);
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return ERR_DH_AVT_NULL_POINTER;
    }
    meta_->SetData(Media::Tag::MEDIA_DESCRIPTION, jsonStr);
    BufferDataType dataType = BufferDataType::AUDIO;
    meta_->SetData(Media::Tag::MEDIA_STREAM_TYPE, dataType);
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline is nullptr.");
        return ERR_DH_AVT_SET_PARAM_FAILED;
    }
    auto ret = Prepare();
    TRUE_RETURN_V(ret != Status::OK, ERR_DH_AVT_PREPARE_FAILED);

    SetCurrentState(StateId::CH_CREATED);
    return DH_AVT_SUCCESS;
}

int32_t AVAudioSenderEngine::Start()
{
    AVTRANS_LOGI("Start enter.");

    bool isErrState = (GetCurrentState() != StateId::CH_CREATED);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_START_FAILED, "current state=%{public}" PRId32 " is invalid.",
        GetCurrentState());

    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline_ is nullptr.");
        return ERR_DH_AVT_START_FAILED;
    }
    Status errCode = pipeline_->Start();
    TRUE_RETURN_V_MSG_E(errCode != Status::OK, ERR_DH_AVT_START_FAILED, "start pipeline failed");
    TRUE_RETURN_V_MSG_E(dhFwkKit_ == nullptr, ERR_DH_AVT_START_FAILED, "dhFwkKit is nullptr");
    int32_t ret = dhFwkKit_->CreateControlChannel(engineId_, peerDevId_);
        TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED,
            "create av control center channel failed");

    SetCurrentState(StateId::STARTED);
    AVTRANS_LOGI("Start sender engine success.");
    return DH_AVT_SUCCESS;
}

int32_t AVAudioSenderEngine::Stop()
{
    AVTRANS_LOGI("Stop sender engine enter.");
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline_ is nullptr.");
        return ERR_DH_AVT_STOP_FAILED;
    }
    Status ret = pipeline_->Stop();
    TRUE_RETURN_V_MSG_E(ret != Status::OK, ERR_DH_AVT_STOP_FAILED, "stop pipeline failed");
    SetCurrentState(StateId::STOPPED);
    NotifyStreamChange(EventType::EVENT_REMOVE_STREAM);
    AVTRANS_LOGI("Stop sender engine success.");
    return DH_AVT_SUCCESS;
}

int32_t AVAudioSenderEngine::Release()
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
    senderCallback_ = nullptr;
    ctlCenCallback_ = nullptr;
    SetCurrentState(StateId::IDLE);
    return DH_AVT_SUCCESS;
}

void AVAudioSenderEngine::SetParameterInner(AVTransTag tag, const std::string &value)
{
    switch (tag) {
        case AVTransTag::VIDEO_CODEC_TYPE:
            SetVideoCodecType(value);
            break;
        case AVTransTag::AUDIO_CODEC_TYPE:
            SetAudioCodecType(value);
            break;
        case AVTransTag::AUDIO_CHANNEL_MASK:
            SetAudioChannelMask(value);
            break;
        case AVTransTag::AUDIO_SAMPLE_RATE:
            SetAudioSampleRate(value);
            break;
        case AVTransTag::AUDIO_CHANNEL_LAYOUT:
            SetAudioChannelLayout(value);
            break;
        case AVTransTag::AUDIO_SAMPLE_FORMAT:
            SetAudioSampleFormat(value);
            break;
        case AVTransTag::AUDIO_FRAME_SIZE:
            SetAudioFrameSize(value);
            break;
        case AVTransTag::SHARED_MEMORY_FD:
            SetSharedMemoryFd(value);
            break;
        case AVTransTag::ENGINE_READY:
            SetEngineReady(value);
            break;
        case AVTransTag::ENGINE_PAUSE:
            SetEnginePause(value);
            break;
        case AVTransTag::ENGINE_RESUME:
            SetEngineResume(value);
            break;
        default:
            break;
    }
}

int32_t AVAudioSenderEngine::SetParameter(AVTransTag tag, const std::string &value)
{
    AVTRANS_LOGI("AVTransTag=%{public}u.", tag);
    switch (tag) {
        case AVTransTag::VIDEO_WIDTH:
            SetVideoWidth(value);
            break;
        case AVTransTag::VIDEO_HEIGHT:
            SetVideoHeight(value);
            break;
        case AVTransTag::VIDEO_PIXEL_FORMAT:
            SetVideoPixelFormat(value);
            break;
        case AVTransTag::VIDEO_FRAME_RATE:
            SetVideoFrameRate(value);
            break;
        case AVTransTag::AUDIO_BIT_RATE:
            SetAudioBitRate(value);
            break;
        case AVTransTag::VIDEO_BIT_RATE:
            SetVideoBitRate(value);
            break;
        case AVTransTag::VIDEO_CODEC_TYPE:
        case AVTransTag::AUDIO_CODEC_TYPE:
        case AVTransTag::AUDIO_CHANNEL_MASK:
        case AVTransTag::AUDIO_SAMPLE_RATE:
        case AVTransTag::AUDIO_CHANNEL_LAYOUT:
        case AVTransTag::AUDIO_SAMPLE_FORMAT:
        case AVTransTag::AUDIO_FRAME_SIZE:
        case AVTransTag::SHARED_MEMORY_FD:
        case AVTransTag::ENGINE_READY:
        case AVTransTag::ENGINE_PAUSE:
        case AVTransTag::ENGINE_RESUME:
            SetParameterInner(tag, value);
            break;
        default:
            AVTRANS_LOGE("AVTransTag %{public}u is undefined.", tag);
            return ERR_DH_AVT_INVALID_PARAM;
    }
    return DH_AVT_SUCCESS;
}

bool AVAudioSenderEngine::ConvertToInt(const std::string& str, int& value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

void AVAudioSenderEngine::RegRespFunMap()
{
    funcMap_[AVTransTag::VIDEO_WIDTH] = &AVAudioSenderEngine::SetVideoWidth;
    funcMap_[AVTransTag::VIDEO_HEIGHT] = &AVAudioSenderEngine::SetVideoHeight;
    funcMap_[AVTransTag::VIDEO_PIXEL_FORMAT] = &AVAudioSenderEngine::SetVideoPixelFormat;
    funcMap_[AVTransTag::VIDEO_FRAME_RATE] = &AVAudioSenderEngine::SetVideoFrameRate;
    funcMap_[AVTransTag::AUDIO_BIT_RATE] = &AVAudioSenderEngine::SetAudioBitRate;
    funcMap_[AVTransTag::VIDEO_BIT_RATE] = &AVAudioSenderEngine::SetVideoBitRate;
    funcMap_[AVTransTag::VIDEO_CODEC_TYPE] = &AVAudioSenderEngine::SetVideoCodecType;
    funcMap_[AVTransTag::AUDIO_CODEC_TYPE] = &AVAudioSenderEngine::SetAudioCodecType;
    funcMap_[AVTransTag::AUDIO_CHANNEL_MASK] = &AVAudioSenderEngine::SetAudioChannelMask;
    funcMap_[AVTransTag::AUDIO_SAMPLE_RATE] = &AVAudioSenderEngine::SetAudioSampleRate;
    funcMap_[AVTransTag::AUDIO_CHANNEL_LAYOUT] = &AVAudioSenderEngine::SetAudioChannelLayout;
    funcMap_[AVTransTag::AUDIO_SAMPLE_FORMAT] = &AVAudioSenderEngine::SetAudioSampleFormat;
    funcMap_[AVTransTag::AUDIO_FRAME_SIZE] = &AVAudioSenderEngine::SetAudioFrameSize;
    funcMap_[AVTransTag::SHARED_MEMORY_FD] = &AVAudioSenderEngine::SetSharedMemoryFd;
    funcMap_[AVTransTag::ENGINE_READY] = &AVAudioSenderEngine::SetEngineReady;
    funcMap_[AVTransTag::ENGINE_PAUSE] = &AVAudioSenderEngine::SetEnginePause;
    funcMap_[AVTransTag::ENGINE_RESUME] = &AVAudioSenderEngine::SetEngineResume;
}

void AVAudioSenderEngine::SetVideoWidth(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::VIDEO_WIDTH, intValue);
        AVTRANS_LOGI("SetParameter VIDEO_WIDTH success, video width = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter VIDEO_WIDTH failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetVideoHeight(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::VIDEO_HEIGHT, intValue);
        AVTRANS_LOGI("SetParameter VIDEO_HEIGHT success, video height = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter VIDEO_HEIGHT failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetVideoPixelFormat(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("avInput_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::VIDEO_PIXEL_FORMAT, intValue);
        AVTRANS_LOGI("SetParameter VIDEO_PIXEL_FORMAT success, pixel format = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter VIDEO_PIXEL_FORMAT failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetVideoFrameRate(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::VIDEO_FRAME_RATE, intValue);
        AVTRANS_LOGI("SetParameter VIDEO_FRAME_RATE success, frame rate = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter VIDEO_FRAME_RATE failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetAudioBitRate(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::MEDIA_BITRATE, intValue);
        AVTRANS_LOGI("SetParameter MEDIA_BITRATE success, bit rate = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter MEDIA_BITRATE failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetVideoBitRate(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::MEDIA_BITRATE, intValue);
        AVTRANS_LOGI("SetParameter MEDIA_BITRATE success, bit rate = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter MEDIA_BITRATE failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetVideoCodecType(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    meta_->SetData(Media::Tag::MIME_TYPE, value);
}

void AVAudioSenderEngine::SetAudioCodecType(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::MIME_TYPE, intValue);
    } else {
        AVTRANS_LOGI("SetParameter MEDIA_BITRATE failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetAudioChannelMask(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::AUDIO_CHANNEL_COUNT, intValue);
        AVTRANS_LOGI("SetParameter AUDIO_CHANNELS success, audio channels = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter AUDIO_CHANNEL_COUNT failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetAudioSampleRate(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::AUDIO_SAMPLE_RATE, intValue);
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_RATE success, audio sample = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_RATE failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetAudioChannelLayout(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("avInput_ or avOutput_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::AUDIO_CHANNEL_LAYOUT, intValue);
        AVTRANS_LOGI("SetParameter AUDIO_CHANNEL_LAYOUT success, audio channel layout = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter AUDIO_CHANNEL_LAYOUT failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetAudioSampleFormat(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        auto sampleFormat = static_cast<MediaAVCodec::AudioSampleFormat>(intValue);
        meta_->SetData(Media::Tag::AUDIO_SAMPLE_FORMAT, sampleFormat);
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_FORMAT success, audio sample format = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_FORMAT failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetAudioFrameSize(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::AUDIO_SAMPLE_PER_FRAME, intValue);
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_PER_FRAME success, audio sample per frame = %{public}s",
            value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_PER_FRAME failed, value conversion failed.");
    }
}

void AVAudioSenderEngine::SetSharedMemoryFd(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    meta_->SetData(Media::Tag::USER_SHARED_MEMORY_FD, value);
    AVTRANS_LOGI("SetParameter USER_SHARED_MEMORY_FD success, shared memory info = %{public}s", value.c_str());
}

void AVAudioSenderEngine::SetEngineReady(const std::string &value)
{
    int32_t ret = PreparePipeline(value);
    TRUE_LOG_MSG(ret != DH_AVT_SUCCESS, "SetParameter ENGINE_READY failed");
}

void AVAudioSenderEngine::SetEnginePause(const std::string &value)
{
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline is null, need init first.");
        return;
    }

    Status ret = pipeline_->Pause();
    TRUE_LOG_MSG(ret != Status::OK, "pipeline pause failed");
}

void AVAudioSenderEngine::SetEngineResume(const std::string &value)
{
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline is null, need init first.");
        return;
    }

    Status ret = pipeline_->Resume();
    TRUE_LOG_MSG(ret != Status::OK, "pipeline resume failed");
}

int32_t AVAudioSenderEngine::PushData(const std::shared_ptr<AVTransBuffer> &buffer)
{
    TRUE_RETURN_V_MSG_E(buffer == nullptr, ERR_DH_AVT_PUSH_DATA_FAILED, "av trans buffer is null");
    StateId currentState = GetCurrentState();
    bool isErrState = (currentState != StateId::STARTED) && (currentState != StateId::PLAYING);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_PUSH_DATA_FAILED,
        "current state=%{public}" PRId32 " is invalid.", currentState);

    if (currentState == StateId::STARTED) {
        NotifyStreamChange(EventType::EVENT_ADD_STREAM);
    }
    TRUE_RETURN_V_MSG_E(avInput_ == nullptr, ERR_DH_AVT_PUSH_DATA_FAILED, "av input filter is null");
    sptr<Media::AVBufferQueueProducer> producer = avInput_->GetInputBufQueProducer();
    auto data = buffer->GetBufferData();
    if (data == nullptr) {
        return ERR_DH_AVT_PUSH_DATA_FAILED;
    }
    Media::AVBufferConfig config;
    auto bufferSize = static_cast<int32_t>(data->GetSize());
    config.size = bufferSize;
    config.memoryType = Media::MemoryType::VIRTUAL_MEMORY;
    config.memoryFlag = Media::MemoryFlag::MEMORY_READ_WRITE;
    std::shared_ptr<Media::AVBuffer> outBuffer = nullptr;
    producer->RequestBuffer(outBuffer, config, TIME_OUT_MS);
    TRUE_RETURN_V_MSG_E(outBuffer == nullptr, ERR_DH_AVT_PUSH_DATA_FAILED, "RequestBuffer fail");
    auto meta = outBuffer->meta_;
    if (meta == nullptr) {
        AVTRANS_LOGE("outBuffer->meta_ is null");
        producer->PushBuffer(outBuffer, true);
        return ERR_DH_AVT_PREPARE_FAILED;
    }
    outBuffer->memory_->Write(data->GetAddress(), bufferSize, 0);
    outBuffer->pts_ = buffer->GetPts();
    AVTRANS_LOGI("buffer->GetPts(): %{public}" PRId64, buffer->GetPts());
    producer->PushBuffer(outBuffer, true);
    SetCurrentState(StateId::PLAYING);
    return DH_AVT_SUCCESS;
}

Status AVAudioSenderEngine::Prepare()
{
    if (avInput_ == nullptr) {
        AVTRANS_LOGE("avInput_ is nullptr");
        return Status::ERROR_INVALID_OPERATION;
    }
    avInput_->SetParameter(meta_);
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("Pipeline_ is nullptr");
        return Status::ERROR_INVALID_OPERATION;
    }
    Status ret = pipeline_->Prepare();
    TRUE_RETURN_V_MSG_E(ret != Status::OK, Status::ERROR_INVALID_OPERATION, "pipeline prepare failed");
    return Status::OK;
}

int32_t AVAudioSenderEngine::SendMessage(const std::shared_ptr<AVTransMessage> &message)
{
    TRUE_RETURN_V_MSG_E(message == nullptr, ERR_DH_AVT_INVALID_PARAM, "input message is nullptr.");
    std::string msgData = message->MarshalMessage();
    return SoftbusChannelAdapter::GetInstance().SendBytesData(sessionName_, message->dstDevId_, msgData);
}

int32_t AVAudioSenderEngine::RegisterSenderCallback(const std::shared_ptr<IAVSenderEngineCallback> &callback)
{
    AVTRANS_LOGI("RegisterSenderCallback enter.");
    TRUE_RETURN_V_MSG_E(callback == nullptr, ERR_DH_AVT_INVALID_PARAM, "input sender engine callback is nullptr.");

    senderCallback_ = callback;
    return DH_AVT_SUCCESS;
}

bool AVAudioSenderEngine::StartDumpMediaData()
{
    return true;
}

bool AVAudioSenderEngine::StopDumpMediaData()
{
    return true;
}

bool AVAudioSenderEngine::ReStartDumpMediaData()
{
    return true;
}

void AVAudioSenderEngine::NotifyStreamChange(EventType type)
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
    } else {
        AVTRANS_LOGE("Unknown owner name=%{public}s", ownerName_.c_str());
        return;
    }

    TRUE_RETURN(dhFwkKit_ == nullptr, "dh fwk kit is nullptr.");
    dhFwkKit_->NotifyAVCenter(engineId_, { type, sceneType, peerDevId_ });
}

void AVAudioSenderEngine::OnEvent(const Pipeline::Event &event)
{
    switch (event.type) {
        case Pipeline::EventType::EVENT_AUDIO_PROGRESS: {
            auto channelEvent = Media::AnyCast<AVTransEvent>(event.param);
            OnChannelEvent(channelEvent);
            break;
        }
        default:
            AVTRANS_LOGE("Invalid event type.");
    }
}

void AVAudioSenderEngine::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter. event type:%{public}" PRId32, event.type);
    TRUE_RETURN(senderCallback_ == nullptr, "sender callback is nullptr");

    switch (event.type) {
        case EventType::EVENT_CHANNEL_OPENED: {
            if (GetCurrentState() != StateId::STARTED) {
                SetCurrentState(StateId::CH_CREATED);
            }
            AVTransEvent eventSend;
            eventSend.type = EventType::EVENT_START_SUCCESS;
            senderCallback_->OnSenderEvent(eventSend);
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

void AVAudioSenderEngine::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

Status AVAudioSenderEngine::OnCallback(std::shared_ptr<Pipeline::Filter> filter,
    const Pipeline::FilterCallBackCommand cmd, Pipeline::StreamType outType)
{
    AVTRANS_LOGI("AVAudioSenderEngine::OnCallback filter, outType: %{public}d", outType);
    if (cmd == Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED) {
        switch (outType) {
            case Pipeline::StreamType::STREAMTYPE_RAW_AUDIO:
                return LinkAudioSinkFilter(filter, outType);
            case Pipeline::StreamType::STREAMTYPE_ENCODED_AUDIO:
                return LinkAudioEncoderFilter(filter, outType);
            default:
                break;
        }
    }
    return Status::OK;
}

Status AVAudioSenderEngine::LinkAudioEncoderFilter(const std::shared_ptr<Pipeline::Filter>& preFilter,
    Pipeline::StreamType type)
{
    AVTRANS_LOGI("AVAudioSenderEngine::LinkAudioDecoderFilter");
    TRUE_RETURN_V(encoderFilter_ != nullptr, Status::OK);
    encoderFilter_ = std::make_shared<Pipeline::AudioEncoderFilter>("builtin.recorder.EncoderFilter",
        Pipeline::FilterType::FILTERTYPE_AENC);
    TRUE_RETURN_V(encoderFilter_ == nullptr, Status::ERROR_NULL_POINTER);
    encoderFilter_->Init(playEventReceiver_, playFilterCallback_);

    return pipeline_->LinkFilters(preFilter, {encoderFilter_}, type);
}

Status AVAudioSenderEngine::LinkAudioSinkFilter(const std::shared_ptr<Pipeline::Filter>& preFilter,
    Pipeline::StreamType type)
{
    AVTRANS_LOGI("AVAudioSenderEngine::LinkAudioDecoderFilter");
    TRUE_RETURN_V(avOutput_ != nullptr, Status::OK);
    avOutput_ = std::make_shared<Pipeline::DSoftbusOutputFilter>("builtin.avtransport.avoutput",
        Pipeline::FilterType::FILTERTYPE_ASINK);
    TRUE_RETURN_V(avOutput_ == nullptr, Status::ERROR_NULL_POINTER);
    avOutput_->Init(playEventReceiver_, playFilterCallback_);
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("Pipeline_ is nullptr");
        return Status::ERROR_INVALID_OPERATION;
    }
    return pipeline_->LinkFilters(preFilter, {avOutput_}, type);
}
} // namespace DistributedHardware
} // namespace OHOS