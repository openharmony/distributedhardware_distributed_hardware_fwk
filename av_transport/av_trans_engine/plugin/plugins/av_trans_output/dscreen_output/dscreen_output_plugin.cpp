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

#include "dscreen_output_plugin.h"

#include "foundation/utils/constants.h"
#include "plugin/factory/plugin_factory.h"
#include "plugin/interface/generic_plugin.h"

namespace OHOS {
namespace DistributedHardware {

GenericPluginDef CreateDscreenOutputPluginDef()
{
    GenericPluginDef definition;
    definition.name = "AVTransDscreenOutputPlugin";
    definition.pkgName = "AVTransDscreenOutputPlugin";
    definition.description = "Send video display and frame rate control.";
    definition.rank = PLUGIN_RANK;
    definition.creator = [] (const std::string& name) -> std::shared_ptr<AvTransOutputPlugin> {
        return std::make_shared<DscreenOutputPlugin>(name);
    };

    definition.pkgVersion = AVTRANS_OUTPUT_API_VERSION;
    definition.license = LicenseType::APACHE_V2;

    Capability inCap(Media::MEDIA_MIME_VIDEO_RAW);
    inCap.AppendDiscreteKeys<VideoPixelFormat>(
        Capability::Key::VIDEO_PIXEL_FORMAT, {VideoPixelFormat::NV12});
    definition.inCaps.push_back(inCap);
    return definition;
}

static AutoRegisterPlugin<DscreenOutputPlugin> g_registerPluginHelper(CreateDscreenOutputPluginDef());

DscreenOutputPlugin::DscreenOutputPlugin(std::string name)
    : AvTransOutputPlugin(std::move(name))
{
    AVTRANS_LOGI("ctor.");
}

DscreenOutputPlugin::~DscreenOutputPlugin()
{
    AVTRANS_LOGI("dtor.");
}

Status DscreenOutputPlugin::Init()
{
    AVTRANS_LOGI("Init.");
    InitOutputController();
    SetCurrentState(State::INITIALIZED);
    return Status::OK;
}

Status DscreenOutputPlugin::Deinit()
{
    AVTRANS_LOGI("Deinit.");
    return Reset();
}

Status DscreenOutputPlugin::Prepare()
{
    AVTRANS_LOGI("Prepare");
    {
        TRUE_RETURN_V_MSG_E((GetCurrentState() != State::INITIALIZED), Status::ERROR_WRONG_STATE,
            "The state is wrong.");
        TRUE_RETURN_V_MSG_E((!controller_), Status::ERROR_NULL_POINTER, "Controller is nullptr.");
        SetCurrentState(State::PREPARED);
    }
    controller_->PrepareControl();
    return Status::OK;
}

Status DscreenOutputPlugin::Reset()
{
    AVTRANS_LOGI("Reset");
    {
        SetCurrentState(State::INITIALIZED);
        eventsCb_ = nullptr;
        dataCb_ = nullptr;
    }
    controllerListener_ = nullptr;
    if (controller_) {
        controller_->ReleaseControl();
        controller_ = nullptr;
    }
    return Status::OK;
}

Status DscreenOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    TRUE_RETURN_V_MSG_E((!controller_), Status::ERROR_NULL_POINTER, "Controller is nullptr.");
    return controller_->GetParameter(tag, value);
}

Status DscreenOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    TRUE_RETURN_V_MSG_E((!controller_), Status::ERROR_NULL_POINTER, "Controller is nullptr.");
    return controller_->SetParameter(tag, value);
}

Status DscreenOutputPlugin::Start()
{
    AVTRANS_LOGI("Start");
    {
        TRUE_RETURN_V_MSG_E((GetCurrentState() != State::PREPARED), Status::ERROR_WRONG_STATE, "The state is wrong.");
        TRUE_RETURN_V_MSG_E((!controller_), Status::ERROR_NULL_POINTER, "Controller is nullptr.");
        SetCurrentState(State::RUNNING);
    }
    controller_->StartControl();
    return Status::OK;
}

Status DscreenOutputPlugin::Stop()
{
    AVTRANS_LOGI("Stop");
    {
        TRUE_RETURN_V_MSG_E((GetCurrentState() != State::RUNNING), Status::ERROR_WRONG_STATE, "The state is wrong.");
        TRUE_RETURN_V_MSG_E((!controller_), Status::ERROR_NULL_POINTER, "Controller is nullptr.");
        SetCurrentState(State::PREPARED);
    }
    controller_->StopControl();
    return Status::OK;
}

Status DscreenOutputPlugin::SetCallback(Callback *cb)
{
    TRUE_RETURN_V_MSG_E((!cb), Status::ERROR_NULL_POINTER, "SetCallback failed, cb is nullptr.");
    eventsCb_ = cb;
    AVTRANS_LOGI("SetCallback success.");
    return Status::OK;
}

Status DscreenOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    dataCb_ = callback;
    AVTRANS_LOGI("SetDataCallback success.");
    return Status::OK;
}

Status DscreenOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset)
{
    TRUE_RETURN_V_MSG_E((buffer == nullptr || buffer->IsEmpty()), Status::ERROR_NULL_POINTER,
        "AVBuffer is nullptr.");
    TRUE_RETURN_V_MSG_E((!controller_), Status::ERROR_NULL_POINTER, "Controller is nullptr.");
    controller_->PushData(buffer);
    return Status::OK;
}

void DscreenOutputPlugin::OnOutput(const std::shared_ptr<Plugin::Buffer>& data)
{
    auto bufferMeta = data->GetBufferMeta();
    uint32_t vFrameNumber = DEFAULT_INVALID_FRAME_NUM;
    if (bufferMeta->IsExist(Tag::USER_FRAME_NUMBER)) {
        vFrameNumber = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::USER_FRAME_NUMBER));
    }
    AVTRANS_LOGD("OnOutput vFrameNumber: %u.", vFrameNumber);
    dataCb_(data);
}

void DscreenOutputPlugin::InitOutputController()
{
    if (!controller_) {
        controller_ = std::make_unique<DScreenOutputController>();
    }
    if (!controllerListener_) {
        controllerListener_ = std::make_shared<OutputControllerListener>(shared_from_this());
    }
    controller_->RegisterListener(controllerListener_);
}
}
}
