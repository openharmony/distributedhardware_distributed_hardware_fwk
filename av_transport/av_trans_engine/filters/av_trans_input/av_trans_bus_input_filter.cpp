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

#include "av_trans_bus_input_filter.h"

#include <algorithm>
#include <memory>

#include "av_trans_constants.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"
#include "av_trans_types.h"
#include "filter_factory.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "AVTransBusInputFilter"

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
namespace {
constexpr int32_t DEFAULT_BUFFER_NUM = 8;
constexpr int32_t MAX_TIME_OUT_MS = 1;
const std::string INPUT_BUFFER_QUEUE_NAME = "AVTransBusInputBufferQueue";
const std::string META_TIMESTAMP = "meta_timestamp";
const std::string META_TIMESTAMP_STRING = "meta_timestamp_string";
const std::string META_TIMESTAMP_SPECIAL = "meta_timestamp_special";

bool IsUInt32(const cJSON *jsonObj, const std::string &key)
{
    cJSON *keyObj = cJSON_GetObjectItemCaseSensitive(jsonObj, key.c_str());
    return (keyObj != nullptr) && cJSON_IsNumber(keyObj) &&
        static_cast<uint32_t>(keyObj->valueint) <= UINT32_MAX;
}
}

static AutoRegisterFilter<AVTransBusInputFilter> g_registerAudioFilter("builtin.avtrans.softbus.input",
    FilterType::AUDIO_DATA_SOURCE,
    [](const std::string& name, const FilterType type) {
        return std::make_shared<AVTransBusInputFilter>(name, FilterType::AUDIO_DATA_SOURCE);
    });

class BusInputFilterLinkCB : public FilterLinkCallback {
public:
    explicit BusInputFilterLinkCB(std::shared_ptr<AVTransBusInputFilter> filter)
        : filter_(std::move(filter)) {}
    ~BusInputFilterLinkCB() = default;

