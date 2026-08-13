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

#include "offline_task.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class OffLineTaskTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(OffLineTaskTest, test_ctor_sets_task_type, TestSize.Level1)
{
    auto task = std::make_shared<OffLineTask>("network_id", "uuid", "udid", "dh_id", DHType::CAMERA);
    EXPECT_EQ(task->GetTaskType(), TaskType::OFF_LINE);
}

HWTEST_F(OffLineTaskTest, test_ctor_sets_task_steps, TestSize.Level1)
{
    auto task = std::make_shared<OffLineTask>("network_id", "uuid", "udid", "dh_id", DHType::CAMERA);
    auto steps = task->GetTaskSteps();
    EXPECT_EQ(steps.size(), 5);
}

HWTEST_F(OffLineTaskTest, test_do_task_basic, TestSize.Level1)
{
    auto task = std::make_shared<OffLineTask>("network_id", "uuid", "udid", "dh_id", DHType::CAMERA);
    EXPECT_NO_FATAL_FAILURE(task->DoTask());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

HWTEST_F(OffLineTaskTest, test_notify_father_finish, TestSize.Level1)
{
    auto task = std::make_shared<OffLineTask>("network_id", "uuid", "udid", "dh_id", DHType::CAMERA);
    EXPECT_NO_FATAL_FAILURE(task->NotifyFatherFinish("test_task_id"));
}

HWTEST_F(OffLineTaskTest, test_add_children_task, TestSize.Level1)
{
    auto task = std::make_shared<OffLineTask>("network_id", "uuid",
        "udid", "dh_id", DHType::CAMERA);
    auto childTask = std::make_shared<OffLineTask>("child_network_id",
        "child_uuid", "child_udid", "child_dh_id", DHType::CAMERA);
    EXPECT_NO_FATAL_FAILURE(task->AddChildrenTask(childTask));
}