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
#ifndef OHOS_DISTRIBUTED_HARDWARE_MCOK_HARDWARE_HANDLER_H
#define OHOS_DISTRIBUTED_HARDWARE_MCOK_HARDWARE_HANDLER_H

#include <vector>

#include "ihardware_handler.h"

namespace OHOS {
namespace DistributedHardware {
class MockHardwareHandler : public IHardwareHandler {
public:
    MockHardwareHandler() = default;
    virtual ~MockHardwareHandler() = default;

    int32_t Initialize() override;
    std::vector<DHItem> Query() override;
    std::map<std::string, std::string> QueryExtraInfo() override;
    bool IsSupportPlugin() override;
    void RegisterPluginListener(std::shared_ptr<PluginListener> listener) override;
    void UnRegisterPluginListener() override;

    int32_t PluginHardware(const std::string &dhId, const std::string &attr);
    int32_t UnPluginHardware(const std::string &dhId);

    std::shared_ptr<PluginListener> listener_ = nullptr;
    std::vector<std::shared_ptr<PluginListener>> listenerVec;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
