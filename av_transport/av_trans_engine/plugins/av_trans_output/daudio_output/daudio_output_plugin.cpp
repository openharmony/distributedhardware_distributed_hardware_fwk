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

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<AvTransOutputPlugin> DaudioOutputPluginCreator(const std::string& name)
{
    return std::make_shared<DaudioOutputPlugin>(name);
}

Status DaudioOutputRegister(const std::shared_ptr<Register> &reg)
{
    AVTRANS_LOGI("DaudioOutputRegister enter.");
    AvTransOutputPluginDef definition;
    definition.name = "AVTransDaudioOutputPlugin";
    definition.description = "Send audio playback and frame rate control.";
    definition.creator = DaudioOutputPluginCreator;
    definition.pluginType = PluginType::AVTRANS_OUTPUT;

    CapabilityBuilder capBuilder;
    capBuilder.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
    DiscreteCapability<uint32_t> valuesSampleRate = {8000, 11025, 12000, 16000,
        22050, 24000, 32000, 44100, 48000, 64000, 96000};
    capBuilder.SetAudioSampleRateList(valuesSampleRate);
    DiscreteCapability<AudioSampleFormat> valuesSampleFormat = {AudioSampleFormat::F32P};
    capBuilder.SetAudioSampleFormatList(valuesSampleFormat);
    definition.inCaps.push_back(capBuilder.Build());

    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTransDaudioOutput, LicenseType::APACHE_V2, DaudioOutputRegister, [] {});

DaudioOutputPlugin::DaudioOutputPlugin(std::string name)
    : AvTransOutputPlugin(std::move(name))
{
    AVTRANS_LOGI("DaudioOutputPlugin ctor.");
}

DaudioOutputPlugin::~DaudioOutputPlugin()
{
    AVTRANS_LOGI("DaudioOutputPlugin dtor.");
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

Status DaudioOutputPlugin::Prepare()
{
    AVTRANS_LOGI("Prepare enter");
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
    Status ret = GetParameter(Tag::AUDIO_CHANNELS, channelsValue);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Not found AUDIO_CHANNELS");
        return Status::ERROR_UNKNOWN;
    }
    uint32_t channels = Plugin::AnyCast<int>(channelsValue);

    ValueType sampleRateValue;
    ret = GetParameter(Tag::AUDIO_SAMPLE_RATE, sampleRateValue);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Not found AUDIO_SAMPLE_RATE");
        return Status::ERROR_UNKNOWN;
    }
    uint32_t sampleRate = Plugin::AnyCast<int>(sampleRateValue);

    ValueType channelsLayoutValue;
    ret = GetParameter(Tag::AUDIO_CHANNEL_LAYOUT, channelsLayoutValue);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Not found AUDIO_CHANNEL_LAYOUT");
        return Status::ERROR_UNKNOWN;
    }
    uint32_t channelsLayout = Plugin::AnyCast<int>(channelsLayoutValue);
    AVTRANS_LOGI("channels = %d, sampleRate = %d, channelLayout = %d.", channels, sampleRate, channelsLayout);
    resample = std::make_shared<Ffmpeg::Resample>();
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
    paramsMap_.clear();
    DataQueueClear(outputBuffer_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DaudioOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    AVTRANS_LOGI("GetParameter enter.");
    auto iter = paramsMap_.find(tag);
    if (iter != paramsMap_.end()) {
        value = iter->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DaudioOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    AVTRANS_LOGI("SetParameter enter.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    paramsMap_.insert(std::make_pair(tag, value));
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
    sendPlayTask_->Pause();
    DataQueueClear(outputBuffer_);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DaudioOutputPlugin::SetCallback(Callback *cb)
{
    AVTRANS_LOGI("SetCallBack enter");
    OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        AVTRANS_LOGE("SetCallBack failed, cb is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventcallback_ = cb;
    return Status::OK;
}

Status DaudioOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    AVTRANS_LOGI("SetDataCallback enter.");
    OSAL::ScopedLock lock(operationMutes_);
    if (callback == nullptr) {
        AVTRANS_LOGE("SetCallBack failed, callback is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    datacallback_ = callback;
    AVTRANS_LOGI("SetDataCallback success.");
    return Status::OK;
}

Status DaudioOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer,
    int32_t offset)
{
    AVTRANS_LOGI("PushData enter.");
    OSAL::ScopedLock lock(operationMutes_);
    if (buffer == nullptr || buffer->IsEmpty()) {
        AVTRANS_LOGE("AVBuffer is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
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
            dataCond_.wait(lock, [this]() { return !outputBuffer_.empty(); });
            if (outputBuffer_.empty()) {
                AVTRANS_LOGD("Data queue is empty.");
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
