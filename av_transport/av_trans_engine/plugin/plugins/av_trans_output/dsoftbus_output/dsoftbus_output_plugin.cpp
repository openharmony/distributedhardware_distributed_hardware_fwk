/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

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
    dumpFlag_.store(false);
    reDumpFlag_.store(false);
    SetCurrentState(State::INITIALIZED);
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
    if (GetCurrentState() != State::INITIALIZED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }

    sessionName_ = ownerName_ + "_" + SENDER_DATA_SESSION_NAME_SUFFIX;
    if (!bufferPopTask_) {
        bufferPopTask_ = std::make_shared<Media::OSAL::Task>("videoBufferQueuePopThread");
        bufferPopTask_->RegisterHandler([this] { FeedChannelData(); });
    }
    SetCurrentState(State::PREPARED);
    return Status::OK;
}

Status DsoftbusOutputPlugin::Reset()
{
    AVTRANS_LOGI("Reset");
    {
        std::lock_guard<std::mutex> lock(paramMapMutex_);
        paramsMap_.clear();
    }
    if (bufferPopTask_) {
        bufferPopTask_->Stop();
        bufferPopTask_.reset();
    }
    DataQueueClear(dataQueue_);
    eventsCb_ = nullptr;
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, peerDevId_);
    SetCurrentState(State::INITIALIZED);
    return Status::OK;
}

Status DsoftbusOutputPlugin::Start()
{
    AVTRANS_LOGI("Dsoftbus Output Plugin start.");
    if (GetCurrentState() != State::PREPARED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    Status ret = OpenSoftbusChannel();
    if (ret != Status::OK) {
        AVTRANS_LOGE("OpenSoftbusSession failed.");
        return Status::ERROR_INVALID_OPERATION;
    }
    DataQueueClear(dataQueue_);
    if (bufferPopTask_) {
        bufferPopTask_->Start();
    }
    SetCurrentState(State::RUNNING);
    return Status::OK;
}

Status DsoftbusOutputPlugin::Stop()
{
    AVTRANS_LOGI("Dsoftbus Output Plugin stop.");
    if (GetCurrentState() != State::RUNNING) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    SetCurrentState(State::PREPARED);
    if (bufferPopTask_) {
        bufferPopTask_->Stop();
    }
    DataQueueClear(dataQueue_);
    CloseSoftbusChannel();
    return Status::OK;
}

Status DsoftbusOutputPlugin::GetParameter(Tag tag, ValueType &value)
{
    std::lock_guard<std::mutex> lock(paramMapMutex_);
    auto res = paramsMap_.find(tag);
    if (res != paramsMap_.end()) {
        value = res->second;
        return Status::OK;
    }
    return Status::ERROR_NOT_EXISTED;
}

Status DsoftbusOutputPlugin::SetParameter(Tag tag, const ValueType &value)
{
    std::lock_guard<std::mutex> lock(paramMapMutex_);
    if (tag == Tag::MEDIA_DESCRIPTION) {
        ParseChannelDescription(Plugin::AnyCast<std::string>(value), ownerName_, peerDevId_);
    }
    if (tag == Tag::SECTION_USER_SPECIFIC_START) {
        dumpFlag_.store(Plugin::AnyCast<bool>(value));
    }
    if (tag == Tag::SECTION_VIDEO_SPECIFIC_START) {
        reDumpFlag_.store(Plugin::AnyCast<bool>(value));
    }
    paramsMap_.insert(std::pair<Tag, ValueType>(tag, value));
    return Status::OK;
}

Status DsoftbusOutputPlugin::SetCallback(Callback *cb)
{
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
        AVTRANS_LOGE("Register channel listener failed ret: %{public}d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    std::string peerSessName = ownerName_ + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX;
    ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(sessionName_, peerSessName, peerDevId_);
    if ((ret != DH_AVT_SUCCESS) && (ret != ERR_DH_AVT_SESSION_HAS_OPENED)) {
        AVTRANS_LOGE("Open softbus channel failed ret: %{public}d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    return Status::OK;
}

void DsoftbusOutputPlugin::CloseSoftbusChannel()
{
    int32_t ret = SoftbusChannelAdapter::GetInstance().CloseSoftbusChannel(sessionName_, peerDevId_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Close softbus channle failed ret: %{public}d.", ret);
    }
}

void DsoftbusOutputPlugin::OnChannelEvent(const AVTransEvent &event)
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

void DsoftbusOutputPlugin::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

Status DsoftbusOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    std::lock_guard<std::mutex> lock(dataQueueMtx_);
    if (buffer == nullptr || buffer->IsEmpty()) {
        AVTRANS_LOGE("Buffer is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    if (GetReDumpFlag() == true) {
        std::remove(SCREEN_FILE_NAME_AFTERCODING.c_str());
        SetReDumpFlagFalse();
    }
    if (GetDumpFlag() == true) {
        auto bufferData = buffer->GetMemory();
        DumpBufferToFile(SCREEN_FILE_NAME_AFTERCODING,
            const_cast<uint8_t*>(bufferData->GetReadOnlyData()), bufferData->GetSize());
    } else {
        AVTRANS_LOGD("DumpFlag = false.");
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
    while (GetCurrentState() == State::RUNNING) {
        std::shared_ptr<Buffer> buffer;
        {
            std::unique_lock<std::mutex> lock(dataQueueMtx_);
            dataCond_.wait_for(lock, std::chrono::milliseconds(PLUGIN_TASK_WAIT_TIME),
                [this]() { return !dataQueue_.empty(); });
            if (GetCurrentState() != State::RUNNING) {
                return;
            }
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
        SendDataToSoftbus(buffer);
    }
}

void DsoftbusOutputPlugin::SendDataToSoftbus(std::shared_ptr<Buffer> &buffer)
{
    if (buffer == nullptr || buffer->GetBufferMeta() == nullptr || buffer->GetMemory() == nullptr) {
        AVTRANS_LOGE("buffer or getbuffermeta or getmemory is nullptr.");
        return;
    }
    cJSON *jsonObj = cJSON_CreateObject();
    if (jsonObj == nullptr) {
        return;
    }
    auto bufferMeta = buffer->GetBufferMeta();
    BufferMetaType metaType = bufferMeta->GetType();
    cJSON_AddNumberToObject(jsonObj, AVT_DATA_META_TYPE.c_str(), static_cast<uint32_t>(metaType));
    if (metaType != BufferMetaType::VIDEO) {
        AVTRANS_LOGE("metaType is wrong");
        cJSON_Delete(jsonObj);
        return;
    }
    auto hisAMeta = std::make_shared<AVTransVideoBufferMeta>();
    hisAMeta->frameNum_ = Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_NUMBER));
    hisAMeta->pts_ = buffer->pts;
    AVTRANS_LOGI("buffer pts: %{public}ld, bufferLen: %{public}zu, frameNumber: %{public}u",
        hisAMeta->pts_, buffer->GetMemory()->GetSize(), hisAMeta->frameNum_);
    if (bufferMeta->IsExist(Tag::MEDIA_START_TIME)) {
        hisAMeta->extPts_ = Plugin::AnyCast<int64_t>(bufferMeta->GetMeta(Tag::MEDIA_START_TIME));
    }
    if (bufferMeta->IsExist(Tag::AUDIO_SAMPLE_PER_FRAME)) {
        hisAMeta->extFrameNum_ = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::AUDIO_SAMPLE_PER_FRAME));
    }
    cJSON_AddStringToObject(jsonObj, AVT_DATA_PARAM.c_str(), hisAMeta->MarshalVideoMeta().c_str());

    char *str = cJSON_PrintUnformatted(jsonObj);
    if (str == nullptr) {
        cJSON_Delete(jsonObj);
        return;
    }
    std::string jsonStr = std::string(str);
    AVTRANS_LOGI("jsonStr->bufLen %{public}zu, jsonStR: %{public}s", jsonStr.length(), jsonStr.c_str());

    auto bufferData = buffer->GetMemory();
    StreamData data = {reinterpret_cast<char *>(const_cast<uint8_t*>(bufferData->GetReadOnlyData())),
        bufferData->GetSize()};
    StreamData ext = {const_cast<char *>(jsonStr.c_str()), jsonStr.length()};

    int32_t ret = SoftbusChannelAdapter::GetInstance().SendStreamData(sessionName_, peerDevId_, &data, &ext);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Send data to softbus failed.");
    }
    cJSON_free(str);
    cJSON_Delete(jsonObj);
}

void DsoftbusOutputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue)
{
    std::lock_guard<std::mutex> lock(dataQueueMtx_);
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DsoftbusOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    AVTRANS_LOGI("SetDataCallback");
    return Status::OK;
}

bool DsoftbusOutputPlugin::GetDumpFlag()
{
    return dumpFlag_;
}

void DsoftbusOutputPlugin::SetDumpFlagFalse()
{
    dumpFlag_ = false;
}

bool DsoftbusOutputPlugin::GetReDumpFlag()
{
    return reDumpFlag_;
}

void DsoftbusOutputPlugin::SetReDumpFlagFalse()
{
    reDumpFlag_ = false;
}
}
}