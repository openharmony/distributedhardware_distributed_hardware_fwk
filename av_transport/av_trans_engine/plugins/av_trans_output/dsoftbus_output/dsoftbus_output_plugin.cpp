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
#include "dsoftbus_output_plugin.h"

#include "output_channel.h"
#include "foundation/utils/constants.h"

namespace OHOS {
namespace DistributedHardware {

std::shared_ptr<AvTransOutputPlugin> DsoftbusOutputPluginCreator(const std::string &name)
{
    return std::make_shared<DsoftbusOutputPlugin>(name);
}

Status DsoftbusOutputRegister(const std::shared_ptr<Register> &reg)
{
    AvTransOutputPluginDef definition;
    definition.name = "AVTransDsoftbusOutputPlugin";
    definition.description = "Video transport to dsoftbus";
    definition.rank = 100;
    definition.protocol.emplace_back(ProtocolType::STREAM);
    definition.inputType = SrcInputType::D_SOFTBUS;
    definition.pluginType = PluginType::AVTRANS_OUTPUT;
    Capability outCap(Media::MEDIA_MIME_VIDEO_H264);
    definition.creator = DsoftbusOutputPluginCreator;
    return reg->AddPlugin(definition);
}

PLUGIN_DEFINITION(AVTransDsoftbusOutput, LicenseType::APACHE_V2, DsoftbusOutputRegister, [] {});

DsoftbusOutputPlugin::DsoftbusOutputPlugin(std::string name)
    : AvTransOutputPlugin(std::move(name))
{
    DHLOGI("ctor.");
}

DsoftbusOutputPlugin::~DsoftbusOutputPlugin()
{
    DHLOGI("dtor.");
}

Status DsoftbusOutputPlugin::Init()
{
    DHLOGI("Init Dsoftbus Output Plugin.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusOutputPlugin::Deinit()
{
    DHLOGI("Deinit Dsoftbus Output Plugin.");
    return Reset();
}

Status DsoftbusOutputPlugin::Prepare()
{
    DHLOGI("Prepare");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }

    auto dsoftbusOutputPlugin = std::shared_ptr<DsoftbusOutputPlugin>(shared_from_this());
    int32_t ret = OutputChannel::GetInstance().Initialize(ownerName_, dsoftbusOutputPlugin);
    if (ret != DH_AVT_SUCCESS) {
        DHLOGE("Create Session Server failed ret: %s.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }

    if (!bufferPopTask_) {
        bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
        bufferPopTask_->RegisterHandler([this] { FeedChannelData(); });
    }
    state_ = State::PREPARED;
    return Status::OK;
}

Status DsoftbusOutputPlugin::Reset()
{
    DHLOGI("Reset");
    Media::OSAL::ScopedLock lock(operationMutes_);
    paramsMap_.clear();
    if (bufferPopTask_) {
        bufferPopTask_->Stop();
        bufferPopTask_.reset();
    }
    DataQueueClear(dataQueue_);
    eventsCb_ = nullptr;
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusOutputPlugin::Start()
{
    DHLOGI("Dsoftbus Output Plugin start.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    Status ret = OpenSoftbusChannel();
    if (ret != Status::OK) {
        DHLOGE("OpenSoftbusSession failed.");
        return Status::ERROR_INVALID_OPERATION;
    }
    DataQueueClear(dataQueue_);
    bufferPopTask_->Start();
    state_ = State::RUNNING;
    return Status::OK;
}

Status DsoftbusOutputPlugin::Stop()
{
    DHLOGI("Dsoftbus Output Plugin stop.");
    if (state_ != State::RUNNING) {
        DHLOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }

    bufferPopTask_->Stop();
    DataQueueClear(dataQueue_);
    CloseSoftbusChannel();
    state_ = State::PREPARED;
    return Status::OK;
}

Status DsoftbusOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    DHLOGI("GetParameter");
    auto res = paramsMap_.find(tag);
    if (res == paramsMap_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DsoftbusOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    DHLOGI("SetParameter");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (tag == Tag::MEDIA_DESCRIPTION) {
        ParseChannelDescription(Plugin::AnyCast<std::string>(value), ownerName_, peerDevId_);
    }
    paramsMap_.insert(std::pair<Tag, ValueType>(tag, value));
    return Status::OK;
}

Status DsoftbusOutputPlugin::SetCallback(Callback *cb)
{
    DHLOGI("SetCallback");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        DHLOGE("SetCallback failed, cb is nullptr.");
        return Status::ERROR_INVALID_OPERATION;
    }
    eventsCb_ = cb;
    return Status::OK;
}

Status DsoftbusOutputPlugin::OpenSoftbusChannel()
{
    int32_t ret = OutputChannel::GetInstance().OpenDataChannel(peerDevId_);
    if (ret != DH_AVT_SUCCESS) {
        DHLOGE("Open softbus channel failed ret: %d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    return Status::OK;
}

Status DsoftbusOutputPlugin::OnSoftbusChannelOpened(int32_t sessionId, int32_t result)
{
    if (eventsCb_ == nullptr) {
        DHLOGE("event callback is null");
        return Status::ERROR_UNKNOWN;
    }
    if (result != DH_AVT_SUCCESS) {
        eventsCb_->OnEvent({PluginEventType::EVENT_CREATE_CHANNEL_FAIL});
        DHLOGE("open softbus session failed, sessionId: %d.", sessionId);
    } else {
        eventsCb_->OnEvent({PluginEventType::EVENT_CHANNEL_CREATED});
        DHLOGI("open softbus session success, sessionId: %d.", sessionId);
    }
    return Status::OK;
}

void DsoftbusOutputPlugin::CloseSoftbusChannel()
{
    int32_t ret = OutputChannel::GetInstance().CloseDataChannel();
    if (ret != DH_AVT_SUCCESS) {
        DHLOGE("Close softbus channle failed ret: %s.", ret);
    }
}

void DsoftbusOutputPlugin::OnSoftbusChannelClosed(int32_t sessionId)
{
    DHLOGI("Session is closed, sessionId: %d.", sessionId);
    if (eventsCb_ != nullptr) {
        eventsCb_->OnEvent({PluginEventType::EVENT_CHANNEL_CLOSED});
    }
}

Status DsoftbusOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    DHLOGI("Push Buffer to output plugin.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (buffer == nullptr || buffer->IsEmpty()) {
        DHLOGE("Buffer is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    while (dataQueue_.size() >= DATA_QUEUE_MAX_SIZE) {
        DHLOGE("Data queue overflow.");
        dataQueue_.pop();
    }
    dataQueue_.push(buffer);
    dataCond_.notify_all();
    return Status::OK;
}

void DsoftbusOutputPlugin::FeedChannelData()
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
        SendDataToSoftbus(buffer);
    }
}

Status DsoftbusOutputPlugin::SendDataToSoftbus(std::shared_ptr<Buffer> &buffer)
{
    json jsonObj;
    auto bufferMeta = buffer->GetBufferMeta();
    BufferMetaType metaType = bufferMeta->GetType();
    jsonObj[AVT_DATA_META_TYPE] = metaType;
    if (metaType == BufferMetaType::VIDEO) {
        auto hisAMeta = ReinterpretCastPointer<AVTransVideoBufferMeta>(bufferMeta);
        jsonObj[AVT_DATA_PARAM] = hisAMeta->MarshalVideoMeta();
    } else if (metaType == BufferMetaType::AUDIO) {
        auto hisAMeta = ReinterpretCastPointer<AVTransAudioBufferMeta>(bufferMeta);
        jsonObj[AVT_DATA_PARAM] = hisAMeta->MarshalAudioMeta();
    }
    std::string jsonStr = jsonObj.dump();
    auto bufferData = buffer->GetMemory();

    uint8_t *buf = reinterpret_cast<uint8_t *>(calloc(bufferData->GetSize(), sizeof(uint8_t)));
    if (buf == nullptr) {
        DHLOGE("SendDataToSoftbus: malloc memory failed");
        return Status::OK;
    }
    size_t size = bufferData->Read(buf, bufferData->GetSize());
    if (size <= 0) {
        DHLOGE("SendDataToSoftbus: memcpy memory failed");
        free(buf);
        return Status::OK;
    }

    StreamData data = {reinterpret_cast<char *>(buf), bufferData->GetSize()};
    StreamData ext = {const_cast<char *>(jsonStr.c_str()), jsonStr.length()};
    StreamFrameInfo frameInfo = {0};

    int32_t ret = OutputChannel::GetInstance().SendData(peerDevId_, &data, &ext, &frameInfo);
    if (ret != DH_AVT_SUCCESS) {
        DHLOGE("Send data to softbus failed.");
    }
    free(buf);
    return Status::OK;
}

void DsoftbusOutputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue) {
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DsoftbusOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    DHLOGI("SetDataCallback");
    return Status::OK;
}
}
}