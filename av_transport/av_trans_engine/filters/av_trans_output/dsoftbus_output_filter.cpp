/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dsoftbus_output_filter.h"

#include <algorithm>

#include "av_trans_log.h"
#include "filter_factory.h"
#include "cJSON.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "av_trans_constants.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DSoftbusOutputFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
const std::string META_DATA_TYPE = "meta_data_type";
const std::string META_TIMESTAMP = "meta_timestamp";
const std::string META_TIMESTAMP_STRING = "meta_timestamp_string";
const std::string META_TIMESTAMP_SPECIAL = "meta_timestamp_special";
const std::string META_FRAME_NUMBER = "meta_frame_number";
const std::string META_EXT_TIMESTAMP = "meta_ext_timestamp";
const std::string META_EXT_FRAME_NUMBER = "meta_ext_frame_number";
const std::string INPUT_BUFFER_QUEUE_NAME = "buffer_queue_input";
const std::string SENDER_DATA_SESSION_NAME_SUFFIX = "sender.avtrans.data";
const std::string RECEIVER_DATA_SESSION_NAME_SUFFIX = "receiver.avtrans.data";
const std::string OUTPUT_BUFFER_QUEUE_NAME = "AVTransAudioOutputBufferQueue";

static AutoRegisterFilter<DSoftbusOutputFilter> g_registerAudioEncoderFilter("builtin.avtransport.avoutput",
    FilterType::FILTERTYPE_ASINK,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<DSoftbusOutputFilter>(name, FilterType::FILTERTYPE_ASINK);
    });

class DAudioSoftbusFilterLinkCallback : public FilterLinkCallback {
public:
    explicit DAudioSoftbusFilterLinkCallback(std::shared_ptr<DSoftbusOutputFilter> filter)
        : outFilter_(std::move(filter)) {}
    ~DAudioSoftbusFilterLinkCallback() = default;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer> &queue, std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = outFilter_.lock()) {
            filter->OnLinkedResult(queue, meta);
        } else {
            AVTRANS_LOGI("invalid DSoftbusOutputFilter");
        }
    }

    void OnUnlinkedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = outFilter_.lock()) {
            filter->OnUnlinkedResult(meta);
        } else {
            AVTRANS_LOGI("invalid DSoftbusOutputFilter");
        }
    }

    void OnUpdatedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = outFilter_.lock()) {
            filter->OnUpdatedResult(meta);
        } else {
            AVTRANS_LOGI("invalid daudioSoftbusOutputFilter");
        }
    }

private:
    std::weak_ptr<DSoftbusOutputFilter> outFilter_ {};
};

class AVBufferAvailableListener : public Media::IConsumerListener {
public:
    explicit AVBufferAvailableListener(std::weak_ptr<DSoftbusOutputFilter> dsoftbusFilter)
    {
        dsoftbusFilter_ = dsoftbusFilter;
    }

    void OnBufferAvailable() override
    {
        auto dsoftbusFilter = dsoftbusFilter_.lock();
        if (dsoftbusFilter != nullptr) {
            dsoftbusFilter->ProcessInputBuffer();
        }
    }

private:
    std::weak_ptr<DSoftbusOutputFilter> dsoftbusFilter_;
};

DSoftbusOutputFilter::DSoftbusOutputFilter(std::string name, FilterType type)
    : Filter(name, type)
{
}

DSoftbusOutputFilter::~DSoftbusOutputFilter()
{
    nextFiltersMap_.clear();
}

void DSoftbusOutputFilter::Init(const std::shared_ptr<EventReceiver>& receiver,
    const std::shared_ptr<FilterCallback>& callback)
{
    eventReceiver_ = receiver;
    filterCallback_ = callback;
}

Status DSoftbusOutputFilter::DoInitAfterLink()
{
    return Status::OK;
}

void DSoftbusOutputFilter::PrepareInputBuffer()
{
    AVTRANS_LOGI("Preparing input buffer.");
    int32_t outputBufNum = DEFAULT_BUFFER_NUM;
    Media::MemoryType memoryType = Media::MemoryType::VIRTUAL_MEMORY;
    if (outputBufQue_ == nullptr) {
        outputBufQue_ = Media::AVBufferQueue::Create(outputBufNum, memoryType, OUTPUT_BUFFER_QUEUE_NAME);
    }
    if (outputBufQue_ == nullptr) {
        AVTRANS_LOGE("Create buffer queue failed.");
        return;
    }
    inputBufQueProducer_ = outputBufQue_->GetProducer();
    TRUE_RETURN((inputBufQueProducer_ == nullptr), "Get producer failed");

    inputBufQueConsumer_ = outputBufQue_->GetConsumer();
    TRUE_RETURN((inputBufQueConsumer_ == nullptr), "Get consumer failed");

    sptr<Media::IConsumerListener> listener(new AVBufferAvailableListener(shared_from_this()));
    inputBufQueConsumer_->SetBufferAvailableListener(listener);
}

