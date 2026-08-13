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

#include "av_audio_receiver_engine.h"

#include <charconv>

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AVAudioReceiverEngine"
class PlayerEventCbRec : public Pipeline::EventReceiver {
public:
    explicit PlayerEventCbRec(std::shared_ptr<AVAudioReceiverEngine> receiver, std::string playerId)
    {
        AVTRANS_LOGI("PlayerEventCbRec ctor called.");
        receiver_ = receiver;
    }

    void OnEvent(const Pipeline::Event &event) override
    {
        AVTRANS_LOGD("PlayerEventCbRec OnEvent.");
        if (auto receiver = receiver_.lock()) {
            receiver->OnEvent(event);
        } else {
            AVTRANS_LOGI("invalid receiver_");
        }
    }

private:
    std::weak_ptr<AVAudioReceiverEngine> receiver_;
};

class PlayerFilterCbRec : public Pipeline::FilterCallback {
public:
    explicit PlayerFilterCbRec(std::shared_ptr<AVAudioReceiverEngine> receiver)
    {
        AVTRANS_LOGI("PlayerFilterCbRec ctor called.");
        receiver_ = receiver;
    }

    Status OnCallback(const std::shared_ptr<Pipeline::Filter>& filter, Pipeline::FilterCallBackCommand cmd,
        Pipeline::StreamType outType) override
    {
        AVTRANS_LOGD("PlayerFilterCbRec OnCallback.");
        if (auto receiver = receiver_.lock()) {
            return receiver->OnCallback(filter, cmd, outType);
        } else {
            AVTRANS_LOGI("invalid receiver_");
            return Status::ERROR_NULL_POINTER;
        }
    }

private:
    std::weak_ptr<AVAudioReceiverEngine> receiver_;
};

AVAudioReceiverEngine::AVAudioReceiverEngine(const std::string &ownerName, const std::string &peerDevId)
    : ownerName_(ownerName), peerDevId_(peerDevId)
{
    AVTRANS_LOGI("AVAudioReceiverEngine ctor.");
    sessionName_ = ownerName_ + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX;
}

AVAudioReceiverEngine::~AVAudioReceiverEngine()
{
    AVTRANS_LOGI("AVAudioReceiverEngine dctor.");
    Release();

    dhFwkKit_ = nullptr;
    pipeline_ = nullptr;
    avInput_ = nullptr;
    avOutput_ = nullptr;
    audioDecoder_ = nullptr;
    ctlCtrCallback_ = nullptr;
}

int32_t AVAudioReceiverEngine::Initialize()
{
    TRUE_RETURN_V_MSG_E(isInitialized_.load(), DH_AVT_SUCCESS, "receiver engine has been initialized");

    int32_t ret = InitPipeline();
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init pipeline failed");

    ret = InitControlCenter();
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "init av control center failed");

    ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_INIT_FAILED, "register receiver channel callback failed");
    RegRespFunMap();
    isInitialized_ = true;
    SetCurrentState(StateId::INITIALIZED);
    return DH_AVT_SUCCESS;
}

int32_t AVAudioReceiverEngine::InitPipeline()
{
    AVTRANS_LOGI("InitPipeline enter.");
    receiverId_ = std::string("AVreceiverEngine") + std::to_string(Pipeline::Pipeline::GetNextPipelineId());
    playEventReceiver_ = std::make_shared<PlayerEventCbRec>(shared_from_this(), receiverId_);
    playFilterCallback_ = std::make_shared<PlayerFilterCbRec>(shared_from_this());
    pipeline_ = std::make_shared<Pipeline::Pipeline>();
    pipeline_->Init(playEventReceiver_, playFilterCallback_, receiverId_);

    avInput_ = std::make_shared<Pipeline::AVTransBusInputFilter>("builtin.avtrans.softbus.input",
        Pipeline::FilterType::AUDIO_DATA_SOURCE);
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

int32_t AVAudioReceiverEngine::InitControlCenter()
{
    dhFwkKit_ = std::make_shared<DistributedHardwareFwkKit>();
    int32_t ret = dhFwkKit_->InitializeAVCenter(TransRole::AV_RECEIVER, engineId_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CTRL_CENTER_INIT_FAIL, "init av trans control center failed");

    ctlCtrCallback_ = sptr<AVTransControlCenterCallback>(new (std::nothrow) AVTransControlCenterCallback());
    TRUE_RETURN_V_MSG_E(ctlCtrCallback_ == nullptr, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "new control center callback failed");

    std::shared_ptr<IAVReceiverEngine> engine = std::shared_ptr<AVAudioReceiverEngine>(shared_from_this());
    ctlCtrCallback_->SetReceiverEngine(engine);

    ret = dhFwkKit_->RegisterCtlCenterCallback(engineId_, ctlCtrCallback_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_REGISTER_CALLBACK_FAIL,
        "register control center callback failed");

    return DH_AVT_SUCCESS;
}

int32_t AVAudioReceiverEngine::CreateControlChannel(const std::vector<std::string> &dstDevIds,
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
    TRUE_RETURN_V(ret == ERR_DH_AVT_SESSION_HAS_OPENED, ERR_DH_AVT_CHANNEL_ALREADY_CREATED);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, ERR_DH_AVT_CREATE_CHANNEL_FAILED,
        "create receiver control channel failed");
    return DH_AVT_SUCCESS;
}

int32_t AVAudioReceiverEngine::PreparePipeline(const std::string &configParam)
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
    auto ret = Prepare();
    TRUE_RETURN_V(ret != Status::OK, ERR_DH_AVT_PREPARE_FAILED);

    SetCurrentState(StateId::CH_CREATED);
    return DH_AVT_SUCCESS;
}

int32_t AVAudioReceiverEngine::Start()
{
    AVTRANS_LOGI("Start enter.");

    bool isErrState = (GetCurrentState() != StateId::CH_CREATED);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_START_FAILED, "current state=%{public}" PRId32 " is invalid.",
        GetCurrentState());

    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline is nullptr.");
        return ERR_DH_AVT_START_FAILED;
    }
    auto ret = pipeline_->Start();
    TRUE_RETURN_V(ret != Status::OK, ERR_DH_AVT_START_FAILED);
    SetCurrentState(StateId::STARTED);
    return DH_AVT_SUCCESS;
}

int32_t AVAudioReceiverEngine::Stop()
{
    AVTRANS_LOGI("Stop enter.");
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("pipeline is nullptr.");
        return ERR_DH_AVT_START_FAILED;
    }
    auto ret = pipeline_->Stop();
    TRUE_RETURN_V(ret != Status::OK, ERR_DH_AVT_STOP_FAILED);
    SetCurrentState(StateId::STOPPED);
    return DH_AVT_SUCCESS;
}

int32_t AVAudioReceiverEngine::Release()
{
    AVTRANS_LOGI("Release enter.");
    TRUE_RETURN_V(GetCurrentState() == StateId::IDLE, DH_AVT_SUCCESS);
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
    ctlCtrCallback_ = nullptr;
    SetCurrentState(StateId::IDLE);
    return DH_AVT_SUCCESS;
}

bool AVAudioReceiverEngine::ConvertToInt(const std::string& str, int& value)
{
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value);
    return ec == std::errc{} && ptr == str.data() + str.size();
}

void AVAudioReceiverEngine::SetParameterInner(AVTransTag tag, const std::string &value)
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
        case AVTransTag::TIME_SYNC_RESULT:
            SetSyncResult(value);
            break;
        case AVTransTag::START_AV_SYNC:
            SetStartAvSync(value);
            break;
        case AVTransTag::STOP_AV_SYNC:
            SetStopAvSync(value);
            break;
        case AVTransTag::SHARED_MEMORY_FD:
            SetSharedMemoryFd(value);
            break;
        case AVTransTag::ENGINE_READY:
            SetEngineReady(value);
            break;
        default:
            break;
    }
}

int32_t AVAudioReceiverEngine::SetParameter(AVTransTag tag, const std::string &value)
{
    AVTRANS_LOGI("AVTransTag=%{public}u.", tag);
    switch (tag) {
        case AVTransTag::VIDEO_WIDTH:
            SetVideoWidth(value);
            break;
        case AVTransTag::VIDEO_HEIGHT:
            SetVideoHeight(value);
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
        case AVTransTag::TIME_SYNC_RESULT:
        case AVTransTag::START_AV_SYNC:
        case AVTransTag::STOP_AV_SYNC:
        case AVTransTag::SHARED_MEMORY_FD:
        case AVTransTag::ENGINE_READY:
            SetParameterInner(tag, value);
            break;
        default:
            AVTRANS_LOGE("AVTransTag %{public}u is undefined.", tag);
            return ERR_DH_AVT_INVALID_PARAM;
    }
    return DH_AVT_SUCCESS;
}

void AVAudioReceiverEngine::RegRespFunMap()
{
    funcMap_[AVTransTag::VIDEO_WIDTH] = &AVAudioReceiverEngine::SetVideoWidth;
    funcMap_[AVTransTag::VIDEO_HEIGHT] = &AVAudioReceiverEngine::SetVideoHeight;
    funcMap_[AVTransTag::VIDEO_FRAME_RATE] = &AVAudioReceiverEngine::SetVideoFrameRate;
    funcMap_[AVTransTag::AUDIO_BIT_RATE] = &AVAudioReceiverEngine::SetAudioBitRate;
    funcMap_[AVTransTag::VIDEO_BIT_RATE] = &AVAudioReceiverEngine::SetVideoBitRate;
    funcMap_[AVTransTag::VIDEO_CODEC_TYPE] = &AVAudioReceiverEngine::SetVideoCodecType;
    funcMap_[AVTransTag::AUDIO_CODEC_TYPE] = &AVAudioReceiverEngine::SetAudioCodecType;
    funcMap_[AVTransTag::AUDIO_CHANNEL_MASK] = &AVAudioReceiverEngine::SetAudioChannelMask;
    funcMap_[AVTransTag::AUDIO_SAMPLE_RATE] = &AVAudioReceiverEngine::SetAudioSampleRate;
    funcMap_[AVTransTag::AUDIO_CHANNEL_LAYOUT] = &AVAudioReceiverEngine::SetAudioChannelLayout;
    funcMap_[AVTransTag::AUDIO_SAMPLE_FORMAT] = &AVAudioReceiverEngine::SetAudioSampleFormat;
    funcMap_[AVTransTag::AUDIO_FRAME_SIZE] = &AVAudioReceiverEngine::SetAudioFrameSize;
    funcMap_[AVTransTag::TIME_SYNC_RESULT] = &AVAudioReceiverEngine::SetSyncResult;
    funcMap_[AVTransTag::START_AV_SYNC] = &AVAudioReceiverEngine::SetStartAvSync;
    funcMap_[AVTransTag::STOP_AV_SYNC] = &AVAudioReceiverEngine::SetStopAvSync;
    funcMap_[AVTransTag::SHARED_MEMORY_FD] = &AVAudioReceiverEngine::SetSharedMemoryFd;
    funcMap_[AVTransTag::ENGINE_READY] = &AVAudioReceiverEngine::SetEngineReady;
}

void AVAudioReceiverEngine::SetVideoWidth(const std::string &value)
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

void AVAudioReceiverEngine::SetVideoHeight(const std::string &value)
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

void AVAudioReceiverEngine::SetVideoFrameRate(const std::string &value)
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

void AVAudioReceiverEngine::SetAudioBitRate(const std::string &value)
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

void AVAudioReceiverEngine::SetVideoBitRate(const std::string &value)
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

void AVAudioReceiverEngine::SetVideoCodecType(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    meta_->SetData(Media::Tag::MIME_TYPE, value);
}

void AVAudioReceiverEngine::SetAudioCodecType(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::MIME_TYPE, intValue);
    } else {
        AVTRANS_LOGI("SetParameter MIME_TYPE failed, value conversion failed.");
    }
}

void AVAudioReceiverEngine::SetAudioChannelMask(const std::string &value)
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

void AVAudioReceiverEngine::SetAudioSampleRate(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    int intValue = 0;
    if (ConvertToInt(value, intValue)) {
        meta_->SetData(Media::Tag::AUDIO_SAMPLE_RATE, intValue);
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_RATE success, audio sample rate = %{public}s", value.c_str());
    } else {
        AVTRANS_LOGI("SetParameter AUDIO_SAMPLE_RATE failed, value conversion failed.");
    }
}

void AVAudioReceiverEngine::SetAudioChannelLayout(const std::string &value)
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

void AVAudioReceiverEngine::SetAudioSampleFormat(const std::string &value)
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

void AVAudioReceiverEngine::SetAudioFrameSize(const std::string &value)
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

void AVAudioReceiverEngine::SetSyncResult(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    meta_->SetData(Media::Tag::USER_TIME_SYNC_RESULT, value);
    AVTRANS_LOGI("SetParameter USER_TIME_SYNC_RESULT success, time sync result = %{public}s", value.c_str());
}

void AVAudioReceiverEngine::SetStartAvSync(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    meta_->SetData(Media::Tag::USER_AV_SYNC_GROUP_INFO, value);
    AVTRANS_LOGI("SetParameter START_AV_SYNC success.");
}

