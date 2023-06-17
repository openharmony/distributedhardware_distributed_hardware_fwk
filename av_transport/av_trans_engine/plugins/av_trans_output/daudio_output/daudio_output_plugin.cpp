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
    AvTransOutputPluginDef definition;
    definition.name = "AVTransDaudioOutputPlugin";
    definition.description = "Send audio playback and frame rate control.";
    definition.rank = PLUGIN_RANK;
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
    DHLOGI("ctor.");
}

DaudioOutputPlugin::~DaudioOutputPlugin()
{
    DHLOGI("dtor.");
}

Status DaudioOutputPlugin::Init()
{
    DHLOGI("Init.");
    OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DaudioOutputPlugin::Deinit()
{
    DHLOGI("Deinit.");
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
        DHLOGE("Resample init error");
    }
}

Status DaudioOutputPlugin::Prepare()
{
    DHLOGI("Prepare");
    OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    if (sendPlayTask_ == nullptr) {
        sendPlayTask_ = std::make_shared<OHOS::Media::OSAL::Task>("sendPlayTask_");
        sendPlayTask_->RegisterHandler([this] { HandleData(); });
    }

    ValueType channelsValue;
    TRUE_RETURN_V_MSG_E(GetParameter(Tag::AUDIO_CHANNELS, channelsValue) != Status::OK,
        Status::ERROR_UNKNOWN, "Not found AUDIO_CHANNELS");
    uint32_t channels = Plugin::AnyCast<int>(channelsValue);

    ValueType sampleRateValue;
    TRUE_RETURN_V_MSG_E(GetParameter(Tag::AUDIO_SAMPLE_RATE, sampleRateValue) != Status::OK,
        Status::ERROR_UNKNOWN, "Not found AUDIO_SAMPLE_RATE");
    uint32_t sampleRate = Plugin::AnyCast<int>(sampleRateValue);

    ValueType channelsLayoutValue;
    TRUE_RETURN_V_MSG_E(GetParameter(Tag::AUDIO_CHANNEL_LAYOUT, channelsLayoutValue) != Status::OK,
        Status::ERROR_UNKNOWN, "Not found AUDIO_CHANNEL_LAYOUT");
    uint32_t channelsLayout = Plugin::AnyCast<int>(channelsLayoutValue);
    if (channelsLayout == AUDIO_CHANNEL_LAYOUT_MONO) {
        channelsLayout = AV_CH_LAYOUT_MONO;
    } else if (channelsLayout == AUDIO_CHANNEL_LAYOUT_STEREO) {
        channelsLayout = AV_CH_LAYOUT_STEREO;
    }
    DHLOGI("channels = %d, sampleRate = %d, channelLayout = %d.", channels, sampleRate, channelsLayout);
    RampleInit(channels, sampleRate, channelsLayout);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DaudioOutputPlugin::Reset()
{
    DHLOGI("Reset enter");
    OSAL::ScopedLock lock(operationMutes_);
    eventcallback_ = nullptr;
    if (sendPlayTask_) {
        sendPlayTask_->Stop();
        sendPlayTask_.reset();
    }
    if (resample_) {
        resample_.reset();
    }
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
    return Status::OK;
}

Status DaudioOutputPlugin::Start()
{
    DHLOGI("Start enter");
    OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    DataQueueClear(outputBuffer_);
    sendPlayTask_->Start();
    state_ = State::RUNNING;
    return Status::OK;
}

Status DaudioOutputPlugin::Stop()
{
    DHLOGI("Stop enter");
    OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::RUNNING) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    sendPlayTask_->Pause();
    DataQueueClear(outputBuffer_);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DaudioOutputPlugin::SetCallback(Callback *cb)
{
    OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        DHLOGE("SetCallBack failed, cb is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventcallback_ = cb;
    DHLOGI("SetCallback success.");
    return Status::OK;
}

Status DaudioOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    OSAL::ScopedLock lock(operationMutes_);
    if (callback == nullptr) {
        DHLOGE("SetCallBack failed, callback is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    datacallback_ = callback;
    DHLOGI("SetDataCallback success.");
    return Status::OK;
}

Status DaudioOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset)
{
    DHLOGI("PushData enter.");
    OSAL::ScopedLock lock(operationMutes_);
    uint32_t frameNumber = 1;
    int64_t pts = 2;
    if (buffer->GetBufferMeta()->IsExist(Tag::USER_FRAME_NUMBER)) {
        frameNumber = Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_NUMBER));
    }
    if (buffer->GetBufferMeta()->IsExist(Tag::USER_FRAME_PTS)) {
        pts = Plugin::AnyCast<int64_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_PTS));
    }
    DHLOGI("buffer pts: %ld, bufferLen: %zu, frameNumber: %zu", pts, buffer->GetMemory()->GetSize(), frameNumber);

    if (buffer == nullptr || buffer->IsEmpty()) {
        DHLOGE("AVBuffer is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    while (outputBuffer_.size() >= DATA_QUEUE_MAX_SIZE) {
        DHLOGE("outputBuffer_ queue overflow.");
        outputBuffer_.pop();
    }

    auto finalBufferMeta = buffer->GetBufferMeta()->Clone();
    auto mem = buffer->GetMemory();
    auto srcBuffer = mem->GetReadOnlyData();
    auto destBuffer = const_cast<uint8_t*>(srcBuffer);
    auto srcLength = mem->GetSize();
    auto destLength = srcLength;
    if (resample_) {
        if (resample_->Convert(srcBuffer, srcLength, destBuffer, destLength) != Status::OK) {
            DHLOGE("Resample convert failed.");
        }
    }
    auto finalBuffer = Buffer::CreateDefaultBuffer(BufferMetaType::AUDIO, destLength);
    auto bufData = finalBuffer->GetMemory();
    auto writeSize = bufData->Write(reinterpret_cast<const uint8_t *>(destBuffer), destLength, 0);
    if (static_cast<ssize_t>(writeSize) != destLength) {
        DHLOGE("Write buffer data failed.");
        return Status::ERROR_NULL_POINTER;
    }
    finalBuffer->UpdateBufferMeta(*finalBufferMeta);
    outputBuffer_.push(finalBuffer);

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
                DHLOGD("Data queue is empty.");
                continue;
            }
            buffer = outputBuffer_.front();
            outputBuffer_.pop();
        }
        if (buffer == nullptr) {
            DHLOGE("Data is null");
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
    DHLOGI("StartOutputQueue enter.");
    return Status::OK;
}

Status DaudioOutputPlugin::ControlFrameRate(const int64_t timestamp)
{
    DHLOGI("ControlFrameRate enter.");
    return Status::OK;
}
} // namespace DistributedHardware
} // namespace OHOS
