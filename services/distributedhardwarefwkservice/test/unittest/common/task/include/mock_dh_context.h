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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_DH_CONTEXT_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_DH_CONTEXT_H

#include <string>
#include <gmock/gmock.h>

#include "dh_context.h"

namespace OHOS {
namespace DistributedHardware {
class IDHContext {
public:
    virtual size_t GetRealTimeOnlineDeviceCount() = 0;
    virtual uint32_t GetIsomerismConnectCount() = 0;
    static std::shared_ptr<IDHContext> GetOrCreateInstance();
    static void ReleaseInstance();
public:
    static std::shared_ptr<IDHContext> dhContext_;
};

class MockDHContext : public IDHContext {
public:
    virtual ~MockDHContext() = default;
    MOCK_METHOD(size_t, GetRealTimeOnlineDeviceCount, ());
    MOCK_METHOD(uint32_t, GetIsomerismConnectCount, ());
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_MOCK_COMPONENT_MANAGER_H
  