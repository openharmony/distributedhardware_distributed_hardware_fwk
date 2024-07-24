/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include <cinttypes>

#include "constants.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "low_latency_listener.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "LowLatencyListener"

LowLatencyListener::LowLatencyListener()
{
    DHLOGI("LowLatencyListener ctor!");
}

LowLatencyListener::~LowLatencyListener()
{
    DHLOGI("LowLatencyListener dtor!");
}

void LowLatencyListener::OnMessage(const DHTopic topic, const std::string& message)
{
    (void) topic;
    (void) message;
#ifdef DHARDWARE_LOW_LATENCY
    if (topic <= DHTopic::TOPIC_MIN || topic >= DHTopic::TOPIC_MAX) {
        DHLOGE("Topic is invalid, topic: %{public}" PRIu32, (uint32_t)topic);
        return;
    }
    if (!IsMessageLengthValid(message)) {
        return;
    }
    cJSON *jsonObj = cJSON_Parse(message.c_str());
    if (jsonObj == NULL) {
        DHLOGE("jsonStr parse failed");
        return;
    }
    if (!IsUInt32(jsonObj, DH_TYPE)) {
        DHLOGE("The DH_TYPE key is invalid!");
        cJSON_Delete(jsonObj);
        return;
    }
    if (!IsBool(jsonObj, LOW_LATENCY_ENABLE)) {
        DHLOGE("The LOW_LATENCY_ENABLE key is invalid!");
        cJSON_Delete(jsonObj);
        return;
    }
    DHType dhType = (DHType)cJSON_GetObjectItem(jsonObj, DH_TYPE.c_str())->valuedouble;
    cJSON *isEnable = cJSON_GetObjectItem(jsonObj, LOW_LATENCY_ENABLE.c_str());
    if (cJSON_IsTrue(isEnable)) {
        LowLatency::GetInstance().EnableLowLatency(dhType);
    } else {
        LowLatency::GetInstance().DisableLowLatency(dhType);
    }
    cJSON_Delete(jsonObj);
#endif
}

sptr<IRemoteObject> LowLatencyListener::AsObject()
{
    return nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