void AVAudioReceiverEngine::SetStopAvSync(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    meta_->SetData(Media::Tag::USER_AV_SYNC_GROUP_INFO, value);
    AVTRANS_LOGI("SetParameter STOP_AV_SYNC success.");
}

void AVAudioReceiverEngine::SetSharedMemoryFd(const std::string &value)
{
    if (meta_ == nullptr) {
        AVTRANS_LOGE("meta_ is nullptr.");
        return;
    }
    meta_->SetData(Media::Tag::USER_SHARED_MEMORY_FD, value);
    AVTRANS_LOGI("SetParameter USER_SHARED_MEMORY_FD success, shared memory info = %{public}s", value.c_str());
}

void AVAudioReceiverEngine::SetEngineReady(const std::string &value)
{
    int32_t ret = PreparePipeline(value);
    TRUE_LOG_MSG(ret != DH_AVT_SUCCESS, "SetParameter ENGINE_READY failed");
}

int32_t AVAudioReceiverEngine::SendMessage(const std::shared_ptr<AVTransMessage> &message)
{
    TRUE_RETURN_V_MSG_E(message == nullptr, ERR_DH_AVT_INVALID_PARAM, "input message is nullptr.");
    std::string msgData = message->MarshalMessage();
    return SoftbusChannelAdapter::GetInstance().SendBytesData(sessionName_, message->dstDevId_, msgData);
}

int32_t AVAudioReceiverEngine::RegisterReceiverCallback(const std::shared_ptr<IAVReceiverEngineCallback> &callback)
{
    AVTRANS_LOGI("RegisterReceiverCallback enter.");
    if (callback == nullptr) {
        AVTRANS_LOGE("RegisterReceiverCallback failed, receiver engine callback is nullptr.");
        return ERR_DH_AVT_INVALID_PARAM;
    }
    receiverCallback_ = callback;
    return DH_AVT_SUCCESS;
}

bool AVAudioReceiverEngine::StartDumpMediaData()
{
    return true;
}

bool AVAudioReceiverEngine::StopDumpMediaData()
{
    return true;
}

bool AVAudioReceiverEngine::ReStartDumpMediaData()
{
    return true;
}

int32_t AVAudioReceiverEngine::HandleOutputBuffer(std::shared_ptr<Media::AVBuffer> &hisBuffer)
{
    StateId currentState = GetCurrentState();
    bool isErrState = (currentState != StateId::STARTED) && (currentState != StateId::PLAYING);
    TRUE_RETURN_V_MSG_E(isErrState, ERR_DH_AVT_OUTPUT_DATA_FAILED,
        "current state=%{public}" PRId32 " is invalid.", currentState);
    std::shared_ptr<AVTransBuffer> transBuffer = std::make_shared<AVTransBuffer>(MetaType::AUDIO);
    TRUE_RETURN_V_MSG_E(hisBuffer == nullptr || hisBuffer->memory_ == nullptr || hisBuffer->meta_ == nullptr,
        ERR_DH_AVT_NULL_POINTER, "hisBuffer is invalid");
    transBuffer->WrapBufferData(hisBuffer->memory_->GetAddr(), hisBuffer->memory_->GetCapacity(),
        hisBuffer->memory_->GetSize());
    transBuffer->SetPts(hisBuffer->pts_);
    int64_t ptsSpecial = 0;
    hisBuffer->meta_->GetData(Media::Tag::USER_FRAME_PTS, ptsSpecial);
    transBuffer->SetPtsSpecial(ptsSpecial);
    SetCurrentState(StateId::PLAYING);
    TRUE_RETURN_V(receiverCallback_ == nullptr, ERR_DH_AVT_OUTPUT_DATA_FAILED);
    return receiverCallback_->OnDataAvailable(transBuffer);
}

void AVAudioReceiverEngine::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter. event type:%{public}" PRId32, event.type);
    TRUE_RETURN(receiverCallback_ == nullptr, "receiver callback is nullptr.");

    switch (event.type) {
        case OHOS::DistributedHardware::EventType::EVENT_CHANNEL_OPENED: {
            if (GetCurrentState() != StateId::STARTED) {
                SetCurrentState(StateId::CH_CREATED);
            }
            AVTransEvent eventSend;
            eventSend.type = EventType::EVENT_START_SUCCESS;
            receiverCallback_->OnReceiverEvent(eventSend);
            break;
        }
        case OHOS::DistributedHardware::EventType::EVENT_CHANNEL_OPEN_FAIL: {
            SetCurrentState(StateId::INITIALIZED);
            receiverCallback_->OnReceiverEvent(event);
            break;
        }
        case OHOS::DistributedHardware::EventType::EVENT_CHANNEL_CLOSED: {
            StateId currentState = GetCurrentState();
            if ((currentState != StateId::IDLE) && (currentState != StateId::INITIALIZED)) {
                SetCurrentState(StateId::INITIALIZED);
                receiverCallback_->OnReceiverEvent(event);
            }
            break;
        }
        case OHOS::DistributedHardware::EventType::EVENT_DATA_RECEIVED: {
            auto avMessage = std::make_shared<AVTransMessage>();
            TRUE_RETURN(!avMessage->UnmarshalMessage(event.content, event.peerDevId), "unmarshal message failed");
            receiverCallback_->OnMessageReceived(avMessage);
            break;
        }
        default:
            AVTRANS_LOGE("Invalid event type.");
    }
}

void AVAudioReceiverEngine::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

void AVAudioReceiverEngine::OnEvent(const Pipeline::Event &event)
{
    switch (event.type) {
        case Pipeline::EventType::EVENT_BUFFER_PROGRESS: {
            auto hisBuffer = Media::AnyCast<std::shared_ptr<Media::AVBuffer>>(event.param);
            HandleOutputBuffer(hisBuffer);
            break;
        }
        case Pipeline::EventType::EVENT_AUDIO_PROGRESS: {
            auto channelEvent = Media::AnyCast<AVTransEvent>(event.param);
            OnChannelEvent(channelEvent);
            break;
        }
        default:
            AVTRANS_LOGE("Invalid event type.");
    }
}

Status AVAudioReceiverEngine::OnCallback(const std::shared_ptr<Pipeline::Filter>& filter,
    const Pipeline::FilterCallBackCommand cmd, Pipeline::StreamType outType)
{
    AVTRANS_LOGI("PipeDemo::OnCallback filter, outType: %{public}d", outType);
    if (cmd == Pipeline::FilterCallBackCommand::NEXT_FILTER_NEEDED) {
        switch (outType) {
            case Pipeline::StreamType::STREAMTYPE_RAW_AUDIO:
                return LinkAudioSinkFilter(filter, outType);
            case Pipeline::StreamType::STREAMTYPE_DECODED_AUDIO:
                return LinkAudioDecoderFilter(filter, outType);
            default:
                break;
        }
    }
    return Status::OK;
}

Status AVAudioReceiverEngine::LinkAudioDecoderFilter(const std::shared_ptr<Pipeline::Filter>& preFilter,
    Pipeline::StreamType type)
{
    AVTRANS_LOGI("PipeDemo::LinkAudioDecoderFilter");
    TRUE_RETURN_V(audioDecoder_ != nullptr, Status::OK);
    audioDecoder_ = std::make_shared<Pipeline::AudioDecoderFilter>("builtin.recorder.audiodecoderfilter",
        Pipeline::FilterType::FILTERTYPE_ADEC);
    TRUE_RETURN_V(audioDecoder_ == nullptr, Status::ERROR_NULL_POINTER);
    audioDecoder_->Init(playEventReceiver_, playFilterCallback_);
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("Pipeline_ is nullptr");
        return Status::ERROR_NULL_POINTER;
    }
    pipeline_->LinkFilters(preFilter, {audioDecoder_}, type);
    return Status::OK;
}

Status AVAudioReceiverEngine::LinkAudioSinkFilter(const std::shared_ptr<Pipeline::Filter>& preFilter,
    Pipeline::StreamType type)
{
    AVTRANS_LOGI("PipeDemo::LinkAudioDecoderFilter");
    TRUE_RETURN_V(avOutput_ != nullptr, Status::OK);
    avOutput_ = std::make_shared<Pipeline::DAudioOutputFilter>("builtin.daudio.output",
        Pipeline::FilterType::FILTERTYPE_SSINK);
    TRUE_RETURN_V(avOutput_ == nullptr, Status::ERROR_NULL_POINTER);
    avOutput_->Init(playEventReceiver_, playFilterCallback_);
    if (pipeline_ == nullptr) {
        AVTRANS_LOGE("Pipeline_ is nullptr");
        return Status::ERROR_NULL_POINTER;
    }
    pipeline_->LinkFilters(preFilter, {avOutput_}, type);
    return Status::OK;
}

Status AVAudioReceiverEngine::Prepare()
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
} // namespace DistributedHardware
} // namespace OHOS