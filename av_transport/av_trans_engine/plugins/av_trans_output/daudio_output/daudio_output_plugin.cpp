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

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<DHPlugin> DaudioOutputPluginCreator(const std::string& name)
{
    return std::make_shared<DaudioOutputPlugin>(name);
}

Status DaudioOutputRegister(const std::shared_ptr<Register> &reg)
{
    DHLOGI("DaudioOutputRegister enter.");

    DHPluginDef definition;
    definition.name = "AVTransDaudioOutputPlugin";
    definition.description = "Send audio playback and frame rate control.";
    definition.rank = 100;
    definition.protocol.emplace_back(ProtocolType::STREAM);
    definition.inputType = SrcInputType::AUD_ES;
    definition.creator = DaudioOutputPluginCreator;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTranseDaudioOutput, LicenseType::APACHE_V2, DaudioOutputRegister, [] {});

DaudioOutputPlugin::DaudioOutputPlugin(std::string name)
    : DHPlugin(std::move(name))
{
    DHLOGI("DaudioOutputPlugin ctor.");
}

DaudioOutputPlugin::~DaudioOutputPlugin()
{
    DHLOGI("DaudioOutputPlugin dtor.");
}

Status DaudioOutputPlugin::Init()
{
    DHLOGI("DaudioOutputPlugin enter.");
    OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DaudioOutputPlugin::Deinit()
{
    DHLOGI("Deinit DaudioOutputPlugin.");
    return Reset();
}

Status DaudioOutputPlugin::Prepare()
{
    DHLOGI("Prepare enter");
    OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    if (sendPlayTask_ == nullptr) {
        sendPlayTask_ = std::make_shared<OHOS::Media::OSAL::Task>("sendPlayTask_");
        sendPlayTask_->RegisterHandler([this] { HandleData(); });
    }
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
    paramsMap_.clear();
    DataQueueClear(outputBuffer_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DaudioOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    DHLOGI("GetParameter enter.");
    {
        Media::OSAL::ScopedLock lock(operationMutes_);
        auto iter = paramsMap_.find(tag);
        if (iter != paramsMap_.end()) {
            value = iter->second;
            return Status::OK;
        }
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DaudioOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    DHLOGI("SetParameter enter.");
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
    sendPlayTask_->Stop();
    DataQueueClear(outputBuffer_);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DaudioOutputPlugin::SetCallback(Callback *cb)
{
    DHLOGI("SetCallBack enter");
    OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        DHLOGE("SetCallBack failed, cb is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventcallback_ = cb;
    return Status::OK;
}

Status DaudioOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    DHLOGI("SetDataCallback enter.");
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
        //dataCallback_->OnDataCallback(buffer);
    }
}

void DaudioOutputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &q) {
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, q);
}

//待补充
Status DaudioOutputPlugin::StartOutputQueue()
{
    DHLOGI("StartOutputQueue enter.");
    return Status::OK;
}

//待补充帧率控制算法
Status DaudioOutputPlugin::ControlFrameRate(const int64_t timestamp)
{
    DHLOGI("ControlFrameRate enter.");
    return Status::OK;
}
} // namespace DistributedHardware
} // namespace OHOS
