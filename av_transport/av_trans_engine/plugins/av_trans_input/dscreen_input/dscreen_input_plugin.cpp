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
#include "dscreen_input_plugin.h"

#include "av_trans_utils.h"
#include "foundation/utils/constants.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace DistributedHardware {
using json = nlohmann::json;

std::shared_ptr<AvTransInputPlugin> DscreenInputPluginCreator(const std::string &name)
{
    return std::make_shared<DscreenInputPlugin>(name);
}

Status DscreenInputRegister(const std::shared_ptr<Register> &reg)
{
    AvTransInputPluginDef definition;
    definition.name = "AVTransDscreenInputPlugin";
    definition.description = "Video transport from dsrceen service";
    definition.rank = 100;
    definition.protocol.emplace_back(ProtocolType::STREAM);
    definition.inputType = SrcInputType::D_SCREEN;
    definition.pluginType = PluginType::AVTRANS_INPUT;
    definition.creator = DscreenInputPluginCreator;
    Capability outCap(Media::MEDIA_MIME_VIDEO_RAW);
    outCap.AppendDiscreteKeys<VideoPixelFormat>(
        Capability::Key::VIDEO_PIXEL_FORMAT, {VideoPixelFormat::RGBA});
    definition.outCaps.push_back(outCap);
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTransDscreenInput, LicenseType::APACHE_V2, DscreenInputRegister, [] {});

DscreenInputPlugin::DscreenInputPlugin(std::string name)
    : AvTransInputPlugin(std::move(name))
{
    DHLOGI("ctor.");
}

DscreenInputPlugin::~DscreenInputPlugin()
{
    DHLOGI("dtor.");
}

Status DscreenInputPlugin::Init()
{
    DHLOGI("Init DscreenInputPlugin.");
    frameNumber_.store(0);
    return Status::OK;
}

Status DscreenInputPlugin::Deinit()
{
    DHLOGI("Deinit DscreenInputPlugin.");
    return Reset();
}

Status DscreenInputPlugin::Reset()
{
    DHLOGI("Reset");
    Media::OSAL::ScopedLock lock(operationMutes_);
    paramsMap_.clear();
    frameNumber_.store(0);
    return Status::OK;
}

Status DscreenInputPlugin::GetParameter(Tag tag, ValueType &value)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    auto res = paramsMap_.find(tag);
    if (res == paramsMap_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DscreenInputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    paramsMap_.insert(std::pair<Tag, ValueType>(tag, value));
    return Status::OK;
}

Status DscreenInputPlugin::PushData(const std::string& inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    DHLOGI("PushData.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (!buffer || buffer->IsEmpty()) {
        DHLOGE("buffer is nullptr or empty.");
        return Status::ERROR_NULL_POINTER;
    }

    auto bufferMeta = buffer->GetBufferMeta();
    if (!bufferMeta || bufferMeta->GetType() != BufferMetaType::VIDEO) {
        DHLOGE("bufferMeta is nullptr or empty.");
        return Status::ERROR_NULL_POINTER;
    }
    auto avTransMeta = ReinterpretCastPointer<AVTransVideoBufferMeta>(bufferMeta);
    if (avTransMeta == nullptr) {
        DHLOGE("avTransMeta is nullptr or empty.");
        return Status::ERROR_NULL_POINTER;
    }
    AddFrameInfo(avTransMeta);
    buffer->SetBufferMeta(avTransMeta);
    DHLOGI("AddFrameInfo buffer pts: %d, frameNumber: %d, dataType: %u, bufferLen: %d.",
        avTransMeta->pts_, avTransMeta->frameNum_, avTransMeta->dataType_, buffer->GetMemory()->GetSize());
    return Status::OK;
}

void DscreenInputPlugin::AddFrameInfo(std::shared_ptr<AVTransVideoBufferMeta> &bufferMeta)
{
    ++frameNumber_;
    bufferMeta->pts_ = GetCurrentTime();
    bufferMeta->frameNum_ = frameNumber_.load();
    bufferMeta->dataType_ = GetValueFromParams(Tag::MEDIA_DESCRIPTION, AVT_PARAM_BUFFERMETA_DATATYPE);
}

BufferDataType DscreenInputPlugin::GetValueFromParams(Tag tag, std::string keyWord)
{
    auto iter = paramsMap_.find(tag);
    if (iter == paramsMap_.end()) {
        DHLOGE("Tag not found.");
        return BufferDataType::UNKNOW;
    }
    json paramsJson = json::parse(Media::Plugin::AnyCast<std::string>(paramsMap_[tag]), nullptr, false);
    if (paramsJson.is_discarded() || !IsUInt32(paramsJson, keyWord)) {
        DHLOGE("The paramsJson is invalid.");
        return BufferDataType::UNKNOW;
    }
    return static_cast<BufferDataType>(paramsJson[keyWord]);
}

Status DscreenInputPlugin::SetCallback(Callback *cb)
{
    DHLOGI("SetCallBack.");
    return Status::OK;
}

Status DscreenInputPlugin::SetDataCallback(AVDataCallback callback)
{
    DHLOGI("SetDataCallback.");
    return Status::OK;
}

}
}