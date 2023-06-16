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

#ifndef OHOS_I_AV_TRANSPORT_CONTROL_CENTER_CALLBACK_H
#define OHOS_I_AV_TRANSPORT_CONTROL_CENTER_CALLBACK_H

#include "iremote_broker.h"

#include "av_sync_utils.h"
#include "av_trans_errno.h"
#include "av_trans_types.h"

namespace OHOS {
namespace DistributedHardware {
class IAVTransControlCenterCallback : public IRemoteBroker {
public:
    virtual int32_t SetParameter(AVTransTag tag, const std::string &value) = 0;
	virtual int32_t SetSharedMemory(const AVTransSharedMemory &memory) = 0;
    virtual int32_t Notify(const AVTransEvent &event) = 0;

    enum class Message : uint32_t {
        SET_PARAMETER = 1,
		SET_SHARED_MEMORY = 2,
        NOTIFY_AV_EVENT = 3
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.distributedhardware.IAVTransControlCenterCallback");
};
}
}
#endif