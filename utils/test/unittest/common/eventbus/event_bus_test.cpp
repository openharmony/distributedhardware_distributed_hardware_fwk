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

#include "event_bus_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    FakeObject *g_obj = nullptr;
    FakeSender *g_sender = nullptr;
    FakeListener *g_listener = nullptr;
    std::shared_ptr<OHOS::DistributedHardware::EventRegistration> g_regHandler = nullptr;
    EventBus* g_eventBus = nullptr;
}

void EventbusTest::SetUpTestCase(void)
{
}

void EventbusTest::TearDownTestCase(void)
{
}

void EventbusTest::SetUp()
{
    g_obj = new FakeObject("Player");
    g_listener = new FakeListener();
    g_sender = new FakeSender();
    g_regHandler = nullptr;
    g_eventBus = new EventBus();
}

void EventbusTest::TearDown()
{
    if (g_obj != nullptr) {
        delete g_obj;
        g_obj = nullptr;
    }
    if (g_listener != nullptr) {
        delete g_listener;
        g_listener = nullptr;
    }
    if (g_sender != nullptr) {
        delete g_sender;
        g_sender = nullptr;
    }
    g_regHandler = nullptr;

    if (g_eventBus != nullptr) {
        delete g_eventBus;
        g_eventBus = nullptr;
    }
}

/**
 * @tc.name: event_bus_test_001
 * @tc.desc: Verify the EventBus AddHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(EventbusTest, event_bus_test_001, TestSize.Level0)
{
    EXPECT_EQ(g_regHandler, nullptr);

    FakeEvent e(*g_sender, *g_obj);
    g_regHandler = g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener);

    EXPECT_NE(g_regHandler, nullptr);
    EXPECT_EQ(g_regHandler->GetSender(), nullptr);
    EXPECT_EQ(g_regHandler->GetHandler(), (void *)g_listener);
}

/**
 * @tc.name: event_bus_test_002
 * @tc.desc: Verify the EventBus AddHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(EventbusTest, event_bus_test_002, TestSize.Level0)
{
    EXPECT_EQ(g_regHandler, nullptr);

    FakeEvent e(*g_sender, *g_obj);
    g_regHandler = g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener);
    std::shared_ptr<EventRegistration> secondHandler = g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener);

    EXPECT_NE(secondHandler, nullptr);
    EXPECT_EQ(g_regHandler, secondHandler);
}

/**
 * @tc.name: event_bus_test_003
 * @tc.desc: Verify the EventBus AddHandler with sender function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(EventbusTest, event_bus_test_003, TestSize.Level0)
{
    EXPECT_EQ(g_regHandler, nullptr);

    FakeEvent e(*g_sender, *g_obj);
    g_regHandler = g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener);
    std::shared_ptr<EventRegistration> secondHandler =
        g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener, *g_sender);

    EXPECT_NE(g_regHandler, nullptr);
    EXPECT_NE(secondHandler, nullptr);
    EXPECT_NE(g_regHandler, secondHandler);
    EXPECT_EQ(secondHandler->GetSender(), g_sender);
    EXPECT_EQ(secondHandler->GetHandler(), (void *)g_listener);
}

/**
 * @tc.name: event_bus_test_004
 * @tc.desc: Verify the EventBus AddHandler and RemoveHandler function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(EventbusTest, event_bus_test_004, TestSize.Level0)
{
    EXPECT_EQ(g_regHandler, nullptr);
    FakeEvent e(*g_sender, *g_obj);
    EXPECT_EQ(false, g_eventBus->RemoveHandler<FakeEvent>(e.GetType(), g_regHandler));

    g_regHandler = g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener);
    EXPECT_EQ(true, g_eventBus->RemoveHandler<FakeEvent>(e.GetType(), g_regHandler));
}

/**
 * @tc.name: event_bus_test_005
 * @tc.desc: Verify the EventBus PostEvent function.
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(EventbusTest, event_bus_test_005, TestSize.Level0)
{
    EXPECT_EQ(0, g_obj->GetAge());

    FakeEvent e(*g_sender, *g_obj, 10);
    g_regHandler = g_eventBus->AddHandler<FakeEvent>(e.GetType(), *g_listener);
    g_eventBus->PostEvent<FakeEvent>(e, POSTMODE::POST_SYNC);
    EXPECT_EQ(10, g_obj->GetAge());
}
}
}