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

#include "event_handler_factory.h"
#include <gtest/gtest.h>
#include "distributed_hardware_errno.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class EventHandlerFactoryTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(EventHandlerFactoryTest, test_init_success, TestSize.Level1)
{
    auto& factory = EventHandlerFactory::GetInstance();
    int32_t ret = factory.Init();
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    auto handler = factory.GetEventHandler();
    EXPECT_NE(handler, nullptr);
    factory.UnInit();
}

HWTEST_F(EventHandlerFactoryTest, test_init_twice, TestSize.Level1)
{
    auto& factory = EventHandlerFactory::GetInstance();
    int32_t ret1 = factory.Init();
    int32_t ret2 = factory.Init();
    EXPECT_EQ(ret1, DH_FWK_SUCCESS);
    EXPECT_EQ(ret2, DH_FWK_SUCCESS);
    factory.UnInit();
}

HWTEST_F(EventHandlerFactoryTest, test_uninit_success, TestSize.Level1)
{
    auto& factory = EventHandlerFactory::GetInstance();
    factory.Init();
    int32_t ret = factory.UnInit();
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    auto handler = factory.GetEventHandler();
    EXPECT_EQ(handler, nullptr);
}

HWTEST_F(EventHandlerFactoryTest, test_get_event_handler_before_init, TestSize.Level1)
{
    auto& factory = EventHandlerFactory::GetInstance();
    factory.UnInit();
    auto handler = factory.GetEventHandler();
    EXPECT_EQ(handler, nullptr);
}

HWTEST_F(EventHandlerFactoryTest, test_singleton_instance, TestSize.Level1)
{
    auto& factory1 = EventHandlerFactory::GetInstance();
    auto& factory2 = EventHandlerFactory::GetInstance();
    EXPECT_EQ(&factory1, &factory2);
}