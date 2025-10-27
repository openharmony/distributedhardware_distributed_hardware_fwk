/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "meta_capability_info.h"

namespace OHOS {
namespace DistributedHardware {
const char* const CAPS_RSP_NETWORKID_KEY = "networkId";
const char* const CAPS_RSP_CAPS_KEY = "caps";
const char* const COMM_MSG_CODE_KEY = "code";
const char* const COMM_MSG_MSG_KEY = "msg";
const char* const COMM_MSG_USERID_KEY = "userId";
const char* const COMM_MSG_TOKENID_KEY = "tokenId";
const char* const COMM_MSG_ACCOUNTID_KEY = "accountId";
const char* const COMM_MSG_SYNC_META_KEY = "sync_meta";

struct FullCapsRsp {
    // the networkd id of rsp from which device
    std::string networkId;
    // the full dh caps
    std::vector<std::shared_ptr<CapabilityInfo>> caps;
    // the full dh metacaps
    std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCaps;
    FullCapsRsp() : networkId(""), caps({}), metaCaps({}) {}
    FullCapsRsp(std::string networkId, std::vector<std::shared_ptr<CapabilityInfo>> caps,
        std::vector<std::shared_ptr<MetaCapabilityInfo>> metaCaps) : networkId(networkId), caps(caps),
        metaCaps(metaCaps) {}
};

void ToJson(cJSON *jsonObject, const FullCapsRsp &capsRsp, bool isSyncMeta);
void FromJson(const cJSON *jsonObject, FullCapsRsp &capsRsp, bool isSyncMeta);

struct CommMsg {
    int32_t code;
    int32_t userId;
    uint64_t tokenId;
    std::string msg;
    std::string accountId;
    bool isSyncMeta;
    CommMsg() : code(-1), userId(-1), tokenId(0), msg(""), accountId(""), isSyncMeta(false) {}
    CommMsg(int32_t code, int32_t userId, uint64_t tokenId, std::string msg, std::string accountId,
        bool isSyncMeta) : code(code), userId(userId), tokenId(tokenId), msg(msg),
        accountId(accountId), isSyncMeta(isSyncMeta) {}
};

void ToJson(cJSON *jsonObject, const CommMsg &commMsg);
void FromJson(const cJSON *jsonObject, CommMsg &commMsg);

std::string GetCommMsgString(const CommMsg &commMsg);

} // DistributedHardware
} // OHOS
#endif