    void OnLinkedResult(const sptr<Media::AVBufferQueueProducer> &queue, std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = filter_.lock()) {
            filter->OnLinkedResult(queue, meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

    void OnUnlinkedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = filter_.lock()) {
            filter->OnUnlinkedResult(meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

    void OnUpdatedResult(std::shared_ptr<Media::Meta> &meta) override
    {
        if (auto filter = filter_.lock()) {
            filter->OnUpdatedResult(meta);
        } else {
            AVTRANS_LOGI("invalid headFilter");
        }
    }

private:
    std::weak_ptr<AVTransBusInputFilter> filter_ {};
};

class InputBufAvailableListener : public Media::IConsumerListener {
public:
    explicit InputBufAvailableListener(const std::weak_ptr<AVTransBusInputFilter> inputFilter)
    {
        inputFilter_ = inputFilter;
    }

    void OnBufferAvailable() override
    {
        auto inputFilter = inputFilter_.lock();
        if (inputFilter != nullptr) {
            inputFilter->ProcessInputBuffer();
        }
    }

private:
    std::weak_ptr<AVTransBusInputFilter> inputFilter_;
};

AVTransBusInputFilter::AVTransBusInputFilter(std::string name, FilterType type, bool isAsyncMode)
    : Filter(name, type)
{
}

AVTransBusInputFilter::~AVTransBusInputFilter()
{
    nextFiltersMap_.clear();
}

void AVTransBusInputFilter::Init(const std::shared_ptr<EventReceiver>& receiver,
    const std::shared_ptr<FilterCallback>& callback)
{
    AVTRANS_LOGI("AVTransBusInputFilter::Init");
    receiver_ = receiver;
    callback_ = callback;
    AVTRANS_LOGI("AVTransBusInputFilter::Init Done");
}

Status AVTransBusInputFilter::DoInitAfterLink()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoInitAfterLink");
    return Status::OK;
}

void AVTransBusInputFilter::PrepareInputBuffer()
{
    AVTRANS_LOGI("Preparing input buffer.");
    int32_t inputBufNum = DEFAULT_BUFFER_NUM;
    Media::MemoryType memoryType = Media::MemoryType::VIRTUAL_MEMORY;
    if (inputBufQue_ == nullptr) {
        inputBufQue_ = Media::AVBufferQueue::Create(inputBufNum, memoryType, INPUT_BUFFER_QUEUE_NAME);
    }
    if (inputBufQue_ == nullptr) {
        AVTRANS_LOGE("Create buffer queue failed.");
        return;
    }
    inputBufQueProducer_ = inputBufQue_->GetProducer();
    TRUE_RETURN((inputBufQueProducer_ == nullptr), "Get producer failed");

    inputBufQueConsumer_ = inputBufQue_->GetConsumer();
    TRUE_RETURN((inputBufQueConsumer_ == nullptr), "Get consumer failed");

    sptr<Media::IConsumerListener> listener(new InputBufAvailableListener(shared_from_this()));
    inputBufQueConsumer_->SetBufferAvailableListener(listener);
}

Status AVTransBusInputFilter::DoPrepare()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoPrepare");
    std::string str;
    if (meta_ != nullptr) {
        meta_->GetData(Media::Tag::MEDIA_DESCRIPTION, str);
    }
    cJSON *jParam = cJSON_Parse(str.c_str());
    TRUE_RETURN_V_MSG_E(jParam == nullptr, Status::ERROR_NULL_POINTER, "Failed to parse json.");
    cJSON *ownerName = cJSON_GetObjectItem(jParam, KEY_ONWER_NAME.c_str());
    if (ownerName == nullptr || !cJSON_IsString(ownerName)) {
        AVTRANS_LOGE("The key ownerName is null.");
        cJSON_Delete(jParam);
        return Status::ERROR_NULL_POINTER;
    }
    ownerName_ = std::string(ownerName->valuestring);
    cJSON *peerDevId = cJSON_GetObjectItem(jParam, KEY_PEERDEVID_NAME.c_str());
    if (peerDevId == nullptr || !cJSON_IsString(peerDevId)) {
        AVTRANS_LOGE("The key peerDevId is null.");
        cJSON_Delete(jParam);
        return Status::ERROR_NULL_POINTER;
    }
    peerDevId_ = std::string(peerDevId->valuestring);
    cJSON_Delete(jParam);
    sessionName_ = ownerName_ + "_" + RECEIVER_DATA_SESSION_NAME_SUFFIX;
    SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, peerDevId_, this);
    int32_t ret = SoftbusChannelAdapter::GetInstance().CreateChannelServer(TransName2PkgName(ownerName_), sessionName_);
    TRUE_RETURN_V_MSG_E(ret != DH_AVT_SUCCESS, Status::ERROR_INVALID_OPERATION,
        "Create Session Server failed ret: %{public}d.", ret);
    TRUE_RETURN_V_MSG_E(callback_ == nullptr || meta_ == nullptr, Status::ERROR_NULL_POINTER, "callback is null");
    int32_t mimeType = 0;
    auto filterType = StreamType::STREAMTYPE_DECODED_AUDIO;
    meta_->GetData(Media::Tag::MIME_TYPE, mimeType);
    if (static_cast<AudioCodecType>(mimeType) == AudioCodecType::AUDIO_CODEC_AAC) {
        filterType = StreamType::STREAMTYPE_RAW_AUDIO;
    }
    callback_->OnCallback(shared_from_this(), FilterCallBackCommand::NEXT_FILTER_NEEDED, filterType);
    return Status::OK;
}

Status AVTransBusInputFilter::DoStart()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoStart");
    return Status::OK;
}

Status AVTransBusInputFilter::DoPause()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoPause");
    return Status::OK;
}

Status AVTransBusInputFilter::DoPauseDragging()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoPauseDragging");
    return Status::OK;
}

Status AVTransBusInputFilter::DoResume()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoResume");
    return Status::OK;
}

Status AVTransBusInputFilter::DoResumeDragging()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoResumeDragging");
    return Status::OK;
}

Status AVTransBusInputFilter::DoStop()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoStop");
    return Status::OK;
}

Status AVTransBusInputFilter::DoFlush()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoFlush");
    return Status::OK;
}

Status AVTransBusInputFilter::DoRelease()
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoRelease");
    SoftbusChannelAdapter::GetInstance().RemoveChannelServer(TransName2PkgName(ownerName_), sessionName_);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, peerDevId_);
    return Status::OK;
}