Status DSoftbusOutputFilter::DoPrepare()
{
    std::string str;
    if (meta_ != nullptr) {
        meta_->GetData(Media::Tag::MEDIA_DESCRIPTION, str);
    }
    cJSON *jParam = cJSON_Parse(str.c_str());
    if (jParam == nullptr) {
        AVTRANS_LOGE("Failed to parse json.");
        return Status::ERROR_NULL_POINTER;
    }
    cJSON *ownerName = cJSON_GetObjectItem(jParam, KEY_ONWER_NAME.c_str());
    if (ownerName == nullptr || !cJSON_IsString(ownerName)) {
        AVTRANS_LOGE("The key ownerName is null.");
        cJSON_Delete(jParam);
        return Status::ERROR_NULL_POINTER;
    }
    AVTRANS_LOGI("RegData type is : %{public}s.", ownerName->valuestring);
    ownerName_ = std::string(ownerName->valuestring);

    cJSON *peerDevId = cJSON_GetObjectItem(jParam, KEY_PEERDEVID_NAME.c_str());
    if (peerDevId == nullptr || !cJSON_IsString(peerDevId)) {
        AVTRANS_LOGE("The key peerDevId is null.");
        cJSON_Delete(jParam);
        return Status::ERROR_NULL_POINTER;
    }
    AVTRANS_LOGI("RegData type is : %{public}s.", peerDevId->valuestring);
    peerDevId_ = std::string(peerDevId->valuestring);
    sessionName_ = ownerName_ + "_" + SENDER_DATA_SESSION_NAME_SUFFIX;
    cJSON_Delete(jParam);
    PrepareInputBuffer();
    AVTRANS_LOGI("OnLinkedResult.");

    std::shared_ptr<Media::Meta> meta = std::make_shared<Media::Meta>();
    if (onLinkedResultCallback_ != nullptr) {
        onLinkedResultCallback_->OnLinkedResult(inputBufQueProducer_, meta);
    }
    return Status::OK;
}

Status DSoftbusOutputFilter::DoStart()
{
    std::string peerSessName = ownerName_ + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX;
    SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    int32_t ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(sessionName_, peerSessName, peerDevId_);
    if ((ret != DH_AVT_SUCCESS) && (ret != ERR_DH_AVT_SESSION_HAS_OPENED)) {
        AVTRANS_LOGE("Open softbus channel failed ret: %{public}d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    return Status::OK;
}

Status DSoftbusOutputFilter::DoPause()
{
    return Status::OK;
}

Status DSoftbusOutputFilter::DoPauseDragging()
{
    return Status::OK;
}

Status DSoftbusOutputFilter::DoResume()
{
    return Status::OK;
}

Status DSoftbusOutputFilter::DoResumeDragging()
{
    return Status::OK;
}

Status DSoftbusOutputFilter::DoStop()
{
    int32_t ret = SoftbusChannelAdapter::GetInstance().CloseSoftbusChannel(sessionName_, peerDevId_);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Close softbus channel failed ret: %{public}d.", ret);
        return Status::ERROR_NULL_POINTER;
    }
    return Status::OK;
}

Status DSoftbusOutputFilter::DoFlush()
{
    return Status::OK;
}

Status DSoftbusOutputFilter::DoRelease()
{
    return Status::OK;
}

Status DSoftbusOutputFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    (void)recvArg;
    (void)dropFrame;
    std::shared_ptr<Media::AVBuffer> filledBuffer = nullptr;
    Media::Status ret = inputBufQueConsumer_->AcquireBuffer(filledBuffer);
    if (ret != Media::Status::OK) {
        AVTRANS_LOGE("Acquire buffer err: %{public}d.", ret);
        return Status::ERROR_INVALID_OPERATION;
    }
    ProcessAndSendBuffer(filledBuffer);
    inputBufQueConsumer_->ReleaseBuffer(filledBuffer);
    return Status::OK;
}

Status DSoftbusOutputFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx,
    uint32_t idx, int64_t renderTimee)
{
    return Status::OK;
}

