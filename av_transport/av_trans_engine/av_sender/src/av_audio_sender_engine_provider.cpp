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

#include "av_audio_sender_engine_provider.h"

#include "av_audio_sender_engine.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "AVAudioSenderEngineProvider"

AVAudioSenderEngineProvider::AVAudioSenderEngineProvider(const std::string ownerName) : ownerName_(ownerName)
{
    AVTRANS_LOGI("AVAudioSenderEngineProvider ctor.");
}

AVAudioSenderEngineProvider::~AVAudioSenderEngineProvider()
{
    AVTRANS_LOGI("AVAudioSenderEngineProvider dctor.");
    std::lock_guard<std::mutex> lock(listMutex_);
    for (auto &sender : senderEngineList_) {
        if (sender == nullptr) {
            continue;
        }
        sender->Release();
    }
    if (ownerName_ == OWNER_NAME_D_MIC || ownerName_ == OWNER_NAME_D_VIRMODEM_MIC) {
        SoftbusChannelAdapter::GetInstance().RemoveChannelServer(TransName2PkgName(ownerName_), sessionName_);
        SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, AV_TRANS_SPECIAL_DEVICE_ID);
    }
    ownerName_ = "";
    sessionName_ = "";
    senderEngineList_.clear();
    providerCallback_ = nullptr;
}

std::shared_ptr<IAVSenderEngine> AVAudioSenderEngineProvider::CreateAVSenderEngine(const std::string &peerDevId)
{
    AVTRANS_LOGI("CreateAVSenderEngine enter.");
    auto sender = std::make_shared<AVAudioSenderEngine>(ownerName_, peerDevId);
    if (sender && sender->Initialize() == DH_AVT_SUCCESS) {
        {
            std::lock_guard<std::mutex> lock(listMutex_);
            senderEngineList_.push_back(sender);
        }
        return sender;
    }
    AVTRANS_LOGE("create sender failed or sender init failed.");
    return nullptr;
}

std::vector<std::shared_ptr<IAVSenderEngine>> AVAudioSenderEngineProvider::GetAVSenderEngineList()
{
    std::lock_guard<std::mutex> lock(listMutex_);
    return senderEngineList_;
}

int32_t AVAudioSenderEngineProvider::RegisterProviderCallback(
    const std::shared_ptr<IAVEngineProviderCallback> &callback)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    providerCallback_ = callback;
    return DH_AVT_SUCCESS;
}

void AVAudioSenderEngineProvider::OnChannelEvent(const AVTransEvent &event)
{
    if (providerCallback_ == nullptr) {
        AVTRANS_LOGE("providerCallback_ is nullptr");
        return;
    }
    if ((event.type == EventType::EVENT_CHANNEL_OPENED) || (event.type == EventType::EVENT_CHANNEL_CLOSED)) {
        AVTRANS_LOGI("on receiver channel event. event type:%{public}" PRId32, event.type);
        providerCallback_->OnProviderEvent(event);
    }
}

void AVAudioSenderEngineProvider::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}
std::string AVAudioSenderEngineProvider::TransName2PkgName(const std::string &ownerName)
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
    OHOS::DistributedHardware::IAVEngineProvider* GetAVAudioSenderEngineProvider(const std::string ownerName)
{
    return new (std::nothrow) OHOS::DistributedHardware::AVAudioSenderEngineProvider(ownerName);
}