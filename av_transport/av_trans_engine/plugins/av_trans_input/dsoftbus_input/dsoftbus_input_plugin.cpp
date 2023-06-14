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

#include "dsoftbus_input_plugin.h"

#include "foundation/utils/constants.h"
#include "input_channel.h"
#include "plugin/common/share_memory.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<AvTransInputPlugin> DsoftbusInputPluginCreator(const std::string &name)
{
    return std::make_shared<DsoftbusInputPlugin>(name);
}

Status DsoftbusInputRegister(const std::shared_ptr<Register> &reg)
{
    AvTransInputPluginDef definition;
    definition.name = "AVTransDsoftbusInputPlugin";
    definition.description = "Video transport from dsoftbus";
    definition.rank = 100;
    definition.protocol.emplace_back(ProtocolType::STREAM);
    definition.inputType = SrcInputType::D_SOFTBUS;
    definition.pluginType = PluginType::AVTRANS_INPUT;
    definition.creator = DsoftbusInputPluginCreator;
    Capability outCap(Media::MEDIA_MIME_VIDEO_H264);
    definition.outCaps.push_back(outCap);
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTransDsoftbusInput, LicenseType::APACHE_V2, DsoftbusInputRegister, [] {});

DsoftbusInputPlugin::DsoftbusInputPlugin(std::string name)
    : AvTransInputPlugin(std::move(name))
{
    DHLOGI("ctor");
}

DsoftbusInputPlugin::~DsoftbusInputPlugin()
{
    DHLOGI("detr");
}

