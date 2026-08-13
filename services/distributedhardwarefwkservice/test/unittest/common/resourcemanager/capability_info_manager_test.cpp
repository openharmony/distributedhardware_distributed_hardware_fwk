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

#include "capability_info_manager.h"
#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class CapabilityInfoManagerTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(CapabilityInfoManagerTest, test_get_instance_singleton, TestSize.Level1)
{
    auto instance1 = CapabilityInfoManager::GetInstance();
    auto instance2 = CapabilityInfoManager::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

HWTEST_F(CapabilityInfoManagerTest, test_get_event_handler_null, TestSize.Level1)
{
    auto instance = CapabilityInfoManager::GetInstance();
    auto handler = instance->GetEventHandler();
    EXPECT_EQ(handler, nullptr);
}