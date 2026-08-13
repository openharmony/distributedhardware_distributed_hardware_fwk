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

#include "mock_dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<IDhUtilTool> IDhUtilTool::UtilToolInstance_;

std::shared_ptr<IDhUtilTool> IDhUtilTool::GetOrCreateInstance()
{
    if (!UtilToolInstance_) {
        UtilToolInstance_ = std::make_shared<MockDhUtilTool>();
    }
    return UtilToolInstance_;
}

void IDhUtilTool::ReleaseInstance()
{
    UtilToolInstance_.reset();
    UtilToolInstance_ = nullptr;
}

std::string GetLocalUdid()
{
    return IDhUtilTool::GetOrCreateInstance()->GetLocalUdid();
}

DeviceInfo GetLocalDeviceInfo()
{
    return IDhUtilTool::GetOrCreateInstance()->GetLocalDeviceInfo();
}
} // namespace DistributedHardware
} // namespace OHOS
