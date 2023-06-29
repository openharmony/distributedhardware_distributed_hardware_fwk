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

#include "av_trans_control_center.h"

#include "anonymous_string.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(AVTransControlCenter);

AVTransControlCenter::AVTransControlCenter()
{
    AVTRANS_LOGI("AVTransControlCenter ctor.");
    transRole_ = TransRole::UNKNOWN;
    rootEngineId_.store(BASE_ENGINE_ID);
    syncManager_ = std::make_shared<AVSyncManager>();
}

AVTransControlCenter::~AVTransControlCenter()
{
    AVTRANS_LOGI("AVTransControlCenter dtor.");
    SoftbusChannelAdapter::GetInstance().RemoveChannelServer(PKG_NAME_DH_FWK, sessionName_);

    sessionName_ = "";
    initialized_ = false;
    syncManager_ = nullptr;
    transRole_ = TransRole::UNKNOWN;
    rootEngineId_.store(BASE_ENGINE_ID);
}

int32_t AVTransControlCenter::Initialize(const TransRole &transRole, int32_t &engineId)
{
    engineId = INVALID_ENGINE_ID;
    if ((transRole != TransRole::AV_SENDER) && (transRole != TransRole::AV_RECEIVER)) {
        AVTRANS_LOGE("Invalid trans role=%d", transRole);
        return ERR_DH_AVT_INVALID_PARAM_VALUE;
    }

    if (initialized_.load()) {
        AVTRANS_LOGI("AV control center already initialized.");
        engineId = rootEngineId_.load();
        rootEngineId_++;
        return DH_AVT_SUCCESS;
    }

    sessionName_ = (transRole == TransRole::AV_SENDER) ? AV_SYNC_SENDER_CONTROL_SESSION_NAME :
        AV_SYNC_RECEIVER_CONTROL_SESSION_NAME;
    int32_t ret = SoftbusChannelAdapter::GetInstance().CreateChannelServer(PKG_NAME_DH_FWK, sessionName_);
    TRUE_RETURN_V_MSG_E((ret != DH_AVT_SUCCESS), ret, "Create contro center session server failed, ret=%d", ret);

    ret = SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, AV_TRANS_SPECIAL_DEVICE_ID, this);
    TRUE_RETURN_V_MSG_E((ret != DH_AVT_SUCCESS), ret, "Register control center channel callback failed, ret=%d", ret);

    initialized_ = true;
    transRole_ = transRole;
    engineId = rootEngineId_.load();
    rootEngineId_++;

    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenter::Release(int32_t engineId)
{
    AVTRANS_LOGI("Release control center channel for engineId=%d.", engineId);
    TRUE_RETURN_V_MSG_E(IsInvalidEngineId(engineId), ERR_DH_AVT_INVALID_PARAM_VALUE,
        "Invalid input engine id = %d", engineId);

    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callbackMap_.erase(engineId);
    }

    std::string peerDevId;
    {
        std::lock_guard<std::mutex> lock(engineIdMutex_);
        if (engine2DevIdMap_.find(engineId) == engine2DevIdMap_.end()) {
            AVTRANS_LOGE("Input engine id is not exist, engineId = %d", engineId);
            return DH_AVT_SUCCESS;
        }
        peerDevId = engine2DevIdMap_[engineId];
        engine2DevIdMap_.erase(engineId);

        bool IsDevIdUsedByOthers = false;
        for (auto it = engine2DevIdMap_.begin(); it != engine2DevIdMap_.end(); it++) {
            if (it->second == peerDevId) {
                IsDevIdUsedByOthers = true;
                break;
            }
        }
        if (IsDevIdUsedByOthers) {
            AVTRANS_LOGI("Control channel is still being used by other engine, peerDevId=%s.",
                GetAnonyString(peerDevId).c_str());
            return DH_AVT_SUCCESS;
        }
    }

    {
        std::lock_guard<std::mutex> lock(devIdMutex_);
        auto iter = std::find(connectedDevIds_.begin(), connectedDevIds_.end(), peerDevId);
        if (iter == connectedDevIds_.end()) {
            AVTRANS_LOGE("Control channel has not been opened successfully for peerDevId=%s.",
                GetAnonyString(peerDevId).c_str());
            return DH_AVT_SUCCESS;
        } else {
            connectedDevIds_.erase(iter);
        }
    }

    SoftbusChannelAdapter::GetInstance().StopDeviceTimeSync(PKG_NAME_DH_FWK, sessionName_, peerDevId);
    SoftbusChannelAdapter::GetInstance().CloseSoftbusChannel(sessionName_, peerDevId);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, peerDevId);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, AV_TRANS_SPECIAL_DEVICE_ID);

    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenter::CreateControlChannel(int32_t engineId, const std::string &peerDevId)
{
    AVTRANS_LOGI("Create control center channel for engineId=%d, peerDevId=%s.", engineId,
        GetAnonyString(peerDevId).c_str());

    TRUE_RETURN_V_MSG_E(IsInvalidEngineId(engineId), ERR_DH_AVT_INVALID_PARAM_VALUE,
        "Invalid input engine id = %d", engineId);

    TRUE_RETURN_V_MSG_E(!initialized_.load(), ERR_DH_AVT_CREATE_CHANNEL_FAILED,
        "AV control center has not been initialized.");

    {
        std::lock_guard<std::mutex> devLock(devIdMutex_);
        auto iter = std::find(connectedDevIds_.begin(), connectedDevIds_.end(), peerDevId);
        if (iter != connectedDevIds_.end()) {
            {
                std::lock_guard<std::mutex> lock(engineIdMutex_);
                engine2DevIdMap_.insert(std::make_pair(engineId, peerDevId));
            }
            AVTRANS_LOGE("AV control center channel has already created, peerDevId=%s.",
                GetAnonyString(peerDevId).c_str());
            return ERR_DH_AVT_CHANNEL_ALREADY_CREATED;
        }
    }

    std::string peerSessName = (transRole_ == TransRole::AV_SENDER) ? AV_SYNC_RECEIVER_CONTROL_SESSION_NAME :
        AV_SYNC_SENDER_CONTROL_SESSION_NAME;
    int32_t ret = SoftbusChannelAdapter::GetInstance().OpenSoftbusChannel(sessionName_, peerSessName, peerDevId);
    TRUE_RETURN_V_MSG_E(((ret != DH_AVT_SUCCESS) && (ret != ERR_DH_AVT_SESSION_HAS_OPENED)), ret,
        "Create av control center channel failed, ret=%d", ret);

    std::lock_guard<std::mutex> lk(engineIdMutex_);
    engine2DevIdMap_.insert(std::make_pair(engineId, peerDevId));

    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenter::Notify(int32_t engineId, const AVTransEvent& event)
{
    TRUE_RETURN_V_MSG_E(IsInvalidEngineId(engineId), ERR_DH_AVT_INVALID_PARAM_VALUE,
        "Invalid input engine id = %d", engineId);

    switch (event.type) {
        case EventType::EVENT_ADD_STREAM: {
            syncManager_->AddStreamInfo(AVStreamInfo{ event.content, event.peerDevId });
            break;
        }
        case EventType::EVENT_REMOVE_STREAM: {
            syncManager_->RemoveStreamInfo(AVStreamInfo{ event.content, event.peerDevId });
            break;
        }
        default:
            AVTRANS_LOGE("Unsupported event type.");
    }
    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenter::RegisterCtlCenterCallback(int32_t engineId,
    const sptr<IAVTransControlCenterCallback> &callback)
{
    TRUE_RETURN_V_MSG_E(IsInvalidEngineId(engineId), ERR_DH_AVT_INVALID_PARAM_VALUE,
        "Invalid input engine id = %d", engineId);

    if (callback == nullptr) {
        AVTRANS_LOGE("Input callback is nullptr.");
        return ERR_DH_AVT_INVALID_PARAM_VALUE;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    callbackMap_.insert(std::make_pair(engineId, callback));

    return DH_AVT_SUCCESS;
}

int32_t AVTransControlCenter::SendMessage(const std::shared_ptr<AVTransMessage> &message)
{
    AVTRANS_LOGI("SendMessage enter.");
    TRUE_RETURN_V_MSG_E(message == nullptr, ERR_DH_AVT_INVALID_PARAM, "Input message is nullptr.");

    std::string msgData = message->MarshalMessage();
    return SoftbusChannelAdapter::GetInstance().SendBytesData(sessionName_, message->dstDevId_, msgData);
}

void AVTransControlCenter::SetParam2Engines(AVTransTag tag, const std::string &value)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    for (auto iter = callbackMap_.begin(); iter != callbackMap_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->SetParameter(tag, value);
        }
    }
}

void AVTransControlCenter::SetParam2Engines(const AVTransSharedMemory &memory)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    for (auto iter = callbackMap_.begin(); iter != callbackMap_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->SetSharedMemory(memory);
        }
    }
}

