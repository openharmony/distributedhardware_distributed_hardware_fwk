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
#include "plugin/common/share_memory.h"
#include "plugin/common/plugin_caps_builder.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<AvTransInputPlugin> DsoftbusInputPluginCreator(const std::string &name)
{
    return std::make_shared<DsoftbusInputPlugin>(name);
}

Status DsoftbusInputRegister(const std::shared_ptr<Register> &reg)
{
    int32_t capNum = 2;
    for (int i = 0; i < capNum; i++) {
        AvTransInputPluginDef definition;
        definition.name = "AVTransDsoftbusInputPlugin_H264";
        definition.description = "Video transport from dsoftbus";
        definition.rank = PLUGIN_RANK;
        definition.pluginType = PluginType::AVTRANS_INPUT;
        definition.creator = DsoftbusInputPluginCreator;

        CapabilityBuilder capBuilder;
        capBuilder.SetMime(Media::MEDIA_MIME_VIDEO_H264);
        if (i == 1) {
            definition.name = "AVTransDsoftbusInputPlugin_H265";
            capBuilder.SetMime(Media::MEDIA_MIME_VIDEO_H265);
        }
        definition.outCaps.push_back(capBuilder.Build());
        reg->AddPlugin(definition);
    }
    return Status::OK;
}

PLUGIN_DEFINITION(AVTransDsoftbusInput, LicenseType::APACHE_V2, DsoftbusInputRegister, [] {});

DsoftbusInputPlugin::DsoftbusInputPlugin(std::string name)
    : AvTransInputPlugin(std::move(name))
{
    AVTRANS_LOGI("ctor");
}

DsoftbusInputPlugin::~DsoftbusInputPlugin()
{
    AVTRANS_LOGI("dtor");
}

Status DsoftbusInputPlugin::Init()
{
    AVTRANS_LOGI("Init");
    Media::OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusInputPlugin::Deinit()
{
    AVTRANS_LOGI("Deinit");
    return Reset();
}

Status DsoftbusInputPlugin::Prepare()
{
    AVTRANS_LOGI("Prepare");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }

    sessionName_ = ownerName_ + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX;
    int32_t ret = SoftbusChannelAdapter::GetInstance().CreateChannelServer(TransName2PkgName(ownerName_), sessionName_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Create Session Server failed ret: %d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Register channel listener failed ret: %d.", ret);
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
    AVTRANS_LOGI("Reset");
    Media::OSAL::ScopedLock lock(operationMutes_);
    eventsCb_ = nullptr;
    dataCb_ = nullptr;
    paramsMap_.clear();
    if (bufferPopTask_) {
        bufferPopTask_->Stop();
        bufferPopTask_.reset();
    }
    DataQueueClear(dataQueue_);
    SoftbusChannelAdapter::GetInstance().RemoveChannelServer(TransName2PkgName(ownerName_), sessionName_);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, peerDevId_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusInputPlugin::Start()
{
    AVTRANS_LOGI("Start");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    DataQueueClear(dataQueue_);
    bufferPopTask_->Start();
    state_ = State::RUNNING;
    return Status::OK;
}

Status DsoftbusInputPlugin::Stop()
{
    AVTRANS_LOGI("Stop");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::RUNNING) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    bufferPopTask_->Pause();
    DataQueueClear(dataQueue_);
    state_ = State::PREPARED;
    return Status::OK;
}

Status DsoftbusInputPlugin::GetParameter(Tag tag, ValueType &value)
{
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
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        AVTRANS_LOGE("SetCallBack failed, callback is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventsCb_ = cb;
    AVTRANS_LOGI("SetCallBack success.");
    return Status::OK;
}

Status DsoftbusInputPlugin::SetDataCallback(AVDataCallback callback)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    dataCb_ = callback;
    AVTRANS_LOGI("SetDataCallback success.");
    return Status::OK;
}

void DsoftbusInputPlugin::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter, event type: %d", event.type);
    if (eventsCb_ == nullptr) {
        AVTRANS_LOGE("OnChannelEvent failed, event callback is nullptr.");
        return;
    }
    switch (event.type) {
        case EventType::EVENT_CHANNEL_OPENED: {
            eventsCb_->OnEvent({PluginEventType::EVENT_CHANNEL_OPENED});
            break;
        }
        case EventType::EVENT_CHANNEL_OPEN_FAIL: {
            eventsCb_->OnEvent({PluginEventType::EVENT_CHANNEL_OPEN_FAIL});
            break;
        }
        case EventType::EVENT_CHANNEL_CLOSED: {
            eventsCb_->OnEvent({PluginEventType::EVENT_CHANNEL_CLOSED});
            break;
        }
        default:
            AVTRANS_LOGE("Unsupported event type.");
    }
}

void DsoftbusInputPlugin::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    std::string message(reinterpret_cast<const char *>(ext->buf), ext->bufLen);
    AVTRANS_LOGI("Receive message : %s", message.c_str());

    json resMsg = json::parse(message, nullptr, false);
    TRUE_RETURN(resMsg.is_discarded(), "The resMsg parse failed");
    TRUE_RETURN(!IsUInt32(resMsg, AVT_DATA_META_TYPE), "invalid data type");
    uint32_t metaType = resMsg[AVT_DATA_META_TYPE];
    AVTRANS_LOGI("The resMsg datatype: %u.", metaType);

    auto buffer = CreateBuffer(metaType, data, resMsg);
    DataEnqueue(buffer);
}

std::shared_ptr<Buffer> DsoftbusInputPlugin::CreateBuffer(uint32_t metaType,
    const StreamData *data, const json &resMsg)
{
    auto buffer = Buffer::CreateDefaultBuffer(static_cast<BufferMetaType>(metaType), data->bufLen);
    auto bufData = buffer->GetMemory();

    auto writeSize = bufData->Write(reinterpret_cast<const uint8_t *>(data->buf), data->bufLen, 0);
    if (static_cast<ssize_t>(writeSize) != data->bufLen) {
        AVTRANS_LOGE("write buffer data failed.");
        return buffer;
    }

    auto meta = std::make_shared<AVTransVideoBufferMeta>();
    meta->UnmarshalVideoMeta(resMsg[AVT_DATA_PARAM]);
    buffer->pts = meta->pts_;
    buffer->GetBufferMeta()->SetMeta(Tag::USER_FRAME_NUMBER, meta->frameNum_);
    AVTRANS_LOGI("buffer pts: %ld, bufferLen: %zu, frameNumber: %zu", buffer->pts, buffer->GetMemory()->GetSize(),
        meta->frameNum_);
    return buffer;
}

void DsoftbusInputPlugin::DataEnqueue(std::shared_ptr<Buffer> &buffer)
{
    while (dataQueue_.size() >= DATA_QUEUE_MAX_SIZE) {
        AVTRANS_LOGE("Data queue overflow.");
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
                AVTRANS_LOGD("Data queue is empty.");
                continue;
            }
            buffer = dataQueue_.front();
            dataQueue_.pop();
        }
        if (buffer == nullptr) {
            AVTRANS_LOGE("Data is null");
            continue;
        }
        dataCb_(buffer);
    }
}

void DsoftbusInputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue)
{
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DsoftbusInputPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    (void) buffer;
    AVTRANS_LOGI("Push Data");
    return Status::OK;
}
}
}