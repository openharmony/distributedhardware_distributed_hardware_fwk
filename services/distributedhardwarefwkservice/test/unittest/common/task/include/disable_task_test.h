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

#ifndef OHOS_DISTRIBUTED_DISABLE_TASK_TEST_H
#define OHOS_DISTRIBUTED_DISABLE_TASK_TEST_H

#include <gtest/gtest.h>
#include "task.h"
#include "disable_task.h"

#include "mock_component_manager.h"
#include "mock_dh_context.h"
#include "mock_dh_utils_tool.h"

namespace OHOS {
namespace DistributedHardware {
class DisableTaskTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    std::shared_ptr<MockComponentManager> componentManager_;
    std::shared_ptr<MockDHContext> dhContext_;
    std::shared_ptr<MockDhUtilTool> utilTool_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif