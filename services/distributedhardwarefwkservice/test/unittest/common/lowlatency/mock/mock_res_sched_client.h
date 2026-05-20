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

#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_RES_SCHED_CLIENT_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_RES_SCHED_CLIENT_H

#include "res_sched_client.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace ResourceSchedule {
class MockResSchedClient {
public:
    static MockResSchedClient& GetInstance()
    {
        static MockResSchedClient instance;
        return instance;
    }
    MOCK_METHOD3(ReportData, void(uint32_t, int32_t,
        const std::unordered_map<std::string, std::string>&));
};
}
}
#endif