Status AVTransBusInputFilter::DoProcessInputBuffer(int recvArg, bool dropFrame)
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoProcessInputBuffer");
    (void)recvArg;
    (void)dropFrame;
    std::shared_ptr<Media::AVBuffer> filledBuffer = nullptr;
    if (curState_ != FilterState::RUNNING) {
        AVTRANS_LOGE("Current status ia not running.");
        return Status::ERROR_WRONG_STATE;
    }
    Media::Status ret = inputBufQueConsumer_->AcquireBuffer(filledBuffer);
    if (ret != Media::Status::OK) {
        AVTRANS_LOGE("Acquire buffer err.");
        return Status::ERROR_INVALID_OPERATION;
    }
    ProcessAndSendBuffer(filledBuffer);
    inputBufQueConsumer_->ReleaseBuffer(filledBuffer);
    return Status::OK;
}

Status AVTransBusInputFilter::ProcessAndSendBuffer(const std::shared_ptr<Media::AVBuffer> buffer)
{
    if (buffer == nullptr || buffer->memory_ == nullptr) {
        AVTRANS_LOGE("AVBuffer is null");
        return Status::ERROR_NULL_POINTER;
    }

    TRUE_RETURN_V_MSG_E((outputBufQueProducer_ == nullptr), Status::ERROR_NULL_POINTER, "Producer is null");
    Media::AVBufferConfig config(buffer->GetConfig());
    AVTRANS_LOGD("outPut config, size: %{public}u, capacity: %{public}u, memtype: %{public}hhu",
        config.size, config.capacity, config.memoryType);
    std::shared_ptr<Media::AVBuffer> outBuffer = nullptr;
    outputBufQueProducer_->RequestBuffer(outBuffer, config, MAX_TIME_OUT_MS);
    TRUE_RETURN_V_MSG_E((outBuffer == nullptr || outBuffer->memory_ == nullptr), Status::ERROR_NULL_POINTER,
        "OutBuffer or memory is null");
    auto meta = outBuffer->meta_;
    if (meta == nullptr) {
        AVTRANS_LOGE("Meta of AVBuffer is null");
        outputBufQueProducer_->PushBuffer(outBuffer, true);
        return Status::ERROR_NULL_POINTER;
    }
    meta->SetData(Media::Tag::USER_FRAME_PTS, outBuffer->pts_);
    outBuffer->memory_->Write(buffer->memory_->GetAddr(), buffer->memory_->GetSize(), 0);
    outputBufQueProducer_->PushBuffer(outBuffer, true);
    return Status::OK;
}

Status AVTransBusInputFilter::DoProcessOutputBuffer(int recvArg, bool dropFrame, bool byIdx,
    uint32_t idx, int64_t renderTimee)
{
    AVTRANS_LOGI("AVTransBusInputFilter::DoProcessOutputBuffer");
    return Status::OK;
}

void AVTransBusInputFilter::SetParameter(const std::shared_ptr<Media::Meta>& meta)
{
    meta_ = meta;
}

void AVTransBusInputFilter::GetParameter(std::shared_ptr<Media::Meta>& meta)
{
    meta = meta_;
}

Status AVTransBusInputFilter::LinkNext(const std::shared_ptr<Filter>& nextFilter, StreamType outType)
{
    AVTRANS_LOGI("cur: AVTransBusInputFilter, link next filter..");
    nextFilter_ = nextFilter;
    nextFiltersMap_[outType].push_back(nextFilter_);
    auto filterLinkCallback = std::make_shared<BusInputFilterLinkCB>(shared_from_this());
    auto ret = nextFilter->OnLinked(outType, meta_, filterLinkCallback);
    if (ret != Status::OK) {
        AVTRANS_LOGE("Onlinked failed, status: %{public}d.", ret);
        return ret;
    }
    return Status::OK;
}

Status AVTransBusInputFilter::UpdateNext(const std::shared_ptr<Filter>&, StreamType)
{
    return Status::OK;
}

Status AVTransBusInputFilter::UnLinkNext(const std::shared_ptr<Filter>&, StreamType)
{
    AVTRANS_LOGI("cur: AVTransBusInputFilter, unlink next filter..");
    return Status::OK;
}

