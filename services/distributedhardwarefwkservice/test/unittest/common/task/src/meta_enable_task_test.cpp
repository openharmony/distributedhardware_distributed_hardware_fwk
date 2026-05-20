/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "meta_enable_task.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class MetaEnableTaskTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(MetaEnableTaskTest, test_ctor_sets_task_type, TestSize.Level1)
{
    auto task = std::make_shared<MetaEnableTask>("network_id", "uuid", "udid", "dh_id", DHType::CAMERA);
    EXPECT_EQ(task->GetTaskType(), TaskType::META_ENABLE);
}

HWTEST_F(MetaEnableTaskTest, test_ctor_sets_task_steps, TestSize.Level1)
{
    auto task = std::make_shared<MetaEnableTask>("network_id", "uuid", "udid", "dh_id", DHType::CAMERA);
    auto steps = task->GetTaskSteps();
    EXPECT_FALSE(steps.empty());
    EXPECT_EQ(steps[0], TaskStep::DO_MODEM_META_ENABLE);
}

HWTEST_F(MetaEnableTaskTest, test_do_task_basic, TestSize.Level1)
{
    auto task = std::make_shared<MetaEnableTask>("network_id", "uuid", "udid", "dh_id", DHType::CAMERA);
    EXPECT_NO_FATAL_FAILURE(task->DoTask());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}