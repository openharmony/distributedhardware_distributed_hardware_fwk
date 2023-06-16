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

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<AvTransOutputPlugin> DscreenOutputPluginCreator(const std::string &name)
{
    return std::make_shared<DscreenOutputPlugin>(name);
}

Status DscreenOutputRegister(const std::shared_ptr<Register> &reg)
{
    AvTransOutputPluginDef definition;
    definition.name = "AVTranseDscreenOutputPlugin";
    definition.description = "Send video display and frame rate control.";
    definition.rank = PLUGIN_RANK;
    definition.pluginType = PluginType::AVTRANS_OUTPUT;
    definition.creator = DscreenOutputPluginCreator;

    Capability inCap(Media::MEDIA_MIME_VIDEO_RAW);
    inCap.AppendDiscreteKeys<VideoPixelFormat>(
        Capability::Key::VIDEO_PIXEL_FORMAT, {VideoPixelFormat::RGBA});
    definition.inCaps.push_back(inCap);
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTransDscreenOutput, LicenseType::APACHE_V2, DscreenOutputRegister, [] {});

DscreenOutputPlugin::DscreenOutputPlugin(std::string name)
    : AvTransOutputPlugin(std::move(name))
{
    DHLOGI("ctor.");
}

DscreenOutputPlugin::~DscreenOutputPlugin()
{
    DHLOGI("dtor.");
}

Status DscreenOutputPlugin::Init()
{
    DHLOGI("Init.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DscreenOutputPlugin::Deinit()
{
    DHLOGI("Deinit.");
    return Reset();
}

Status DscreenOutputPlugin::Prepare()
{
    DHLOGI("Prepare");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    if (!sendDisplayTask_) {
        sendDisplayTask_ = std::make_shared<OHOS::Media::OSAL::Task>("sendDisplayTask");
        sendDisplayTask_->RegisterHandler([this] { HandleData(); });
    }
    state_ = State::PREPARED;
    return Status::OK;
}

Status DscreenOutputPlugin::Reset()
{
    DHLOGI("Reset");
    Media::OSAL::ScopedLock lock(operationMutes_);
    eventsCb_ = nullptr;
    if (sendDisplayTask_) {
        sendDisplayTask_->Stop();
        sendDisplayTask_.reset();
    }
    paramsMap_.clear();
    DataQueueClear(outputBuffer_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DscreenOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    auto iter = paramsMap_.find(tag);
    if (iter != paramsMap_.end()) {
        value = iter->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DscreenOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    paramsMap_.insert(std::make_pair(tag, value));
    return Status::OK;
}

Status DscreenOutputPlugin::Start()
{
    DHLOGI("Start");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    sendDisplayTask_->Start();
    DataQueueClear(outputBuffer_);
    state_ = State::RUNNING;
    return Status::OK;
}

Status DscreenOutputPlugin::Stop()
{
    DHLOGI("Stop");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::RUNNING) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    sendDisplayTask_->Stop();
    DataQueueClear(outputBuffer_);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DscreenOutputPlugin::SetCallback(Callback *cb)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        DHLOGE("SetCallback failed, cb is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventsCb_ = cb;
    DHLOGI("SetCallback success.");
    return Status::OK;
}

Status DscreenOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    dataCb_ = callback;
    DHLOGI("SetDataCallback success.");
    return Status::OK;
}

Status DscreenOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    DHLOGI("Queue Output AVBuffer.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    uint32_t frameNumber = 1;
    if (buffer->GetBufferMeta()->IsExist(Tag::USER_FRAME_NUMBER)) {
        frameNumber = Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_NUMBER));
    }
    DHLOGI("buffer pts: %ld, bufferLen: %zu, frameNumber: %zu", buffer->pts, buffer->GetMemory()->GetSize(),
        frameNumber);

    if (buffer == nullptr || buffer->IsEmpty()) {
        DHLOGE("AVBuffer is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    while (outputBuffer_.size() >= DATA_QUEUE_MAX_SIZE) {
        DHLOGE("outputBuffer_ queue overflow.");
        outputBuffer_.pop();
    }
    outputBuffer_.push(buffer);
    dataCond_.notify_all();
    return Status::OK;
}

void DscreenOutputPlugin::HandleData()
{
    while (state_ == State::RUNNING) {
        std::shared_ptr<Buffer> buffer;
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
        dataCb_(buffer);
    }
}

void DscreenOutputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue)
{
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DscreenOutputPlugin::StartOutputQueue()
{
    DHLOGI("StartOutputQueue.");
    return Status::OK;
}

Status DscreenOutputPlugin::ControlFrameRate(const int64_t timestamp)
{
    DHLOGI("ControlFrameRate.");
    return Status::OK;
}
}
}
