/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_MOCK_IDISTRIBUTED_HARDWARE_SOURCE_H
#define OHOS_MOCK_IDISTRIBUTED_HARDWARE_SOURCE_H

#include <string>

#include "gmock/gmock.h"

#include "idistributed_hardware_source.h"
namespace OHOS {
namespace DistributedHardware {
class MockIDistributedHardwareSource : public IDistributedHardwareSource {
public:
    virtual ~MockIDistributedHardwareSource() {}
    MOCK_METHOD1(InitSource, int32_t(const std::string &params));
    MOCK_METHOD0(ReleaseSource, int32_t());
    MOCK_METHOD4(RegisterDistributedHardware, int32_t(const std::string &uuid, const std::string &dhId,
        const EnableParam &parameters, std::shared_ptr<RegisterCallback> callback));
    MOCK_METHOD3(UnregisterDistributedHardware,
        int32_t(const std::string &uuid, const std::string &dhId, std::shared_ptr<UnregisterCallback> callback));

    int32_t ConfigDistributedHardware(const std::string &uuid, const std::string &dhId, const std::string &key,
        const std::string &value)
    {
        return 0;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
