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

#include "av_audio_receiver_engine_provider.h"

#include "av_audio_receiver_engine.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AVAudioReceiverEngineProvider"

AVAudioReceiverEngineProvider::AVAudioReceiverEngineProvider(const std::string &ownerName) : ownerName_(ownerName)
{
    AVTRANS_LOGI("AVAudioReceiverEngineProvider ctor.");
}

AVAudioReceiverEngineProvider::~AVAudioReceiverEngineProvider()
{
    AVTRANS_LOGI("AVAudioReceiverEngineProvider dctor.");
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (auto &receiver : receiverEngineList_) {
            if (receiver == nullptr) {
                continue;
            }
            receiver->Release();
        }
    }
    if (ownerName_ != OWNER_NAME_D_MIC && ownerName_ != OWNER_NAME_D_VIRMODEM_MIC) {
        SoftbusChannelAdapter::GetInstance().RemoveChannelServer(TransName2PkgName(ownerName_), sessionName_);
        SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, AV_TRANS_SPECIAL_DEVICE_ID);
    }
    ownerName_ = "";
    sessionName_ = "";
    receiverEngineList_.clear();
    providerCallback_ = nullptr;
}

std::shared_ptr<IAVReceiverEngine> AVAudioReceiverEngineProvider::CreateAVReceiverEngine(const std::string &peerDevId)
{
    AVTRANS_LOGI("CreateAVReceiverEngine enter.");
    auto receiver = std::make_shared<AVAudioReceiverEngine>(ownerName_, peerDevId);
    if (receiver && receiver->Initialize() == DH_AVT_SUCCESS) {
        {
            std::lock_guard<std::mutex> lock(listMutex_);
            receiverEngineList_.push_back(receiver);
        }
        return receiver;
    }
    AVTRANS_LOGE("create receiver failed or receiver init failed.");
    return nullptr;
}

std::vector<std::shared_ptr<IAVReceiverEngine>> AVAudioReceiverEngineProvider::GetAVReceiverEngineList()
{
    std::lock_guard<std::mutex> lock(listMutex_);
    return receiverEngineList_;
}

int32_t AVAudioReceiverEngineProvider::RegisterProviderCallback(
    const std::shared_ptr<IAVEngineProviderCallback> &callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    providerCallback_ = callback;
    return DH_AVT_SUCCESS;
}

void AVAudioReceiverEngineProvider::OnChannelEvent(const AVTransEvent &event)
{
    if (providerCallback_ == nullptr) {
        AVTRANS_LOGE("providerCallback_ is nullptr.");
        return;
    }
    if ((event.type == EventType::EVENT_CHANNEL_OPENED) || (event.type == EventType::EVENT_CHANNEL_CLOSED)) {
        AVTRANS_LOGI("on receiver channel event. event type:%{public}" PRId32, event.type);
        providerCallback_->OnProviderEvent(event);
    }
}

void AVAudioReceiverEngineProvider::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}

std::string AVAudioReceiverEngineProvider::TransName2PkgName(const std::string &ownerName)
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
} // namespace DistributedHardware
} // namespace OHOS

extern "C" __attribute__((visibility("default")))
    OHOS::DistributedHardware::IAVEngineProvider* GetAVAudioReceiverEngineProvider(const std::string ownerName)
{
    return new (std::nothrow) OHOS::DistributedHardware::AVAudioReceiverEngineProvider(ownerName);
}