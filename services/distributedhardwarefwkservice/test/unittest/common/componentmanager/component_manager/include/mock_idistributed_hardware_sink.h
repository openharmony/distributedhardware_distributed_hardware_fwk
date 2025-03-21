/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_IDISTRIBUTED_HARDWARE_SINK_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_IDISTRIBUTED_HARDWARE_SINK_H

#include <gmock/gmock.h>
#include <string>

#include "idistributed_hardware_sink.h"

namespace OHOS {
namespace DistributedHardware {
class MockIDistributedHardwareSink : public IDistributedHardwareSink {
public:
    virtual ~MockIDistributedHardwareSink() {}
    MOCK_METHOD(int32_t, InitSink, (const std::string &));
    MOCK_METHOD(int32_t, ReleaseSink, ());
    MOCK_METHOD(int32_t, SubscribeLocalHardware, (const std::string &, const std::string &));
    MOCK_METHOD(int32_t, UnsubscribeLocalHardware, (const std::string &));
    MOCK_METHOD(int32_t, RegisterPrivacyResources, (std::shared_ptr<PrivacyResourcesListener>));
    MOCK_METHOD(int32_t, PauseDistributedHardware, (const std::string &));
    MOCK_METHOD(int32_t, ResumeDistributedHardware, (const std::string &));
    MOCK_METHOD(int32_t, StopDistributedHardware, (const std::string &));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