std::string DSoftbusOutputFilter::MarshalAudioMeta(BufferDataType dataType,
    int64_t pts, int64_t ptsSpecail, uint32_t frameNumber)
{
    cJSON *metaJson = cJSON_CreateObject();
    if (metaJson == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(metaJson, META_DATA_TYPE.c_str(), static_cast<uint32_t>(dataType));
    cJSON_AddNumberToObject(metaJson, META_TIMESTAMP.c_str(), pts);
    cJSON_AddNumberToObject(metaJson, META_FRAME_NUMBER.c_str(), frameNumber);
    cJSON_AddStringToObject(metaJson, META_TIMESTAMP_STRING.c_str(), std::to_string(pts).c_str());
    cJSON_AddStringToObject(metaJson, META_TIMESTAMP_SPECIAL.c_str(), std::to_string(ptsSpecail).c_str());
    char *data = cJSON_PrintUnformatted(metaJson);
    if (data == nullptr) {
        cJSON_Delete(metaJson);
        return "";
    }
    std::string jsonstr(data);
    cJSON_free(data);
    cJSON_Delete(metaJson);
    return jsonstr;
}

Status DSoftbusOutputFilter::ProcessAndSendBuffer(const std::shared_ptr<Media::AVBuffer> buffer)
{
    if (buffer == nullptr || buffer->memory_ == nullptr || buffer->meta_ == nullptr || meta_ == nullptr) {
        AVTRANS_LOGE("AVBuffer is nullptr");
        return Status::ERROR_NULL_POINTER;
    }
    auto bufferData = buffer->memory_;
    int64_t pts = 0;
    int64_t ptsSpecail = 0;
    uint32_t frameNumber = 0;
    pts = buffer->pts_;
    AVTRANS_LOGD("AVBuffer pts is %{public}" PRId64, pts);
    buffer->meta_->GetData(Media::Tag::USER_FRAME_PTS, ptsSpecail);
    buffer->meta_->GetData(Media::Tag::AUDIO_OBJECT_NUMBER, frameNumber);
    BufferDataType dataType;
    meta_->GetData(Media::Tag::MEDIA_STREAM_TYPE, dataType);
    auto dataParam = MarshalAudioMeta(dataType, pts, ptsSpecail, frameNumber);
    cJSON *jsonObj = cJSON_CreateObject();
    if (jsonObj == nullptr) {
        return Status::ERROR_NULL_POINTER;
    }
    cJSON_AddNumberToObject(jsonObj, AVT_DATA_META_TYPE.c_str(), static_cast<int32_t>(dataType));
    cJSON_AddStringToObject(jsonObj, AVT_DATA_PARAM.c_str(), dataParam.c_str());
    auto str = cJSON_PrintUnformatted(jsonObj);
    if (str == nullptr) {
        cJSON_Delete(jsonObj);
        return Status::ERROR_NULL_POINTER;
    }
    std::string jsonStr = std::string(str);
    cJSON_free(str);
    StreamData data = {reinterpret_cast<char *>(const_cast<uint8_t*>(bufferData->GetAddr())),
        bufferData->GetSize()};
    StreamData ext = {const_cast<char *>(jsonStr.c_str()), jsonStr.length()};
    int32_t ret = SoftbusChannelAdapter::GetInstance().SendStreamData(sessionName_, peerDevId_, &data, &ext);
    if (ret != DH_AVT_SUCCESS) {
        AVTRANS_LOGE("Send data to softbus failed.");
        cJSON_Delete(jsonObj);
        return Status::ERROR_INVALID_OPERATION;
    }
    cJSON_Delete(jsonObj);
    return Status::OK;
}

void DSoftbusOutputFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void DSoftbusOutputFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status DSoftbusOutputFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    nextFilter_ = nextFilter;
    nextFiltersMap_[outType].push_back(nextFilter_);
    auto filterLinkCallback = std::make_shared<DAudioSoftbusFilterLinkCallback>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, meta_, filterLinkCallback);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Onlinked failed, status: %{public}d.", ret);
        return ret;
    }
    return Status::OK;
}

Status DSoftbusOutputFilter::UpdateNext(const std::shared_ptr<Filter>&, StreamType outType)
{
    return Status::OK;
}

Status DSoftbusOutputFilter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType outType)
{
    AVTRANS_LOGI("cur: DSoftbusOutputFilter, unlink next filter..");
    return Status::OK;
}

Status DSoftbusOutputFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    AVTRANS_LOGI("DSoftbusOutputFilter, OnLinked");
    onLinkedResultCallback_ = callback;
    SetParameter(meta);
    return Status::OK;
};

Status DSoftbusOutputFilter::OnUpdated(StreamType inType, const std::shared_ptr<Media::Meta>& meta,
    const std::shared_ptr<FilterLinkCallback>& callback)
{
    AVTRANS_LOGI("DSoftbusOutputFilter, OnUpdated");
    return Status::OK;
}

Status DSoftbusOutputFilter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>& callback)
{
    return Status::OK;
}

void DSoftbusOutputFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue,
    std::shared_ptr<Media::Meta>& meta)
{
    outputBufQueProducer_ = queue;
}

void DSoftbusOutputFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
}

void DSoftbusOutputFilter::OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta)
{
}

void DSoftbusOutputFilter::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter, event type: %{public}d", event.type);
    TRUE_RETURN(eventReceiver_ == nullptr, "receiver_ is nullptr");
    Event channelEvent;
    channelEvent.type = EventType::EVENT_AUDIO_PROGRESS;
    channelEvent.param = event;
    eventReceiver_->OnEvent(channelEvent);
}

void DSoftbusOutputFilter::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
