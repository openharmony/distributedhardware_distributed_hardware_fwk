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

#include <algorithm>
#include <chrono>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

#include "access_manager.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_manager_factory.h"
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class AccessManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AccessManagerTest::SetUp() {}

void AccessManagerTest::TearDown() {}

void AccessManagerTest::SetUpTestCase() {}

void AccessManagerTest::TearDownTestCase() {}

/**
 * @tc.name: GetInstance_001
 * @tc.desc: Verify GetInstance success
 * @tc.type: FUNC
 * @tc.require: AR000GHSJK
 */
HWTEST_F(AccessManagerTest, GetInstance_001, TestSize.Level1)
{
    auto instance = AccessManager::GetInstance();
    ASSERT_TRUE(instance != nullptr);
}
}
}
