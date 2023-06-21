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
    AVTRANS_LOGI("ctor.");
}

DscreenOutputPlugin::~DscreenOutputPlugin()
{
    AVTRANS_LOGI("dtor.");
}

Status DscreenOutputPlugin::Init()
{
    AVTRANS_LOGI("Init.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    state_ = State::INITIALIZED;
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
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::INITIALIZED) {
        AVTRANS_LOGE("The state is wrong.");
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
    AVTRANS_LOGI("Reset");
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
    if (tag == Plugin::Tag::USER_SHARED_MEMORY_FD) {
        sharedMemory_ = UnmarshalSharedMemory(Media::Plugin::AnyCast<std::string>(value));
    }
    if (tag == Plugin::Tag::USER_AV_SYNC_GROUP_INFO) {
        std::string groupInfo = Media::Plugin::AnyCast<std::string>(value);
        AVTRANS_LOGE("SetParameter USER_AV_SYNC_GROUP_INFO success. groupInfo=%s", groupInfo.c_str());
    }
    if (tag == Plugin::Tag::USER_TIME_SYNC_RESULT) {
        std::string timeSync = Media::Plugin::AnyCast<std::string>(value);
        AVTRANS_LOGE("SetParameter USER_TIME_SYNC_RESULT success. timeSync=%s", timeSync.c_str());
    }
    return Status::OK;
}

Status DscreenOutputPlugin::Start()
{
    AVTRANS_LOGI("Start");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::PREPARED) {
        AVTRANS_LOGE("The state is wrong.");
        return Status::ERROR_WRONG_STATE;
    }
    sendDisplayTask_->Start();
    DataQueueClear(outputBuffer_);
    state_ = State::RUNNING;
    return Status::OK;
}

Status DscreenOutputPlugin::Stop()
{
    AVTRANS_LOGI("Stop");
    Media::OSAL::ScopedLock lock(operationMutes_);
    if (state_ != State::RUNNING) {
        AVTRANS_LOGE("The state is wrong.");
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
        AVTRANS_LOGE("SetCallback failed, cb is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    eventsCb_ = cb;
    AVTRANS_LOGI("SetCallback success.");
    return Status::OK;
}

Status DscreenOutputPlugin::SetDataCallback(AVDataCallback callback)
{
    Media::OSAL::ScopedLock lock(operationMutes_);
    dataCb_ = callback;
    AVTRANS_LOGI("SetDataCallback success.");
    return Status::OK;
}

Status DscreenOutputPlugin::PushData(const std::string &inPort, std::shared_ptr<Buffer> buffer, int32_t offset)
{
    AVTRANS_LOGI("Queue Output AVBuffer.");
    Media::OSAL::ScopedLock lock(operationMutes_);
    uint32_t frameNumber = 1;
    if (buffer->GetBufferMeta()->IsExist(Tag::USER_FRAME_NUMBER)) {
        frameNumber = Plugin::AnyCast<uint32_t>(buffer->GetBufferMeta()->GetMeta(Tag::USER_FRAME_NUMBER));
    }
    AVTRANS_LOGI("buffer pts: %ld, bufferLen: %zu, frameNumber: %zu", buffer->pts, buffer->GetMemory()->GetSize(),
        frameNumber);

    if (buffer == nullptr || buffer->IsEmpty()) {
        AVTRANS_LOGE("AVBuffer is nullptr.");
        return Status::ERROR_NULL_POINTER;
    }
    while (outputBuffer_.size() >= DATA_QUEUE_MAX_SIZE) {
        AVTRANS_LOGE("outputBuffer_ queue overflow.");
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
                AVTRANS_LOGD("Data queue is empty.");
                continue;
            }
            buffer = outputBuffer_.front();
            outputBuffer_.pop();
        }
        if (buffer == nullptr) {
            AVTRANS_LOGE("Data is null");
            continue;
        }
        dataCb_(buffer);
        ReadMasterClockFromMemory(buffer);
    }
}

void DscreenOutputPlugin::ReadMasterClockFromMemory(const std::shared_ptr<Plugin::Buffer> &buffer)
{
    if ((buffer == nullptr) || (buffer->GetBufferMeta() == nullptr)) {
        AVTRANS_LOGE("output buffer or buffer meta is nullptr.");
        return;
    }

    if ((sharedMemory_.fd <= 0) || (sharedMemory_.size <= 0) || sharedMemory_.name.empty()) {
        AVTRANS_LOGE("invalid master clock shared memory info.");
        return;
    }

    auto bufferMeta = buffer->GetBufferMeta();
    if (!bufferMeta->IsExist(Tag::MEDIA_START_TIME) || !bufferMeta->IsExist(Tag::AUDIO_SAMPLE_PER_FRAME)) {
        AVTRANS_LOGE("the output buffer meta does not contains extPts and extFrameNum , ignore.");
        return;
    }

    int64_t audioPts = Plugin::AnyCast<int64_t>(bufferMeta->GetMeta(Tag::MEDIA_START_TIME));
    uint32_t audioFrmNum = Plugin::AnyCast<uint32_t>(bufferMeta->GetMeta(Tag::AUDIO_SAMPLE_PER_FRAME));
    AVTRANS_LOGI("get audioFrmNum=%" PRId32 ", audioPts=%lld from output buffer meta success.", audioFrmNum,
        (long long)audioPts);

    AVSyncClockUnit clockUnit = AVSyncClockUnit{ 0, audioFrmNum, 0 };
    int32_t ret = ReadClockUnitFromMemory(sharedMemory_, clockUnit);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("read master clock from shared memory failed.");
    }
}

void DscreenOutputPlugin::DataQueueClear(std::queue<std::shared_ptr<Buffer>> &queue)
{
    std::queue<std::shared_ptr<Buffer>> empty;
    swap(empty, queue);
}

Status DscreenOutputPlugin::StartOutputQueue()
{
    AVTRANS_LOGI("StartOutputQueue.");
    return Status::OK;
}

Status DscreenOutputPlugin::ControlFrameRate(const int64_t timestamp)
{
    AVTRANS_LOGI("ControlFrameRate.");
    return Status::OK;
}
}
}
