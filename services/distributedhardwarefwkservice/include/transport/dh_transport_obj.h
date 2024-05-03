/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_TRANSPORT_OBJ_H
#define OHOS_DISTRIBUTED_HARDWARE_TRANSPORT_OBJ_H

#include <cstdint>
#include <string>
#include <cJSON.h>

#include "capability_info.h"

namespace OHOS {
namespace DistributedHardware {
const char* const CAPS_RSP_NETWORKID_KEY = "networkId";
const char* const CAPS_RSP_CAPS_KEY = "caps";
const char* const COMM_MSG_CODE_KEY = "code";
const char* const COMM_MSG_MSG_KEY = "msg";

struct FullCapsRsp {
    // the networkd id of rsp from which device
    std::string networkId;
    // the full dh caps
    std::vector<std::shared_ptr<CapabilityInfo>> caps;
    FullCapsRsp() : networkId(""), caps({}) {}
    FullCapsRsp(std::string networkId, std::vector<std::shared_ptr<CapabilityInfo>> caps) : networkId(networkId),
        caps(caps) {}
};

void ToJson(cJSON *jsonObject, const FullCapsRsp &capsRsp);
void FromJson(const cJSON *jsonObject, FullCapsRsp &capsRsp);

struct CommMsg {
    int32_t code;
    std::string msg;
    CommMsg() : code(-1), msg("") {}
    CommMsg(int32_t code, std::string msg) : code(code), msg(msg) {}
};

void ToJson(cJSON *jsonObject, const CommMsg &commMsg);
void FromJson(const cJSON *jsonObject, CommMsg &commMsg);

std::string GetCommMsgString(const CommMsg &commMsg);

} // DistributedHardware
} // OHOS
#endif