Status DsoftbusInputPlugin::Init()
{
    DHLOGI("Init Dsoftbus Input Plugin.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusInputPlugin::Deinit()
{
    DHLOGI("Deinit Dsoftbus Input Plugin.");
    return Reset();
}

Status DsoftbusInputPlugin::Prepare()
{
    DHLOGI("Prepare");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    auto dsoftbusInputPlugin = std::shared_ptr<DsoftbusInputPlugin>(shared_from_this());
    int32_t ret = InputChannel::GetInstance().Initialize(ownerName_, dsoftbusInputPlugin);
    if (ret != DH_AVT_SUCCESS) {
        DHLOGE("Create Session Server failed ret: %d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }

    if (!bufferPopTask_) {
        bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
        bufferPopTask_->RegisterHandler([this] { HandleData(); });
    }
    state_ = State::PREPARED;
    return Status::OK;
}

Status DsoftbusInputPlugin::Reset()
{
    DHLOGI("Reset");
    Media::OSAL::ScopedLock lock(operationMutes_);
    eventsCb_ = nullptr;
    dataCb_ = nullptr;
    paramsMap_.clear();
    if (bufferPopTask_) {
        bufferPopTask_->Stop();
        bufferPopTask_.reset();
    }
    DataQueueClear(dataQueue_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusInputPlugin::Start()
{
    DHLOGI("Start");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    DataQueueClear(dataQueue_);
    bufferPopTask_->Start();
    state_ = State::RUNNING;
    return Status::OK;
}

Status DsoftbusInputPlugin::Stop()
{
    DHLOGI("Stop");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::RUNNING) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    bufferPopTask_->Stop();
    DataQueueClear(dataQueue_);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DsoftbusInputPlugin::GetParameter(Tag tag, ValueType &value)
{
    DHLOGI("GetParameter");
    auto res = paramsMap_.find(tag);
    if (res == paramsMap_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DsoftbusInputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (tag == Tag::MEDIA_DESCRIPTION) {
        ParseChannelDescription(Plugin::AnyCast<std::string>(value), ownerName_, peerDevId_);
    }
    paramsMap_.insert(std::pair<Tag, ValueType>(tag, value));
    return Status::OK;
}

Status DsoftbusInputPlugin::SetCallback(Callback *cb)
{
    DHLOGI("SetCallBack.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        DHLOGE("SetCallBack failed, callback is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventsCb_ = cb;
    DHLOGI("SetCallBack success.");
    return Status::OK;
}

Status DsoftbusInputPlugin::SetDataCallback(AVDataCallback callback)
{
    DHLOGI("SetDataCallback.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    dataCb_ = callback;
    DHLOGI("SetDataCallback success.");
    return Status::OK;
}

Status DsoftbusInputPlugin::OnSoftbusChannelOpened(int32_t sessionId, int32_t result)
{
    if (result != DH_AVT_SUCCESS) {
        DHLOGE("Session is opened failed, sessionId: %d.", sessionId);
        if (eventsCb_ != nullptr) {
            eventsCb_->OnEvent({PluginEventType::EVENT_CREATE_CHANNEL_FAIL}); // 通知上层通道打开失败
        }
        return Status::ERROR_UNKNOWN;
    }
    DHLOGI("Session is opened success, sessionId: %d.", sessionId);
    if (eventsCb_ != nullptr) {
        eventsCb_->OnEvent({PluginEventType::EVENT_CHANNEL_CREATED}); // 通知上层通道打开成功
    }
    return Status::OK;
}

void DsoftbusInputPlugin::OnSoftbusChannelClosed(int32_t sessionId)
{
    DHLOGI("Session is closed, sessionId: %d.", sessionId);
    if (eventsCb_ != nullptr) {
        eventsCb_->OnEvent({PluginEventType::EVENT_CHANNEL_CLOSED}); // 通知上层通道关闭
    }
}

void DsoftbusInputPlugin::GetJsonMeta(const std::shared_ptr<Buffer> &buffer, const json &resMsg)
{
    if (resMsg.is_discarded()) {
        DHLOGE("The resMsg parse failed");
        return;
    }
    TRUE_RETURN(!IsUInt32(resMsg, AVT_DATA_META_TYPE));
    uint32_t metaType = resMsg[AVT_DATA_META_TYPE];
    DHLOGI("The resMsg datatype: %u.", metaType);
    if (metaType == static_cast<uint32_t>(MetaType::VIDEO)) {
        auto meta = std::make_shared<AVTransVideoBufferMeta>();
        meta->UnmarshalVideoMeta(resMsg[AVT_DATA_PARAM]);
        buffer->SetBufferMeta(meta);
    } else if (metaType == static_cast<uint32_t>(MetaType::AUDIO)) {
        auto meta = std::make_shared<AVTransAudioBufferMeta>();
        meta->UnmarshalAudioMeta(resMsg[AVT_DATA_PARAM]);
        buffer->SetBufferMeta(meta);
    }
}

void DsoftbusInputPlugin::OnStreamDataReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *frameInfo)
{
    auto buffer = Buffer::CreateDefaultBuffer(BufferMetaType::VIDEO, data->bufLen);
    auto bufData = buffer->GetMemory();
    if (bufData->GetMemoryType() != Plugin::MemoryType::VIRTUAL_ADDR || bufData->GetCapacity() <= 0) {
        DHLOGE("Get Memory error.");
        return;
    }
    auto writeSize = bufData->Write(reinterpret_cast<uint8_t *>(data->buf), data->bufLen);
    if (static_cast<ssize_t>(writeSize) != data->bufLen) {
        DHLOGE("write buffer data failed.");
        return;
    }

    std::string message(reinterpret_cast<const char *>(ext), ext->bufLen);
    json resMsg = json::parse(message, nullptr, false);
    GetJsonMeta(buffer, resMsg);
    DataEnqueue(buffer);
}

void DsoftbusInputPlugin::DataEnqueue(std::shared_ptr<Buffer> &buffer)
{
    while (dataQueue_.size() >= DATA_QUEUE_MAX_SIZE) {
        DHLOGE("Data queue overflow.");
        dataQueue_.pop();
    }
    dataQueue_.push(buffer);
    dataCond_.notify_all();
}

void DsoftbusInputPlugin::HandleData()
{
    while (state_ == State::RUNNING) {
        std::shared_ptr<Buffer> buffer;
        {
            std::unique_lock<std::mutex> lock(dataQueueMtx_);
            dataCond_.wait(lock, [this]() { return !dataQueue_.empty(); });
            if (dataQueue_.empty()) {
                DHLOGD("Data queue is empty.");
                continue;
            }
            buffer = dataQueue_.front();
            dataQueue_.pop();
        }
        if (buffer == nullptr) {
            DHLOGE("Data is null");
            continue;
        }
        dataCb_(buffer);
    }
}

void DsoftbusInputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue) {
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DsoftbusInputPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    (void) buffer;
    DHLOGI("Push Data");
    return Status::OK;
}

void DsoftbusInputPlugin::OnBytesReceived(int32_t sessionId, const void *data, int32_t dataLen)
{
    (void) data;
    (void) dataLen;
    DHLOGD("OnBytesReceived, sessionId:%d.", sessionId);
}

}
}