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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_DH_UTILS_TOOL_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_DH_UTILS_TOOL_H

#include <gmock/gmock.h>

#include "dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
class IDhUtilTool {
public:
    virtual ~IDhUtilTool() = default;

    virtual std::string GetLocalUdid() = 0;
    virtual DeviceInfo GetLocalDeviceInfo() = 0;
    static std::shared_ptr<IDhUtilTool> GetOrCreateInstance();
    static void ReleaseInstance();
private:
    static std::shared_ptr<IDhUtilTool> UtilToolInstance_;
};

class MockDhUtilTool : public IDhUtilTool {
public:
    MOCK_METHOD(std::string, GetLocalUdid, ());
    MOCK_METHOD(DeviceInfo, GetLocalDeviceInfo, ());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_DH_UTILS_TOOL_H
