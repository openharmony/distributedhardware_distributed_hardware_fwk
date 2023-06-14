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

#include "daudio_input_plugin.h"

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<DHPlugin> DaudioInputPluginCreator(const std::string& name)
{
    return std::make_shared<DaudioInputPlugin>(name);
}

Status DaudioInputRegister(const std::shared_ptr<Register>& reg)
{
    DHLOGI("DaudioInputRegister enter.");
    DHPluginDef definition;
    definition.name = "AVTransDaudioInputPlugin";
    definition.description = "Audio transport from daudio service";
    definition.rank = 100;
    definition.protocol.emplace_back(ProtocolType::STREAM);
    definition.inputType = SrcInputType::AUD_ES;
    definition.creator = DaudioInputPluginCreator;
    definition.pluginType = PluginType::DH;
    // Capability outCaps(MEDIA_MIME_AUDIO_RAW);
    // outCaps.AppendDiscreteKeys<AudioSampleFormat>(
    //     Capability::Key::AUDIO_SAMPLE_FORMAT, {OHOS::AudioStandard::AudioSampleFormat::SAMPLE_S16LE});
    // definition.outCaps.push_back(outCaps);
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTransDaudioInput, LicenseType::APACHE_V2, DaudioInputRegister, [] {});

DaudioInputPlugin::DaudioInputPlugin(std::string name)
    : DHPlugin(std::move(name))
{
    DHLOGI("DaudioInputPlugin ctor.");
}

DaudioInputPlugin::~DaudioInputPlugin()
{
    DHLOGI("DaudioInputPlugin dtor.");
}

Status DaudioInputPlugin::Init()
{
    DHLOGI("Init enter.");
    frameNumber_.store(0);
    return Status::OK;
}

Status DaudioInputPlugin::Deinit()
{
    DHLOGI("Deinit enter.");
    return Reset();
}

Status DaudioInputPlugin::Reset()
{
    DHLOGI("Reset enter.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    tagMap_.clear();
    frameNumber_.store(0);
    return Status::OK;
}

Status DaudioInputPlugin::GetParameter(Tag tag, ValueType &value)
{
    DHLOGI("GetParameter enter.");
    {
        Media::OSAL::ScopedLock lock(operationMutes_);
        auto iter = tagMap_.find(tag);
        if (iter != tagMap_.end()) {
            value = iter->second;
            return Status::OK;
        }
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DaudioInputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    DHLOGI("SetParameter enter.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    tagMap_.insert(std::make_pair(tag, value));
    return Status::OK;
}

Status DaudioInputPlugin::PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset)
{
    DHLOGI("PushData enter.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    TRUE_RETURN_V(buffer == nullptr, Status::ERROR_NULL_POINTER);

    if (buffer->IsEmpty()) {
        DHLOGE("bufferData is Empty.");
        return Status::ERROR_INVALID_PARAMETER;
    }

    auto bufferMeta = buffer->GetBufferMeta();
    TRUE_RETURN_V(bufferMeta == nullptr, Status::ERROR_NULL_POINTER);
    if (bufferMeta->GetType() != BufferMetaType::AUDIO) {
        DHLOGE("bufferMeta is wrong.");
        return Status::ERROR_INVALID_PARAMETER;
    }

    auto avTransMeta = ReinterpretCastPointer<AVTransAudioBufferMeta>(bufferMeta);
    TRUE_RETURN_V(avTransMeta == nullptr, Status::ERROR_NULL_POINTER);
    AddFrameInfo(avTransMeta);
    buffer->SetBufferMeta(avTransMeta);
    DHLOGI("AddFrameInfo buffer pts: %d, frameNumber: %d, dataType: %u, bufferLen: %d.",
        avTransMeta->pts_, avTransMeta->frameNum_, avTransMeta->dataType_, buffer->GetMemory()->GetSize());
    return Status::OK;
}

void DaudioInputPlugin::AddFrameInfo(std::shared_ptr<AVTransAudioBufferMeta>& bufferMeta)
{
    DHLOGI("AddFrameInfo enter.");
    ++frameNumber_;
    bufferMeta->frameNum_ = frameNumber_.load();
    bufferMeta->pts_ = GetCurrentTime();
    bufferMeta->dataType_ = GetValueFromParams(Tag::MEDIA_DESCRIPTION, AVT_PARAM_BUFFERMETA_DATATYPE);
}

BufferDataType DaudioInputPlugin::GetValueFromParams(Tag tag, std::string keyWord)
{
    auto iter = tagMap_.find(tag);
    if (iter == tagMap_.end()) {
        DHLOGE("Tag not found.");
        return BufferDataType::UNKNOW;
    }
    json paramsJson = json::parse(Media::Plugin::AnyCast<std::string>(tagMap_[tag]), nullptr, false);
    if (paramsJson.is_discarded() || !IsUInt32(paramsJson, keyWord)) {
        DHLOGE("The paramsJson is invalid.");
        return BufferDataType::UNKNOW;
    }
    return static_cast<BufferDataType>(paramsJson[keyWord]);
}

Status DaudioInputPlugin::SetCallback(Callback *cb)
{
    DHLOGI("SetCallback enter.");
    (void)cb;
    return Status::OK;
}

Status DaudioInputPlugin::SetDataCallback(AVDataCallback callback)
{
    DHLOGI("SetDataCallback enter.");
    (void)callback;
    return Status::OK;
}
} // namespace DistributedHardware
} // namespace OHOS
