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

#include "dsoftbus_input_audio_plugin.h"

#include "foundation/utils/constants.h"
#include "plugin/common/share_memory.h"
#include "plugin/common/plugin_caps_builder.h"
#include "plugin/factory/plugin_factory.h"
#include "plugin/interface/generic_plugin.h"

namespace OHOS {
namespace DistributedHardware {

GenericPluginDef CreateDsoftbusInputAudioPluginDef()
{
    GenericPluginDef definition;
    definition.name = "AVTransDsoftbusInputAudioPlugin";
    definition.pkgName = "AVTransDsoftbusInputAudioPlugin";
    definition.description = "Audio transport from dsoftbus";
    definition.rank = PLUGIN_RANK;
    definition.creator = [] (const std::string& name) -> std::shared_ptr<AvTransInputPlugin> {
        return std::make_shared<DsoftbusInputAudioPlugin>(name);
    };

    definition.pkgVersion = AVTRANS_INPUT_API_VERSION;
    definition.license = LicenseType::APACHE_V2;

    CapabilityBuilder capBuilder;
    capBuilder.SetMime(OHOS::Media::MEDIA_MIME_AUDIO_AAC);
    DiscreteCapability<uint32_t> values = {8000, 11025, 12000, 16000,
        22050, 24000, 32000, 44100, 48000, 64000, 96000};
    capBuilder.SetAudioSampleRateList(values);
    definition.outCaps.push_back(capBuilder.Build());
    return definition;
}

static AutoRegisterPlugin<DsoftbusInputAudioPlugin> g_registerPluginHelper(CreateDsoftbusInputAudioPluginDef());

DsoftbusInputAudioPlugin::DsoftbusInputAudioPlugin(std::string name)
    : AvTransInputPlugin(std::move(name))
{
    AVTRANS_LOGI("ctor");
}

DsoftbusInputAudioPlugin::~DsoftbusInputAudioPlugin()
{
    AVTRANS_LOGI("dtor");
}

Status DsoftbusInputAudioPlugin::Init()
{
    AVTRANS_LOGI("Init");
    SetCurrentState(State::INITIALIZED);
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::Deinit()
{
    AVTRANS_LOGI("Deinit");
    return Reset();
}

Status DsoftbusInputAudioPlugin::Prepare()
{
    AVTRANS_LOGI("Prepare");
    if (GetCurrentState() != State::INITIALIZED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }

    sessionName_ = ownerName_ + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX;
    int32_t ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Register channel listener failed ret: %{public}d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    ret = SoftbusChannelAdapter::GetInstance().CreateChannelServer(TransName2PkgName(ownerName_), sessionName_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Create Session Server failed ret: %{public}d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    
    if (!bufferPopTask_) {
        bufferPopTask_ = std::make_shared<Media::OSAL::Task>("audioBufferQueuePopThread");
        bufferPopTask_->RegisterHandler([this] { HandleData(); });
    }
    SetCurrentState(State::PREPARED);
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::Reset()
{
    AVTRANS_LOGI("Reset");
    eventsCb_ = nullptr;
    dataCb_ = nullptr;
    {
        std::lock_guard<std::mutex> lock(paramsMapMutex_);
        paramsMap_.clear();
    }
    if (bufferPopTask_) {
        isrunning_.store(false);
        bufferPopTask_->Stop();
        bufferPopTask_.reset();
    }
    DataQueueClear(dataQueue_);
    SoftbusChannelAdapter::GetInstance().RemoveChannelServer(TransName2PkgName(ownerName_), sessionName_);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, peerDevId_);
    SetCurrentState(State::INITIALIZED);
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::Start()
{
    AVTRANS_LOGI("Start");
    if (GetCurrentState() != State::PREPARED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    DataQueueClear(dataQueue_);
    isrunning_.store(true);
    bufferPopTask_->Start();
    SetCurrentState(State::RUNNING);
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::Stop()
{
    AVTRANS_LOGI("Stop");
    if (GetCurrentState() != State::RUNNING) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    SetCurrentState(State::PREPARED);
    isrunning_.store(false);
    DataQueueClear(dataQueue_);
    bufferPopTask_->Stop();
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::Pause()
{
    AVTRANS_LOGD("Pause not supported.");
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::Resume()
{
    AVTRANS_LOGD("Resume not supported.");
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::GetParameter(Tag tag, ValueType &value)
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    auto res = paramsMap_.find(tag);
    if (res != paramsMap_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DsoftbusInputAudioPlugin::SetParameter(Tag tag, const ValueType &value)
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    if (tag == Tag::MEDIA_DESCRIPTION) {
        ParseChannelDescription(Plugin::AnyCast<std::string>(value), ownerName_, peerDevId_);
    }
    paramsMap_.insert(std::pair<Tag, ValueType>(tag, value));
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::SetCallback(Callback *cb)
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    if (cb == nullptr) {
        AVTRANS_LOGE("SetCallBack failed, callback is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventsCb_ = cb;
    AVTRANS_LOGI("SetCallBack success.");
    return Status::OK;
}

Status DsoftbusInputAudioPlugin::SetDataCallback(AVDataCallback callback)
{
    std::lock_guard<std::mutex> lock(paramsMapMutex_);
    dataCb_ = callback;
    AVTRANS_LOGI("SetDataCallback success.");
    return Status::OK;
}

void DsoftbusInputAudioPlugin::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter, event type: %{public}d", event.type);
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

void DsoftbusInputAudioPlugin::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    std::string message(reinterpret_cast<const char *>(ext->buf), ext->bufLen);
    AVTRANS_LOGI("Receive message : %{public}s", message.c_str());

    cJSON *resMsg = cJSON_Parse(message.c_str());
    if (resMsg == nullptr) {
        AVTRANS_LOGE("The resMsg parse failed.");
        return;
    }
    if (!IsUInt32(resMsg, AVT_DATA_META_TYPE)) {
        AVTRANS_LOGE("Invalid data type.");
        cJSON_Delete(resMsg);
        return;
    }
    cJSON *typeItem = cJSON_GetObjectItem(resMsg, AVT_DATA_META_TYPE.c_str());
    if (typeItem == nullptr) {
        cJSON_Delete(resMsg);
        return;
    }
    uint32_t metaType = typeItem->valueint;
    auto buffer = CreateBuffer(metaType, data, resMsg);
    DataEnqueue(buffer);
    cJSON_Delete(resMsg);
}

std::shared_ptr<Buffer> DsoftbusInputAudioPlugin::CreateBuffer(uint32_t metaType,
    const StreamData *data, const cJSON *resMsg)
{
    auto buffer = Buffer::CreateDefaultBuffer(static_cast<BufferMetaType>(metaType), data->bufLen);
    auto bufData = buffer->GetMemory();

    auto writeSize = bufData->Write(reinterpret_cast<const uint8_t *>(data->buf), data->bufLen, 0);
    if (static_cast<ssize_t>(writeSize) != data->bufLen) {
        AVTRANS_LOGE("write buffer data failed.");
        return buffer;
    }
    cJSON *paramItem = cJSON_GetObjectItem(resMsg, AVT_DATA_PARAM.c_str());
    if (paramItem == NULL) {
        return nullptr;
    }
    auto meta = std::make_shared<AVTransAudioBufferMeta>();
    meta->UnmarshalAudioMeta(std::string(paramItem->valuestring));
    buffer->pts = meta->pts_;
    buffer->GetBufferMeta()->SetMeta(Tag::USER_FRAME_PTS, meta->pts_);
    buffer->GetBufferMeta()->SetMeta(Tag::USER_FRAME_NUMBER, meta->frameNum_);
    AVTRANS_LOGI("buffer pts: %{public}ld, bufferLen: %{public}zu, frameNumber: %{public}zu",
        buffer->pts, buffer->GetMemory()->GetSize(), meta->frameNum_);
    return buffer;
}

void DsoftbusInputAudioPlugin::DataEnqueue(std::shared_ptr<Buffer> &buffer)
{
    std::lock_guard<std::mutex> lock(dataQueueMtx_);
    while (dataQueue_.size() >= DATA_QUEUE_MAX_SIZE) {
        AVTRANS_LOGE("Data queue overflow.");
        dataQueue_.pop();
    }
    dataQueue_.push(buffer);
    dataCond_.notify_all();
}

void DsoftbusInputAudioPlugin::HandleData()
{
    AVTRANS_LOGI("HandleData enter.");
    while (isrunning_) {
        if (GetCurrentState() != State::RUNNING) {
            continue;
        }
        std::shared_ptr<Buffer> buffer;
        {
            std::unique_lock<std::mutex> lock(dataQueueMtx_);
            dataCond_.wait_for(lock, std::chrono::milliseconds(PLUGIN_TASK_WAIT_TIME),
                [this]() { return !dataQueue_.empty(); });
            if (dataQueue_.empty()) {
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
    AVTRANS_LOGI("HandleData end.");
}

void DsoftbusInputAudioPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue)
{
    std::lock_guard<std::mutex> lock(dataQueueMtx_);
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DsoftbusInputAudioPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    (void) buffer;
    AVTRANS_LOGI("Push Data");
    return Status::OK;
}
}
}