Status AVTransBusInputFilter::OnLinked(StreamType inType, const std::shared_ptr<Media::Meta> &meta,
    const std::shared_ptr<FilterLinkCallback> &callback)
{
    AVTRANS_LOGI("cur: AVTransBusInputFilter, OnLinked");
    return Status::OK;
};

Status AVTransBusInputFilter::OnUpdated(StreamType, const std::shared_ptr<Media::Meta>&,
    const std::shared_ptr<FilterLinkCallback>&)
{
    return Status::OK;
}

Status AVTransBusInputFilter::OnUnLinked(StreamType, const std::shared_ptr<FilterLinkCallback>&)
{
    AVTRANS_LOGI("cur: AVTransBusInputFilter, OnUnLinked.");
    return Status::OK;
}

void AVTransBusInputFilter::OnLinkedResult(const sptr<Media::AVBufferQueueProducer>& queue,
    std::shared_ptr<Media::Meta>& meta)
{
    AVTRANS_LOGI("cur: AVTransBusInputFilter, OnLinkedResult");
    outputBufQueProducer_ = queue;
}

void AVTransBusInputFilter::OnUnlinkedResult(std::shared_ptr<Media::Meta>& meta)
{
    (void)meta;
}

void AVTransBusInputFilter::OnUpdatedResult(std::shared_ptr<Media::Meta>& meta)
{
    (void)meta;
}

void AVTransBusInputFilter::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter, event type: %{public}d", event.type);
    switch (event.type) {
        case OHOS::DistributedHardware::EventType::EVENT_CHANNEL_OPENED: {
            AVTRANS_LOGD("channel opened.");
            TRUE_RETURN(receiver_ == nullptr, "receiver_ is nullptr");
            Event channelEvent;
            channelEvent.type = EventType::EVENT_AUDIO_PROGRESS;
            channelEvent.param = event;
            receiver_->OnEvent(channelEvent);
            break;
        }
        case OHOS::DistributedHardware::EventType::EVENT_CHANNEL_OPEN_FAIL: {
            AVTRANS_LOGE("channel open failed.");
            break;
        }
        case OHOS::DistributedHardware::EventType::EVENT_CHANNEL_CLOSED: {
            AVTRANS_LOGI("channel closed.");
            break;
        }
        default:
            AVTRANS_LOGE("Unsupported event type.");
    }
}

void AVTransBusInputFilter::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    if (ext == nullptr) {
        AVTRANS_LOGE("ext is nullptr.");
        return;
    }
    std::string message(reinterpret_cast<const char *>(ext->buf), ext->bufLen);
    TRUE_RETURN(message.length() > MAX_MESSAGES_LEN, "Message length is iilegal.");
    AVTRANS_LOGD("Receive message : %{public}s", message.c_str());

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
    StreamDataEnqueue(data, resMsg);
    cJSON_Delete(resMsg);
}

bool AVTransBusInputFilter::UnmarshalAudioMeta(const std::string& jsonStr, int64_t& pts, int64_t& ptsSpecial)
{
    cJSON *metaJson = cJSON_Parse(jsonStr.c_str());
    if (metaJson == nullptr) {
        return false;
    }
    cJSON *ptsObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_TIMESTAMP.c_str());
    if (ptsObj == nullptr || !cJSON_IsNumber(ptsObj)) {
        cJSON_Delete(metaJson);
        return false;
    }
    pts = static_cast<int64_t>(ptsObj->valueint);
    cJSON *ptsStringObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_TIMESTAMP_STRING.c_str());
    if (ptsStringObj == nullptr || !cJSON_IsString(ptsStringObj)) {
        cJSON_Delete(metaJson);
        return false;
    }
    auto ptsStr = std::string(ptsStringObj->valuestring);
    if (ptsStr.empty()) {
        cJSON_Delete(metaJson);
        return false;
    }
    pts = std::atoll(ptsStr.c_str());
    cJSON *ptsSpecialObj = cJSON_GetObjectItemCaseSensitive(metaJson, META_TIMESTAMP_SPECIAL.c_str());
    if (ptsSpecialObj == nullptr || !cJSON_IsString(ptsSpecialObj)) {
        cJSON_Delete(metaJson);
        return false;
    }
    auto ptsSpecialStr = std::string(ptsSpecialObj->valuestring);
    if (ptsSpecialStr.empty()) {
        cJSON_Delete(metaJson);
        return false;
    }
    ptsSpecial = std::atoll(ptsSpecialStr.c_str());
    AVTRANS_LOGD("pts: %{public}" PRId64", ptsSpecial: %{public}" PRId64, pts, ptsSpecial);
    cJSON_Delete(metaJson);
    return true;
}

void AVTransBusInputFilter::StreamDataEnqueue(const StreamData *data, const cJSON *extMsg)
{
    TRUE_RETURN((outputBufQueProducer_ == nullptr || data == nullptr || extMsg == nullptr), "Producer is null");
    Media::AVBufferConfig config;
    config.size = data->bufLen;
    config.memoryType = Media::MemoryType::VIRTUAL_MEMORY;
    config.memoryFlag = Media::MemoryFlag::MEMORY_READ_WRITE;
    AVTRANS_LOGD("outPut config, size: %{public}u, capacity: %{public}u, memtype: %{public}hhu",
        config.size, config.capacity, config.memoryType);
    std::shared_ptr<Media::AVBuffer> outBuffer = nullptr;
    outputBufQueProducer_->RequestBuffer(outBuffer, config, MAX_TIME_OUT_MS);
    TRUE_RETURN((outBuffer == nullptr || outBuffer->memory_ == nullptr),
        "OutBuffer or memory is null");
    auto meta = outBuffer->meta_;
    if (meta == nullptr) {
        AVTRANS_LOGE("Meta of AVBuffer is null");
        outputBufQueProducer_->PushBuffer(outBuffer, false);
        return;
    }
    cJSON *paramItem = cJSON_GetObjectItem(extMsg, AVT_DATA_PARAM.c_str());
    if (paramItem == nullptr || !cJSON_IsString(paramItem)) {
        AVTRANS_LOGE("paramItem is invalid.");
        return;
    }
    int64_t ptsValue = 0;
    int64_t ptsSpecialValue = 0;
    UnmarshalAudioMeta(std::string(paramItem->valuestring), ptsValue, ptsSpecialValue);
    AVTRANS_LOGI("buffer->GetPts(): %{public}" PRId64, ptsValue);
    outBuffer->pts_ = ptsValue;
    meta->SetData(Media::Tag::USER_FRAME_PTS, ptsSpecialValue);
    outBuffer->memory_->Write(reinterpret_cast<uint8_t *>(data->buf), data->bufLen, 0);
    outputBufQueProducer_->PushBuffer(outBuffer, true);
}

sptr<Media::AVBufferQueueProducer> AVTransBusInputFilter::GetInputBufQueProducer()
{
    return inputBufQueProducer_;
}

std::string AVTransBusInputFilter::TransName2PkgName(const std::string &ownerName)
{
    const static std::pair<std::string, std::string> mapArray[] = {
        {OWNER_NAME_D_MIC, PKG_NAME_D_AUDIO},
        {OWNER_NAME_D_VIRMODEM_MIC, PKG_NAME_D_CALL},
        {OWNER_NAME_D_CAMERA, PKG_NAME_D_CAMERA},
        {OWNER_NAME_D_SCREEN, PKG_NAME_D_SCREEN},
        {OWNER_NAME_D_SPEAKER, PKG_NAME_D_AUDIO},
        {OWNER_NAME_D_VIRMODEM_SPEAKER, PKG_NAME_D_CALL},
        {AV_SYNC_SENDER_CONTROL_SESSION_NAME, PKG_NAME_DH_FWK},
        {AV_SYNC_RECEIVER_CONTROL_SESSION_NAME, PKG_NAME_DH_FWK},
    };
    auto foundItem = std::find_if(std::begin(mapArray), std::end(mapArray),
        [&](const auto& item) { return item.first == ownerName; });
    if (foundItem != std::end(mapArray)) {
        return foundItem->second;
    }
    return EMPTY_STRING;
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS
