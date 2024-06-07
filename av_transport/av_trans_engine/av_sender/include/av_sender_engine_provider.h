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

#ifndef OHOS_AV_SENDER_ENGINE_PROVIDER_H
#define OHOS_AV_SENDER_ENGINE_PROVIDER_H

#include <mutex>

#include "i_av_engine_provider.h"
#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {
class AVSenderEngineProvider : public IAVEngineProvider, public ISoftbusChannelListener {
public:
    AVSenderEngineProvider(const std::string ownerName);
    ~AVSenderEngineProvider() override;

    std::shared_ptr<IAVSenderEngine> CreateAVSenderEngine(const std::string &peerDevId) override;
    std::vector<std::shared_ptr<IAVSenderEngine>> GetAVSenderEngineList() override;
    int32_t RegisterProviderCallback(const std::shared_ptr<IAVEngineProviderCallback> &callback) override;

    // interfaces from ISoftbusChannelListener
    void OnChannelEvent(const AVTransEvent &event) override;
    void OnStreamReceived(const StreamData *data, const StreamData *ext) override;

private:
    std::string ownerName_;
    std::string sessionName_;
    std::mutex listMutex_;
    std::mutex callbackMutex_;
    std::shared_ptr<IAVEngineProviderCallback> providerCallback_;
    std::vector<std::shared_ptr<IAVSenderEngine>> senderEngineList_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_SENDER_ENGINE_PROVIDER_H