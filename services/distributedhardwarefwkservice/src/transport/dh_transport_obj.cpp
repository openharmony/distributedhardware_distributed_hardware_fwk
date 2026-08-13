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

#include "dh_transport_obj.h"

#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
void ToJson(cJSON *jsonObject, const FullCapsRsp &capsRsp, bool isSyncMeta)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    const char *networkId = capsRsp.networkId.c_str();
    cJSON_AddStringToObject(jsonObject, CAPS_RSP_NETWORKID_KEY, networkId);
    cJSON *capArr = cJSON_CreateArray();
    if (capArr == nullptr) {
        return;
    }
    if (isSyncMeta) {
        for (auto const &metaCap : capsRsp.metaCaps) {
            cJSON *capValJson = cJSON_CreateObject();
            if (capValJson == nullptr) {
                cJSON_Delete(capArr);
                return;
            }
            ToJson(capValJson, *metaCap);
            cJSON_AddItemToArray(capArr, capValJson);
        }
        cJSON_AddItemToObject(jsonObject, CAPS_RSP_CAPS_KEY, capArr);
    } else {
        for (auto const &cap : capsRsp.caps) {
            cJSON *capValJson = cJSON_CreateObject();
            if (capValJson == nullptr) {
                cJSON_Delete(capArr);
                return;
            }
            ToJson(capValJson, *cap);
            cJSON_AddItemToArray(capArr, capValJson);
        }
        cJSON_AddItemToObject(jsonObject, CAPS_RSP_CAPS_KEY, capArr);
    }
}

void FromJson(const cJSON *jsonObject, FullCapsRsp &capsRsp, bool isSyncMeta)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *capsRspJson = cJSON_GetObjectItem(jsonObject, CAPS_RSP_NETWORKID_KEY);
    if (!IsString(capsRspJson)) {
        DHLOGE("CAPS_RSP_NETWORKID_KEY is invalid!");
        return;
    }
    capsRsp.networkId = capsRspJson->valuestring;
    cJSON *capsRspKeyJson = cJSON_GetObjectItem(jsonObject, CAPS_RSP_CAPS_KEY);
    if (IsArray(capsRspKeyJson)) {
        int32_t arrSize = cJSON_GetArraySize(capsRspKeyJson);
        for (int32_t i = 0; i < arrSize; i++) {
            cJSON *cap = cJSON_GetArrayItem(capsRspKeyJson, i);
            if (cap == nullptr) {
                DHLOGE("get array failed, cap is nullptr");
                continue;
            }
            if (isSyncMeta) {
                std::shared_ptr<MetaCapabilityInfo> metaCapPtr = std::make_shared<MetaCapabilityInfo>();
                FromJson(cap, *metaCapPtr);
                capsRsp.metaCaps.push_back(metaCapPtr);
            } else {
                std::shared_ptr<CapabilityInfo> capPtr = std::make_shared<CapabilityInfo>();
                FromJson(cap, *capPtr);
                capsRsp.caps.push_back(capPtr);
            }
        }
    }
}

void ToJson(cJSON *jsonObject, const CommMsg &commMsg)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_CODE_KEY, commMsg.code);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_USERID_KEY, commMsg.userId);
    cJSON_AddNumberToObject(jsonObject, COMM_MSG_TOKENID_KEY, commMsg.tokenId);
    const char *msg = commMsg.msg.c_str();
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, msg);
    cJSON_AddStringToObject(jsonObject, COMM_MSG_ACCOUNTID_KEY, commMsg.accountId.c_str());
    cJSON_AddBoolToObject(jsonObject, COMM_MSG_SYNC_META_KEY, commMsg.isSyncMeta);
}

void FromJson(const cJSON *jsonObject, CommMsg &commMsg)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    cJSON *commMsgCodeJson = cJSON_GetObjectItem(jsonObject, COMM_MSG_CODE_KEY);
    if (IsInt32(commMsgCodeJson)) {
        commMsg.code = commMsgCodeJson->valueint;
    }
    cJSON *commMsgUserIdJson = cJSON_GetObjectItem(jsonObject, COMM_MSG_USERID_KEY);
    if (commMsgUserIdJson != NULL && cJSON_IsNumber(commMsgUserIdJson)) {
        commMsg.userId = commMsgUserIdJson->valueint;
    }
    cJSON *commMsgTokenIdJson = cJSON_GetObjectItem(jsonObject, COMM_MSG_TOKENID_KEY);
    if (commMsgTokenIdJson != NULL && cJSON_IsNumber(commMsgTokenIdJson)) {
        commMsg.tokenId = static_cast<uint64_t>(commMsgTokenIdJson->valueint);
    }
    cJSON *commMsgeJson = cJSON_GetObjectItem(jsonObject, COMM_MSG_MSG_KEY);
    if (commMsgeJson != NULL && cJSON_IsString(commMsgeJson)) {
        commMsg.msg = commMsgeJson->valuestring;
    }
    cJSON *commMsgeAccountIdJson = cJSON_GetObjectItem(jsonObject, COMM_MSG_ACCOUNTID_KEY);
    if (commMsgeAccountIdJson != NULL && cJSON_IsString(commMsgeAccountIdJson)) {
        commMsg.accountId = commMsgeAccountIdJson->valuestring;
    }
    cJSON *commMsgeSyncJson = cJSON_GetObjectItem(jsonObject, COMM_MSG_SYNC_META_KEY);
    if (commMsgeSyncJson != NULL && cJSON_IsBool(commMsgeSyncJson)) {
        commMsg.isSyncMeta = commMsgeSyncJson->valueint;
    }
}

std::string GetCommMsgString(const CommMsg &commMsg)
{
    cJSON *rootMsg = cJSON_CreateObject();
    if (rootMsg == nullptr) {
        DHLOGE("Create cJSON object failed.");
        return "";
    }
    ToJson(rootMsg, commMsg);
    char *msg = cJSON_PrintUnformatted(rootMsg);
    if (msg == nullptr) {
        cJSON_Delete(rootMsg);
        return "";
    }
    std::string msgStr = std::string(msg);
    cJSON_free(msg);
    cJSON_Delete(rootMsg);

    return msgStr;
}
} // DistributedHardware
} // OHOS