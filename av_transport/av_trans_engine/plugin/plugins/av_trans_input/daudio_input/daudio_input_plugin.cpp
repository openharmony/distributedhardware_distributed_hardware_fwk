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

#include "foundation/utils/constants.h"
#include "plugin/common/plugin_caps_builder.h"
#include "plugin/factory/plugin_factory.h"
#include "plugin/interface/generic_plugin.h"

namespace OHOS {
namespace DistributedHardware {

GenericPluginDef CreateDaudioInputPluginDef()
{
    GenericPluginDef definition;
    definition.name = "AVTransDaudioInputPlugin";
    definition.pkgName = "AVTransDaudioInputPlugin";
    definition.description = "Audio transport from daudio service";
    definition.rank = PLUGIN_RANK;
    definition.creator = [] (const std::string& name) -> std::shared_ptr<AvTransInputPlugin> {
        return std::make_shared<DaudioInputPlugin>(name);
    };

    definition.pkgVersion = AVTRANS_INPUT_API_VERSION;
    definition.license = LicenseType::APACHE_V2;

    CapabilityBuilder capBuilder;
    capBuilder.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_RAW);
    DiscreteCapability<uint32_t> values = {8000, 11025, 12000, 16000,
        22050, 24000, 32000, 44100, 48000, 64000, 96000};
    capBuilder.SetAudioSampleRateList(values);
    definition.outCaps.push_back(capBuilder.Build());
    return definition;
}

static AutoRegisterPlugin<DaudioInputPlugin> g_registerPluginHelper(CreateDaudioInputPluginDef());

DaudioInputPlugin::DaudioInputPlugin(std::string name)
    : AvTransInputPlugin(std::move(name))
{
    AVTRANS_LOGI("DaudioInputPlugin ctor.");
}

DaudioInputPlugin::~DaudioInputPlugin()
{
    AVTRANS_LOGI("DaudioInputPlugin dtor.");
}

Status DaudioInputPlugin::Init()
{
    AVTRANS_LOGI("Init enter.");
    frameNumber_.store(0);
    return Status::OK;
}

Status DaudioInputPlugin::Deinit()
{
    AVTRANS_LOGI("Deinit enter.");
    return Reset();
}

Status DaudioInputPlugin::Reset()
{
    AVTRANS_LOGI("Reset enter.");
    std::lock_guard<std::mutex> lock(tagMapMutex_);
    tagMap_.clear();
    frameNumber_.store(0);
    return Status::OK;
}

Status DaudioInputPlugin::Pause()
{
    AVTRANS_LOGI("Pause enter.");
    if ((sharedMemory_.fd > 0) && (sharedMemory_.size > 0) && !sharedMemory_.name.empty()) {
        ResetSharedMemory(sharedMemory_);
    }
    return Status::OK;
}

Status DaudioInputPlugin::Resume()
{
    AVTRANS_LOGI("Resume enter.");
    return Status::OK;
}

Status DaudioInputPlugin::GetParameter(Tag tag, ValueType &value)
{
    {
        std::lock_guard<std::mutex> lock(tagMapMutex_);
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
    std::lock_guard<std::mutex> lock(tagMapMutex_);
    tagMap_.insert(std::make_pair(tag, value));
    if (tag == Plugin::Tag::USER_SHARED_MEMORY_FD) {
        sharedMemory_ = UnmarshalSharedMemory(Media::Plugin::AnyCast<std::string>(value));
    }
    return Status::OK;
}

Status DaudioInputPlugin::PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset)
{
    std::lock_guard<std::mutex> lock(tagMapMutex_);
    TRUE_RETURN_V(buffer == nullptr, Status::ERROR_NULL_POINTER);
    if (buffer->IsEmpty()) {
        AVTRANS_LOGE("bufferData is Empty.");
        return Status::ERROR_INVALID_PARAMETER;
    }

    auto bufferMeta = buffer->GetBufferMeta();
    TRUE_RETURN_V(bufferMeta == nullptr, Status::ERROR_NULL_POINTER);
    if (bufferMeta->GetType() != BufferMetaType::AUDIO) {
        AVTRANS_LOGE("bufferMeta is wrong.");
        return Status::ERROR_INVALID_PARAMETER;
    }

    ++frameNumber_;
    buffer->pts = GetCurrentTime();
    bufferMeta->SetMeta(Tag::USER_FRAME_PTS, buffer->pts);
    bufferMeta->SetMeta(Tag::USER_FRAME_NUMBER, frameNumber_.load());
    AVTRANS_LOGI("Push audio buffer pts: %ld, bufferLen: %d, indexNumber: %u.",
        buffer->pts, buffer->GetMemory()->GetSize(),
        Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_NUMBER)));

    if ((sharedMemory_.fd > 0) && (sharedMemory_.size > 0) && !sharedMemory_.name.empty()) {
        WriteFrameInfoToMemory(sharedMemory_, frameNumber_.load(), buffer->pts);
    }

    return Status::OK;
}

Status DaudioInputPlugin::SetCallback(Callback *cb)
{
    (void)cb;
    return Status::OK;
}

Status DaudioInputPlugin::SetDataCallback(AVDataCallback callback)
{
    (void)callback;
    return Status::OK;
}
} // namespace DistributedHardware
} // namespace OHOS
