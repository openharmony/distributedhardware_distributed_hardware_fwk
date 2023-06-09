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

#ifndef OHOS_AV_TRANSPORT_MESSAGE_H
#define OHOS_AV_TRANSPORT_MESSAGE_H

#include <string>
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
class AVTransMessage {
public:
    AVTransMessage();
    AVTransMessage(uint32_t type, std::string content, std::string dstDevId);
    ~AVTransMessage();

    std::string MarshalMessage();
    bool UnmarshalMessage(const std::string &jsonStr);

public:
    uint32_t type_;
    std::string content_;
    std::string dstDevId_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANSPORT_MESSAGE_H