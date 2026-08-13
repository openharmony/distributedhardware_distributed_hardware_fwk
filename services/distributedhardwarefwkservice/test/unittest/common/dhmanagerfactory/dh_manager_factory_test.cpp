/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"

#include "distributed_hardware_errno.h"
#include "distributed_hardware_manager_factory.h"
#include "event_handler_factory.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace DistributedHardware {
class DhManagerFactoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DhManagerFactoryTest::SetUp() {}

void DhManagerFactoryTest::TearDown() {}

void DhManagerFactoryTest::SetUpTestCase() {}

void DhManagerFactoryTest::TearDownTestCase() {}

HWTEST_F(DhManagerFactoryTest, CreateSaStatusHandler_001, TestSize.Level1)
{
    DistributedHardwareManagerFactory::GetInstance().saStatusHandler_ = nullptr;
    auto ret = DistributedHardwareManagerFactory::GetInstance().CreateSaStatusHandler();
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);

    EventHandlerFactory::GetInstance().Init();
    ret = DistributedHardwareManagerFactory::GetInstance().CreateSaStatusHandler();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EventHandlerFactory::GetInstance().Init();
    DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();
}

HWTEST_F(DhManagerFactoryTest, DestroySaStatusHandler_001, TestSize.Level1)
{
    DistributedHardwareManagerFactory::GetInstance().saStatusHandler_ = nullptr;
    auto ret = DistributedHardwareManagerFactory::GetInstance().DestroySaStatusHandler();
    EXPECT_EQ(ERR_DH_FWK_POINTER_IS_NULL, ret);

    DistributedHardwareManagerFactory::GetInstance().DelaySaStatusTask();

    DistributedHardwareManagerFactory::GetInstance().CreateSaStatusHandler();
    ret = DistributedHardwareManagerFactory::GetInstance().DestroySaStatusHandler();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    EventHandlerFactory::GetInstance().UnInit();
}

HWTEST_F(DhManagerFactoryTest, Init_001, TestSize.Level1)
{
    auto ret = DistributedHardwareManagerFactory::GetInstance().Init();
    EXPECT_EQ(true, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
