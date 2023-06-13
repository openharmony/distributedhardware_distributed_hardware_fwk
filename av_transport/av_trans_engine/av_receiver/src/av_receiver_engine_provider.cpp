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

#include "av_receiver_engine_provider.h"

#include "av_receiver_engine.h"

namespace OHOS {
namespace DistributedHardware {
AVReceiverEngineProvider::AVReceiverEngineProvider(const std::string &ownerName) : ownerName_(ownerName)
{
    DHLOGI("AVReceiverEngineProvider ctor.");
    sessionName_ = ownerName + "_" + RECEIVER_CONTROL_SESSION_NAME_SUFFIX;
    SoftbusChannelAdapter::GetInstance().CreateChannelServer(ownerName, sessionName_);
    SoftbusChannelAdapter::GetInstance().RegisterChannelListener(sessionName_, AV_TRANS_SPECIAL_DEVICE_ID, this);
}

AVReceiverEngineProvider::~AVReceiverEngineProvider()
{
    DHLOGI("AVReceiverEngineProvider dctor.");
    {
        std::lock_guard<std::mutex> lock(listMutex_);
        for (auto &receiver : receiverEngineList_) {
            receiver->Release();
        }
    }
    SoftbusChannelAdapter::GetInstance().RemoveChannelServer(ownerName_, sessionName_);
    SoftbusChannelAdapter::GetInstance().UnRegisterChannelListener(sessionName_, AV_TRANS_SPECIAL_DEVICE_ID);
    ownerName_ = "";
    sessionName_ = "";
    receiverEngineList_.clear();
    providerCallback_ = nullptr;
}

std::shared_ptr<IAVReceiverEngine> AVReceiverEngineProvider::CreateAVReceiverEngine(const std::string &peerDevId)
{
    DHLOGI("CreateAVReceiverEngine enter.");
    auto receiver = std::make_shared<AVReceiverEngine>(ownerName_, peerDevId);
    if (receiver && receiver->Initialize() == DH_AVT_SUCCESS) {
        {
            std::lock_guard<std::mutex> lock(listMutex_);
            receiverEngineList_.push_back(receiver);
        }
        return receiver;
    }
    DHLOGE("create receiver failed or receiver init failed.");
    return nullptr;
}

std::vector<std::shared_ptr<IAVReceiverEngine>> AVReceiverEngineProvider::GetAVReceiverEngineList()
{
    std::lock_guard<std::mutex> lock(listMutex_);
    return receiverEngineList_;
}

int32_t AVReceiverEngineProvider::RegisterProviderCallback(
    const std::shared_ptr<IAVEngineProviderCallback> &callback)
{
    providerCallback_ = callback;
    return DH_AVT_SUCCESS;
}

void AVReceiverEngineProvider::OnChannelEvent(const AVTransEvent &event)
{
    if ((providerCallback_ != nullptr) &&
        ((event.type == EventType::EVENT_CHANNEL_OPENED) || (event.type == EventType::EVENT_CHANNEL_CLOSED))) {
        DHLOGI("on receiver channel event. event type:%" PRId32, event.type);
        providerCallback_->OnProviderEvent(event);
    }
}

void AVReceiverEngineProvider::OnStreamReceived(const StreamData *data, const StreamData *ext)
{
    (void)data;
    (void)ext;
}
} // namespace DistributedHardware
} // namespace OHOS

extern "C" __attribute__((visibility("default")))
    OHOS::DistributedHardware::IAVEngineProvider* GetAVReceiverEngineProvider(const std::string ownerName)
{
    return new (std::nothrow) OHOS::DistributedHardware::AVReceiverEngineProvider(ownerName);
}