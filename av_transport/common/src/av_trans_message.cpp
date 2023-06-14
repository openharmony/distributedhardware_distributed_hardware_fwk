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
    nlohmann::json msgJson;
    msgJson[KEY_TYPE] = type_;
    msgJson[KEY_CONTENT] = content_;
    msgJson[KEY_DST_DEVID] = dstDevId_;
    return msgJson.dump();
}

bool AVTransMessage::UnmarshalMessage(const std::string& jsonStr)
{
    nlohmann::json msgJson = nlohmann::json::parse(jsonStr, nullptr, false);
    if (msgJson.is_discarded()) {
        return false;
    }
    if (!IsUInt32(msgJson, KEY_TYPE) || !IsString(msgJson, KEY_CONTENT) || !IsString(msgJson, KEY_DST_DEVID)) {
        return false;
    }
    type_ = msgJson[KEY_TYPE].get<uint32_t>();
    content_ = msgJson[KEY_CONTENT].get<std::string>();
    dstDevId_ = msgJson[KEY_DST_DEVID].get<std::string>();
    return true;
}

bool AVTransMessage::IsUInt32(const nlohmann::json &msgJson, const std::string &key)
{
    return msgJson.contains(key) && msgJson[key].is_number_unsigned() && msgJson[key] <= UINT32_MAX;
}

bool AVTransMessage::IsString(const nlohmann::json &msgJson, const std::string &key)
{
    return msgJson.contains(key) && msgJson[key].is_string() && msgJson[key].size() <= MAX_MESSAGES_LEN;
}
} // namespace DistributedHardware
} // namespace OHOS