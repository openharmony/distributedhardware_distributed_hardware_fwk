/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AV_TRANS_CONTROL_CENTER_KIT_H
#define OHOS_AV_TRANS_CONTROL_CENTER_KIT_H

#include <atomic>
#include <functional>
#include <mutex>

#include "single_instance.h"
#include "idistributed_hardware.h"

namespace OHOS {
namespace DistributedHardware {
class AVTransControlCenterKit {
    DECLARE_SINGLE_INSTANCE_BASE(AVTransControlCenterKit);

public:
    AVTransControlCenterKit();
    virtual ~AVTransControlCenterKit();

    int32_t Initialize(const TransRole &transRole, int32_t &engineId);
    int32_t Release(int32_t engineId);
    int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId);
    int32_t Notify(int32_t engineId, const AVTransEvent &event);
    int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAVTransControlCenterCallback> &callback);

private:
    sptr<IDistributedHardware> GetAVTransCtlCenterProxy();

private:
    std::mutex proxyMutex_;
    sptr<IDistributedHardware> ctlCenterProxy_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANS_CONTROL_CENTER_KIT_H