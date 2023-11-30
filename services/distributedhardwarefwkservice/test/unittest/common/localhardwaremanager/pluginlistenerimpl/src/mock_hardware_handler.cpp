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

#include "mock_hardware_handler.h"

#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {
int32_t MockHardwareHandler::Initialize()
{
    return DH_FWK_SUCCESS;
}

std::vector<DHItem> MockHardwareHandler::Query()
{
    DHItem item;
    return std::vector<DHItem> { item };
}

std::map<std::string, std::string> MockHardwareHandler::QueryExtraInfo()
{
    std::map<std::string, std::string> extraInfo;
    return extraInfo;
}

bool MockHardwareHandler::IsSupportPlugin()
{
    return true;
}

void MockHardwareHandler::RegisterPluginListener(std::shared_ptr<PluginListener> listener)
{
    listener_ = listener;
}

void MockHardwareHandler::UnRegisterPluginListener()
{
    listener_ = nullptr;
}

int32_t MockHardwareHandler::PluginHardware(const std::string &dhId, const std::string &attr,
    const std::string &subtype)
{
    if (listener_ != nullptr) {
        listener_->PluginHardware(dhId, attr, subtype);
    }
    return DH_FWK_SUCCESS;
}

int32_t MockHardwareHandler::UnPluginHardware(const std::string &dhId)
{
    if (listener_ != nullptr) {
        listener_->UnPluginHardware(dhId);
    }
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS