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

#include "foundation/utils/constants.h"
#include "plugin/common/plugin_caps_builder.h"
#include "plugin/factory/plugin_factory.h"
#include "plugin/interface/generic_plugin.h"

namespace OHOS {
namespace DistributedHardware {

std::vector<GenericPluginDef> CreateDsoftbusOutputPluginDef()
{
    int32_t capNum = 2;
    std::vector<GenericPluginDef> definitionList;
    for (int i = 0; i < capNum; i++) {
        AVTRANS_LOGI("DsoftbusOutputPlugin_H264 registered.");
        GenericPluginDef definition;
        definition.name = "AVTransDsoftbusOutputPlugin_H264";
        definition.pkgName = "AVTransDsoftbusOutputPlugin";
        definition.description = "Video transport to dsoftbus";
        definition.rank = PLUGIN_RANK;
        definition.creator = [] (const std::string& name) -> std::shared_ptr<AvTransOutputPlugin> {
            return std::make_shared<DsoftbusOutputPlugin>(name);
        };

        definition.pkgVersion = AVTRANS_OUTPUT_API_VERSION;
        definition.license = LicenseType::APACHE_V2;

        CapabilityBuilder capBuilder;
        capBuilder.SetMime(Media::MEDIA_MIME_VIDEO_H264);
        if (i == 1) {
            AVTRANS_LOGI("DsoftbusOutputPlugin_H265 registered.");
            definition.name = "AVTransDsoftbusOutputPlugin_H265";
            capBuilder.SetMime(Media::MEDIA_MIME_VIDEO_H265);
        }
        definition.inCaps.push_back(capBuilder.Build());
        definitionList.push_back(definition);
    }
    return definitionList;
}

static AutoRegisterPlugin<DsoftbusOutputPlugin> g_registerPluginHelper(CreateDsoftbusOutputPluginDef());

DsoftbusOutputPlugin::DsoftbusOutputPlugin(std::string name)
    : AvTransOutputPlugin(std::move(name))
{
    AVTRANS_LOGI("ctor.");
}

DsoftbusOutputPlugin::~DsoftbusOutputPlugin()
{
    AVTRANS_LOGI("dtor.");
}

Status DsoftbusOutputPlugin::Init()
{
    AVTRANS_LOGI("Init Dsoftbus Output Plugin.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusOutputPlugin::Deinit()
{
    AVTRANS_LOGI("Deinit Dsoftbus Output Plugin.");
    return Reset();
}

Status DsoftbusOutputPlugin::Prepare()
{
    AVTRANS_LOGI("Prepare");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }

    sessionName_ = ownerName_ + "_" + SENDER_DATA_SESSION_NAME_SUFFIX;
    int32_t ret = SoftbusChannelAdapter::GetInstance().CreateChannelServer(TransName2PkgName(ownerName_), sessionName_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Create Session Server failed ret: %d.", ret);
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
    AVTRANS_LOGI("Reset");
    Media::OSAL::ScopedLock lock(operationMutes_);
    paramsMap_.clear();
    if (bufferPopTask_) {
        bufferPopTask_->Stop();
        bufferPopTask_.reset();
    }
    DataQueueClear(dataQueue_);
    eventsCb_ = nullptr;
    SoftbusChannelAdapter::GetInstance().RemoveChannelServer(TransName2PkgName(ownerName_), sessionName_);
    state_ = State::INITIALIZED;
    return Status::OK;
}

Status DsoftbusOutputPlugin::Start()
{
    AVTRANS_LOGI("Dsoftbus Output Plugin start.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    Status ret = OpenSoftbusChannel();
    if (ret != Status::OK) {
        AVTRANS_LOGE("OpenSoftbusSession failed.");
        return Status::ERROR_INVALID_OPERATION;
    }
    DataQueueClear(dataQueue_);
    bufferPopTask_->Start();
    state_ = State::RUNNING;
    return Status::OK;
}

Status DsoftbusOutputPlugin::Stop()
{
    AVTRANS_LOGI("Dsoftbus Output Plugin stop.");
    if (state_ != State::RUNNING) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }

    bufferPopTask_->Pause();
    DataQueueClear(dataQueue_);
    CloseSoftbusChannel();
    state_ = State::PREPARED;
    return Status::OK;
}

Status DsoftbusOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    auto res = paramsMap_.find(tag);
    if (res == paramsMap_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DsoftbusOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (tag == Tag::MEDIA_DESCRIPTION) {
        ParseChannelDescription(Plugin::AnyCast<std::string>(value), ownerName_, peerDevId_);
    }
    paramsMap_.insert(std::pair<Tag, ValueType>(tag, value));
    return Status::OK;
}

Status DsoftbusOutputPlugin::SetCallback(Callback *cb)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (cb == nullptr) {
        AVTRANS_LOGE("SetCallback failed, cb is nullptr.");
        return Status::ERROR_INVALID_OPERATION;
    }
    eventsCb_ = cb;
    AVTRANS_LOGI("SetCallback success");
    return Status::OK;
}

Status DsoftbusOutputPlugin::OpenSoftbusChannel()
{
    int32_t ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Register channel listener failed ret: %d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    std::string peerSessName_ = ownerName_ + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX;
    ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(sessionName_, peerSessName_, peerDevId_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Open softbus channel failed ret: %d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    return Status::OK;
}

void DsoftbusOutputPlugin::CloseSoftbusChannel()
{
    int32_t ret = SoftbusChannelAdapter::GetInstance().CloseSoftbusChannel(sessionName_, peerDevId_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Close softbus channle failed ret: %s.", ret);
    }
}

void DsoftbusOutputPlugin::OnChannelEvent(const AVTransEvent &event)
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

void DsoftbusOutputPlugin::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

Status DsoftbusOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    AVTRANS_LOGI("Push Buffer to output plugin.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (buffer == nullptr || buffer->IsEmpty()) {
        AVTRANS_LOGE("Buffer is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    while (dataQueue_.size() >= DATA_QUEUE_MAX_SIZE) {
        AVTRANS_LOGE("Data queue overflow.");
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
        SendDataToSoftbus(buffer);
    }
}

void DsoftbusOutputPlugin::SendDataToSoftbus(std::shared_ptr<Buffer> &buffer)
{
    json jsonObj;
    auto bufferMeta = buffer->GetBufferMeta();
    BufferMetaType metaType = bufferMeta->GetType();
    jsonObj[AVT_DATA_META_TYPE] = metaType;
    if (metaType != BufferMetaType::VIDEO) {
        AVTRANS_LOGE("metaType is wrong");
        return;
    }
    auto hisAMeta = std::make_shared<AVTransVideoBufferMeta>();
    hisAMeta->frameNum_ = Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_NUMBER));
    hisAMeta->pts_ = buffer->pts;
    AVTRANS_LOGI("buffer pts: %ld, bufferLen: %zu, frameNumber: %zu", hisAMeta->pts_, buffer->GetMemory()->GetSize(),
        hisAMeta->frameNum_);
    if (bufferMeta->IsExist(Tag::MEDIA_START_TIME)) {
        hisAMeta->extPts_ = Plugin::AnyCast<int64_t>(bufferMeta->GetMeta(Tag::MEDIA_START_TIME));
    }
    if (bufferMeta->IsExist(Tag::AUDIO_SAMPLE_PER_FRAME)) {
        hisAMeta->extFrameNum_ = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::AUDIO_SAMPLE_PER_FRAME));
    }
    jsonObj[AVT_DATA_PARAM] = hisAMeta->MarshalVideoMeta();

    std::string jsonStr = jsonObj.dump();
    AVTRANS_LOGI("jsonStr->bufLen %zu, jsonStR: %s", jsonStr.length(), jsonStr.c_str());

    auto bufferData = buffer->GetMemory();
    StreamData data = {reinterpret_cast<char *>(const_cast<uint8_t*>(bufferData->GetReadOnlyData())),
        bufferData->GetSize()};
    StreamData ext = {const_cast<char *>(jsonStr.c_str()), jsonStr.length()};

    int32_t ret = SoftbusChannelAdapter::GetInstance().SendStreamData(sessionName_, peerDevId_, &data, &ext);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Send data to softbus failed.");
    }
}

void DsoftbusOutputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue)
{
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DsoftbusOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    AVTRANS_LOGI("SetDataCallback");
    return Status::OK;
}
}
}