/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_HARDWARE_HANDLER_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_HARDWARE_HANDLER_H

#include <gmock/gmock.h>

#include "ihardware_handler.h"

namespace OHOS {
namespace DistributedHardware {
class MockHardwareHandler : public IHardwareHandler {
public:
    virtual ~MockHardwareHandler() = default;

    MOCK_METHOD(int32_t, Initialize, ());
    MOCK_METHOD((std::vector<DHItem>), QueryMeta, ());
    MOCK_METHOD((std::vector<DHItem>), Query, ());
    MOCK_METHOD((std::map<std::string, std::string>), QueryExtraInfo, ());
    MOCK_METHOD(bool, IsSupportPlugin, ());
    MOCK_METHOD(void, RegisterPluginListener, (std::shared_ptr<PluginListener>));
    MOCK_METHOD(void, UnRegisterPluginListener, ());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_HARDWARE_HANDLER_H