void AVTransControlCenter::OnChannelEvent(const AVTransEvent &event)
{
    AVTRANS_LOGI("OnChannelEvent enter. event type:%d", event.type);
    switch (event.type) {
        case EventType::EVENT_CHANNEL_OPENED:
        case EventType::EVENT_CHANNEL_CLOSED:
        case EventType::EVENT_CHANNEL_OPEN_FAIL: {
            HandleChannelEvent(event);
            break;
        }
        case EventType::EVENT_DATA_RECEIVED: {
            HandleDataReceived(event.content, event.peerDevId);
            break;
        }
        case EventType::EVENT_TIME_SYNC_RESULT: {
            SetParam2Engines(AVTransTag::TIME_SYNC_RESULT, event.content);
            break;
        }
        default:
            AVTRANS_LOGE("Unsupported event type.");
    }
}

void AVTransControlCenter::HandleChannelEvent(const AVTransEvent &event)
{
    if (event.type == EventType::EVENT_CHANNEL_CLOSED) {
        AVTRANS_LOGI("Control channel has been closed.");
        return;
    }

    if (event.type == EventType::EVENT_CHANNEL_OPEN_FAIL) {
        AVTRANS_LOGE("Open control channel failed for peerDevId=%s.", GetAnonyString(event.peerDevId).c_str());
        return;
    }

    if (event.type == EventType::EVENT_CHANNEL_OPENED) {
        if (transRole_ == TransRole::AV_RECEIVER) {
            SoftbusChannelAdapter::GetInstance().StartDeviceTimeSync(PKG_NAME_DH_FWK, sessionName_, event.peerDevId);
        }
        std::lock_guard<std::mutex> lock(devIdMutex_);
        connectedDevIds_.push_back(event.peerDevId);
    }
}

void AVTransControlCenter::HandleDataReceived(const std::string &content, const std::string &peerDevId)
{
    auto avMessage = std::make_shared<AVTransMessage>();
    if (!avMessage->UnmarshalMessage(content, peerDevId)) {
        AVTRANS_LOGE("unmarshal event content to av message failed");
        return;
    }
    AVTRANS_LOGI("Handle data received, av message type = %d", avMessage->type_);
    if ((avMessage->type_ == (uint32_t)AVTransTag::START_AV_SYNC) ||
        (avMessage->type_ == (uint32_t)AVTransTag::STOP_AV_SYNC)) {
        syncManager_->HandleAvSyncMessage(avMessage);
    }
}

void AVTransControlCenter::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

bool AVTransControlCenter::IsInvalidEngineId(int32_t engineId)
{
    return (engineId < BASE_ENGINE_ID) || (engineId > rootEngineId_.load());
}
}
}