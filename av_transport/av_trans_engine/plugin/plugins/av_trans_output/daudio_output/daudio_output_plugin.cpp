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

#include "daudio_output_plugin.h"

#include "foundation/utils/constants.h"
#include "plugin/common/plugin_caps_builder.h"
#include "plugin/factory/plugin_factory.h"
#include "plugin/interface/generic_plugin.h"

namespace OHOS {
namespace DistributedHardware {

GenericPluginDef CreateDaudioOutputPluginDef()
{
    GenericPluginDef definition;
    definition.name = "AVTransDaudioOutputPlugin";
    definition.pkgName = "AVTransDaudioOutputPlugin";
    definition.description = "Send audio playback and frame rate control.";
    definition.rank = PLUGIN_RANK;
    definition.creator = [] (const std::string& name) -> std::shared_ptr<AvTransOutputPlugin> {
        return std::make_shared<DaudioOutputPlugin>(name);
    };

    definition.pkgVersion = AVTRANS_OUTPUT_API_VERSION;
    definition.license = LicenseType::APACHE_V2;

    CapabilityBuilder capBuilder;
    capBuilder.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
    DiscreteCapability<uint32_t> valuesSampleRate = {8000, 11025, 12000, 16000,
        22050, 24000, 32000, 44100, 48000, 64000, 96000};
    capBuilder.SetAudioSampleRateList(valuesSampleRate);
    DiscreteCapability<AudioSampleFormat> valuesSampleFormat = {AudioSampleFormat::F32P};
    capBuilder.SetAudioSampleFormatList(valuesSampleFormat);
    definition.inCaps.push_back(capBuilder.Build());
    return definition;
}

static AutoRegisterPlugin<DaudioOutputPlugin> g_registerPluginHelper(CreateDaudioOutputPluginDef());

DaudioOutputPlugin::DaudioOutputPlugin(std::string name)
    : AvTransOutputPlugin(std::move(name))
{
    AVTRANS_LOGI("ctor.");
}

DaudioOutputPlugin::~DaudioOutputPlugin()
{
    AVTRANS_LOGI("dtor.");
}

Status DaudioOutputPlugin::Init()
{
    AVTRANS_LOGI("Init.");
    OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DaudioOutputPlugin::Deinit()
{
    AVTRANS_LOGI("Deinit.");
    return Reset();
}

void DaudioOutputPlugin::RampleInit(uint32_t channels, uint32_t sampleRate, uint32_t channelLayout)
{
    resample_ = std::make_shared<Ffmpeg::Resample>();
    Ffmpeg::ResamplePara resamplePara {
        channels, // channels_
        sampleRate, // sampleRate_
        0, // bitsPerSample_
        static_cast<int64_t>(channelLayout), // channelLayout_
        AV_SAMPLE_FMT_FLTP,
        2048, // samplePerFrame_
        AV_SAMPLE_FMT_S16,
    };
    if (resample_->Init(resamplePara) != Status::OK) {
        AVTRANS_LOGE("Resample init error");
    }
}

Status DaudioOutputPlugin::Prepare()
{
    AVTRANS_LOGI("Prepare");
    OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    if (sendPlayTask_ == nullptr) {
        sendPlayTask_ = std::make_shared<OHOS::Media::OSAL::Task>("sendPlayTask_");
        sendPlayTask_->RegisterHandler([this] { HandleData(); });
    }

    ValueType channelsValue;
    TRUE_RETURN_V_MSG_E(GetParameter(Tag::AUDIO_CHANNELS, channelsValue) != Status::OK,
        Status::ERROR_UNKNOWN, "Not found AUDIO_CHANNELS");
    uint32_t channels = static_cast<uint32_t>(Plugin::AnyCast<int>(channelsValue));

    ValueType sampleRateValue;
    TRUE_RETURN_V_MSG_E(GetParameter(Tag::AUDIO_SAMPLE_RATE, sampleRateValue) != Status::OK,
        Status::ERROR_UNKNOWN, "Not found AUDIO_SAMPLE_RATE");
    uint32_t sampleRate = static_cast<uint32_t>(Plugin::AnyCast<int>(sampleRateValue));

    ValueType channelsLayoutValue;
    TRUE_RETURN_V_MSG_E(GetParameter(Tag::AUDIO_CHANNEL_LAYOUT, channelsLayoutValue) != Status::OK,
        Status::ERROR_UNKNOWN, "Not found AUDIO_CHANNEL_LAYOUT");
    uint32_t channelsLayout = static_cast<uint32_t>(Plugin::AnyCast<int>(channelsLayoutValue));
    if (channelsLayout == AUDIO_CHANNEL_LAYOUT_MONO) {
        channelsLayout = AV_CH_LAYOUT_MONO;
    } else if (channelsLayout == AUDIO_CHANNEL_LAYOUT_STEREO) {
        channelsLayout = AV_CH_LAYOUT_STEREO;
    }
    AVTRANS_LOGI("channels = %d, sampleRate = %d, channelLayout = %d.", channels, sampleRate, channelsLayout);
    RampleInit(channels, sampleRate, channelsLayout);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DaudioOutputPlugin::Reset()
{
    AVTRANS_LOGI("Reset enter");
    OSAL::ScopedLock lock(operationMutes_);
    eventcallback_ = nullptr;
    if (sendPlayTask_) {
        sendPlayTask_->Stop();
        sendPlayTask_.reset();
    }
    if (resample_) {
        resample_.reset();
    }
    smIndex_ = 0;
    paramsMap_.clear();
    DataQueueClear(outputBuffer_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DaudioOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    auto iter = paramsMap_.find(tag);
    if (iter != paramsMap_.end()) {
        value = iter->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DaudioOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    paramsMap_.insert(std::make_pair(tag, value));
    if (tag == Plugin::Tag::USER_SHARED_MEMORY_FD) {
        std::unique_lock<std::mutex> lock(sharedMemMtx_);
        sharedMemory_ = UnmarshalSharedMemory(Media::Plugin::AnyCast<std::string>(value));
    }
    if (tag == Plugin::Tag::USER_AV_SYNC_GROUP_INFO) {
        std::string groupInfo = Media::Plugin::AnyCast<std::string>(value);
        AVTRANS_LOGI("Set USER_AV_SYNC_GROUP_INFO parameter done, group info = %s.", GetAnonyString(groupInfo).c_str());
    }
    return Status::OK;
}

Status DaudioOutputPlugin::Start()
{
    AVTRANS_LOGI("Start enter");
    OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    DataQueueClear(outputBuffer_);
    sendPlayTask_->Start();
    state_ = State::RUNNING;
    return Status::OK;
}

Status DaudioOutputPlugin::Stop()
{
    AVTRANS_LOGI("Stop enter");
    OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::RUNNING) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    state_ = State::PREPARED;
    sendPlayTask_->Stop();
    DataQueueClear(outputBuffer_);
    return Status::OK;
}

Status DaudioOutputPlugin::SetCallback(Callback *cb)
{
    OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        AVTRANS_LOGE("SetCallBack failed, cb is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventcallback_ = cb;
    AVTRANS_LOGI("SetCallback success.");
    return Status::OK;
}

Status DaudioOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    OSAL::ScopedLock lock(operationMutes_);
    if (callback == nullptr) {
        AVTRANS_LOGE("SetCallBack failed, callback is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    datacallback_ = callback;
    AVTRANS_LOGI("SetDataCallback success.");
    return Status::OK;
}

Status DaudioOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset)
{
    if (buffer == nullptr || buffer->IsEmpty() || (buffer->GetBufferMeta() == nullptr)) {
        AVTRANS_LOGE("input buffer is nullptr, push data failed.");
        return Status::ERROR_NULL_POINTER;
    }

    auto bufferMeta = buffer->GetBufferMeta();
    if (bufferMeta->IsExist(Tag::USER_FRAME_NUMBER) && bufferMeta->IsExist(Tag::USER_FRAME_PTS)) {
        int64_t pts = Plugin::AnyCast<int64_t>(bufferMeta->GetMeta(Tag::USER_FRAME_PTS));
        uint32_t frameNum = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::USER_FRAME_NUMBER));
        AVTRANS_LOGI("Push audio buffer, bufferLen: %zu, frameNum: %d, pts: %ld", buffer->GetMemory()->GetSize(),
            frameNum, pts);
    } else {
        AVTRANS_LOGI("Push audio buffer, bufferLen: %zu, not contains metadata.", buffer->GetMemory()->GetSize());
    }

    OSAL::ScopedLock lock(operationMutes_);
    while (outputBuffer_.size() >= DATA_QUEUE_MAX_SIZE) {
        AVTRANS_LOGE("outputBuffer_ queue overflow.");
        outputBuffer_.pop();
    }
    outputBuffer_.push(buffer);
    dataCond_.notify_all();
    return Status::OK;
}

void DaudioOutputPlugin::HandleData()
{
    while (state_ == State::RUNNING) {
        std::shared_ptr<Plugin::Buffer> buffer;
        {
            std::unique_lock<std::mutex> lock(dataQueueMtx_);
            dataCond_.wait_for(lock, std::chrono::milliseconds(PLUGIN_TASK_WAIT_TIME),
                [this]() { return !outputBuffer_.empty(); });
            if (state_ != State::RUNNING) {
                return;
            }
            if (outputBuffer_.empty()) {
                continue;
            }
            buffer = outputBuffer_.front();
            outputBuffer_.pop();
        }
        if (buffer == nullptr) {
            AVTRANS_LOGE("Data is null");
            continue;
        }
        datacallback_(buffer);
        WriteMasterClockToMemory(buffer);
    }
}

void DaudioOutputPlugin::WriteMasterClockToMemory(const std::shared_ptr<Plugin::Buffer> &buffer)
{
    std::unique_lock<std::mutex> lock(sharedMemMtx_);
    if ((sharedMemory_.fd <= 0) || (sharedMemory_.size <= 0) || sharedMemory_.name.empty()) {
        return;
    }

    if ((buffer == nullptr) || (buffer->GetBufferMeta() == nullptr)) {
        AVTRANS_LOGE("output buffer or buffer meta is nullptr.");
        return;
    }

    if (!buffer->GetBufferMeta()->IsExist(Tag::USER_FRAME_NUMBER)) {
        AVTRANS_LOGE("the output buffer meta does not contains tag user_frame_number.");
        return;
    }

    if (!buffer->GetBufferMeta()->IsExist(Tag::USER_FRAME_PTS)) {
        AVTRANS_LOGE("the output buffer meta does not contains tag USER_FRAME_PTS.");
        return;
    }

    uint32_t frameNum = Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_NUMBER));
    int64_t pts = Plugin::AnyCast<int64_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_PTS));
    AVSyncClockUnit clockUnit = AVSyncClockUnit{ smIndex_, frameNum, pts };
    int32_t ret = WriteClockUnitToMemory(sharedMemory_, clockUnit);
    if (ret == DH_AVT_SUCCESS) {
        smIndex_ = clockUnit.index;
    }
}

void DaudioOutputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &q)
{
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, q);
}

Status DaudioOutputPlugin::StartOutputQueue()
{
    AVTRANS_LOGI("StartOutputQueue enter.");
    return Status::OK;
}

Status DaudioOutputPlugin::ControlFrameRate(const int64_t timestamp)
{
    AVTRANS_LOGI("ControlFrameRate enter.");
    return Status::OK;
}
} // namespace DistributedHardware
} // namespace OHOS
