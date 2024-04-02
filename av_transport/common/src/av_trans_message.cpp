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

#include "av_trans_message.h"

#include "av_trans_constants.h"

namespace OHOS {
namespace DistributedHardware {
const std::string KEY_TYPE = "type";
const std::string KEY_CONTENT = "content";
const std::string KEY_DST_DEVID = "dstDevId";

AVTransMessage::AVTransMessage()
{
    type_ = 0;
}

AVTransMessage::AVTransMessage(uint32_t type, std::string content, std::string dstDevId)
    : type_(type), content_(content), dstDevId_(dstDevId)
{
}

AVTransMessage::~AVTransMessage()
{
}

std::string AVTransMessage::MarshalMessage()
{
    cJSON *msgJson = cJSON_CreateObject();
    if (msgJson == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(msgJson, KEY_TYPE.c_str(), type_);
    cJSON_AddStringToObject(msgJson, KEY_CONTENT.c_str(), content_.c_str());
    cJSON_AddStringToObject(msgJson, KEY_DST_DEVID.c_str(), dstDevId_.c_str());
    char *data = cJSON_Print(msgJson);
    if (data == nullptr) {
        cJSON_Delete(msgJson);
        return "";
    }
    std::string jsonstr(data);
    cJSON_free(data);
    cJSON_Delete(msgJson);
    return jsonstr;
}

bool AVTransMessage::UnmarshalMessage(const std::string& jsonStr, const std::string &peerDevId)
{
    cJSON *metaJson = cJSON_Parse(jsonStr.c_str());
    if (metaJson == nullptr) {
        return false;
    }
    if (!IsUInt32(metaJson, KEY_TYPE) || !IsString(metaJson, KEY_CONTENT)) {
        cJSON_Delete(metaJson);
        return false;
    }
    cJSON *typeObj = cJSON_GetObjectItemCaseSensitive(metaJson, KEY_TYPE.c_str());
    if (typeObj == nullptr || !cJSON_IsNumber(typeObj)) {
        cJSON_Delete(metaJson);
        return false;
    }

    type_ = typeObj->valueint;
    cJSON *contentObj = cJSON_GetObjectItemCaseSensitive(metaJson, KEY_CONTENT.c_str());
    if (contentObj == nullptr || !cJSON_IsString(contentObj)) {
        cJSON_Delete(metaJson);
        return false;
    }
    content_ = contentObj->valuestring;
    dstDevId_ = peerDevId;
    cJSON_Delete(metaJson);
    return true;
}

bool AVTransMessage::IsUInt32(const cJSON *msgJson, const std::string &key)
{
    cJSON *keyObj = cJSON_GetObjectItemCaseSensitive(msgJson, key.c_str());
    return (keyObj != nullptr) && cJSON_IsNumber(keyObj) &&
        static_cast<uint32_t>(keyObj->valueint) <= UINT32_MAX;
}

bool AVTransMessage::IsString(const cJSON *msgJson, const std::string &key)
{
    cJSON *keyObj = cJSON_GetObjectItemCaseSensitive(msgJson, key.c_str());
    return (keyObj != nullptr) && cJSON_IsString(keyObj) &&
        strlen(cJSON_GetStringValue(keyObj)) <= MAX_MESSAGES_LEN;
}
} // namespace DistributedHardware
} // namespace OHOS