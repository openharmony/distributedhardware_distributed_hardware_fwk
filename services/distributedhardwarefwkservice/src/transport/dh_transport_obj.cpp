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

#include "dh_transport_obj.h"

#include "dh_utils_tool.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {

void ToJson(cJSON *jsonObject, const FullCapsRsp &capsRsp)
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

void FromJson(const cJSON *jsonObject, FullCapsRsp &capsRsp)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    std::string keyNetworkId(CAPS_RSP_NETWORKID_KEY);
    if (IsString(jsonObject, keyNetworkId)) {
        capsRsp.networkId = cJSON_GetObjectItem(jsonObject, CAPS_RSP_NETWORKID_KEY)->valuestring;
    }
    std::string keyCaps(CAPS_RSP_CAPS_KEY);
    if (IsArray(jsonObject, keyCaps)) {
        cJSON *capsArr = cJSON_GetObjectItem(jsonObject, CAPS_RSP_CAPS_KEY);
        int32_t arrSize = cJSON_GetArraySize(capsArr);
        for (int32_t i = 0; i < arrSize; i++) {
            cJSON *cap = cJSON_GetArrayItem(capsArr, i);
            std::shared_ptr<CapabilityInfo> capPtr = std::make_shared<CapabilityInfo>();
            FromJson(cap, *capPtr);
            capsRsp.caps.push_back(capPtr);
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
    const char *msg = commMsg.msg.c_str();
    cJSON_AddStringToObject(jsonObject, COMM_MSG_MSG_KEY, msg);
}

void FromJson(const cJSON *jsonObject, CommMsg &commMsg)
{
    if (jsonObject == nullptr) {
        DHLOGE("Json pointer is nullptr!");
        return;
    }
    std::string keyCode(COMM_MSG_CODE_KEY);
    if (IsInt32(jsonObject, keyCode)) {
        commMsg.code = cJSON_GetObjectItem(jsonObject, COMM_MSG_CODE_KEY)->valueint;
    }
    std::string keyMsg(COMM_MSG_MSG_KEY);
    if (IsString(jsonObject, keyMsg)) {
        commMsg.msg = cJSON_GetObjectItem(jsonObject, COMM_MSG_MSG_KEY)->valuestring;
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