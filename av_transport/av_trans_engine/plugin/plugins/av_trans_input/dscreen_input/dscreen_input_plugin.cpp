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
#include "plugin/factory/plugin_factory.h"
#include "plugin/interface/generic_plugin.h"

namespace OHOS {
namespace DistributedHardware {
using json = nlohmann::json;

GenericPluginDef CreateDscreenInputPluginDef()
{
    GenericPluginDef definition;
    definition.name = "AVTransDscreenInputPlugin";
    definition.pkgName = "AVTransDscreenInputPlugin";
    definition.description = "Video transport from dscreen service";
    definition.rank = PLUGIN_RANK;
    definition.creator = [] (const std::string& name) -> std::shared_ptr<AvTransInputPlugin> {
        return std::make_shared<DscreenInputPlugin>(name);
    };

    definition.pkgVersion = AVTRANS_INPUT_API_VERSION;
    definition.license = LicenseType::APACHE_V2;

    Capability outCap(Media::MEDIA_MIME_VIDEO_RAW);
    outCap.AppendDiscreteKeys<VideoPixelFormat>(
        Capability::Key::VIDEO_PIXEL_FORMAT, {VideoPixelFormat::RGBA});
    definition.outCaps.push_back(outCap);
    return definition;
}

static AutoRegisterPlugin<DscreenInputPlugin> g_registerPluginHelper(CreateDscreenInputPluginDef());

DscreenInputPlugin::DscreenInputPlugin(std::string name)
    : AvTransInputPlugin(std::move(name))
{
    AVTRANS_LOGI("ctor.");
}

DscreenInputPlugin::~DscreenInputPlugin()
{
    AVTRANS_LOGI("dtor.");
}

Status DscreenInputPlugin::Init()
{
    AVTRANS_LOGI("Init.");
    frameNumber_.store(0);
    return Status::OK;
}

Status DscreenInputPlugin::Deinit()
{
    AVTRANS_LOGI("Deinit.");
    return Reset();
}

Status DscreenInputPlugin::Reset()
{
    AVTRANS_LOGI("Reset");
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    paramsMap_.clear();
    frameNumber_.store(0);
    return Status::OK;
}

Status DscreenInputPlugin::Pause()
{
    AVTRANS_LOGD("Pause not supported.");
    return Status::OK;
}

Status DscreenInputPlugin::Resume()
{
    AVTRANS_LOGD("Resume not supported.");
    return Status::OK;
}

Status DscreenInputPlugin::GetParameter(Tag tag, ValueType &value)
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    auto res = paramsMap_.find(tag);
    if (res != paramsMap_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DscreenInputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    paramsMap_.insert(std::pair<Tag, ValueType>(tag, value));
    if (tag == Plugin::Tag::USER_SHARED_MEMORY_FD) {
        sharedMemory_ = UnmarshalSharedMemory(Media::Plugin::AnyCast<std::string>(value));
    }
    return Status::OK;
}

Status DscreenInputPlugin::PushData(const std::string& inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    if (!buffer || buffer->IsEmpty()) {
        AVTRANS_LOGE("buffer is nullptr or empty.");
        return Status::ERROR_NULL_POINTER;
    }

    auto bufferMeta = buffer->GetBufferMeta();
    if (!bufferMeta || bufferMeta->GetType() != BufferMetaType::VIDEO) {
        AVTRANS_LOGE("bufferMeta is nullptr or empty.");
        return Status::ERROR_NULL_POINTER;
    }

    ++frameNumber_;
    bufferMeta->SetMeta(Tag::USER_FRAME_NUMBER, frameNumber_.load());
    AVTRANS_LOGI("Push video buffer pts: %{public}ld, bufferLen: %{public}d, frameNumber: %{public}u.",
        buffer->pts, buffer->GetMemory()->GetSize(),
        Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::USER_FRAME_NUMBER)));

    if ((sharedMemory_.fd > 0) && (sharedMemory_.size > 0) && !sharedMemory_.name.empty()) {
        int64_t audioTimestamp = 0;
        uint32_t audioFrameNum = 0;
        int32_t ret = ReadFrameInfoFromMemory(sharedMemory_, audioFrameNum, audioTimestamp);
        if ((ret == DH_AVT_SUCCESS) && (audioFrameNum > 0) && (audioTimestamp > 0)) {
            bufferMeta->SetMeta(Tag::MEDIA_START_TIME, audioTimestamp);
            bufferMeta->SetMeta(Tag::AUDIO_SAMPLE_PER_FRAME, audioFrameNum);
        }
    }

    return Status::OK;
}

Status DscreenInputPlugin::SetCallback(Callback *cb)
{
    AVTRANS_LOGI("SetCallBack.");
    return Status::OK;
}

Status DscreenInputPlugin::SetDataCallback(AVDataCallback callback)
{
    AVTRANS_LOGI("SetDataCallback.");
    return Status::OK;
}

}
}