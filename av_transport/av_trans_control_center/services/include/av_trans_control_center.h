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

#ifndef OHOS_AV_TRANS_CONTROL_CENTER_H
#define OHOS_AV_TRANS_CONTROL_CENTER_H

#include "av_sync_manager.h"
#include "av_trans_constants.h"
#include "av_trans_message.h"
#include "iav_trans_control_center_callback.h"
#include "single_instance.h"
#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {
class AVTransControlCenter : public ISoftbusChannelListener {
    DECLARE_SINGLE_INSTANCE_BASE(AVTransControlCenter);
public:
    AVTransControlCenter();
    virtual ~AVTransControlCenter();

    int32_t Initialize(const TransRole &transRole, int32_t &engineId);
    int32_t Release(int32_t engineId);
    int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId);
    int32_t Notify(int32_t engineId, const AVTransEvent &event);
    int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAVTransControlCenterCallback> &callback);

    // interfaces from ISoftbusChannelListener
    void OnChannelEvent(const AVTransEvent &event) override;
    void OnStreamReceived(const StreamData *data, const StreamData *ext) override;

public:
    int32_t SendMessage(const std::shared_ptr<AVTransMessage> &message);
    void SetParam2Engines(AVTransTag tag, const std::string &value);
    void SetParam2Engines(const AVTransSharedMemory &memory);

private:
    void HandleChannelEvent(const AVTransEvent &event);
    void HandleDataReceived(const std::string &content, const std::string &peerDevId);
    bool IsInvalidEngineId(int32_t engineId);

private:
    TransRole transRole_;
    std::string sessionName_;
    std::atomic<int32_t> rootEngineId_;
    std::atomic<bool> initialized_ {false};
    std::shared_ptr<AVSyncManager> syncManager_ = nullptr;

    std::mutex devIdMutex_;
    std::mutex engineIdMutex_;
    std::mutex callbackMutex_;

    std::vector<std::string> connectedDevIds_;
    std::map<int32_t, std::string> engine2DevIdMap_;
    std::map<int32_t, sptr<IAVTransControlCenterCallback>> callbackMap_;
};
}
}
#endif