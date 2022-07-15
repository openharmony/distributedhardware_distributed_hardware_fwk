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

#ifndef OHOS_DISTRIBUTED_HARDWARE_EVENT_BUS_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_EVENT_BUS_TEST_H

#include <cstdint>
#include <gtest/gtest_pred_impl.h>
#include <string>

#include "event.h"
#include "event_sender.h"
#include "eventbus_handler.h"

namespace OHOS {
namespace DistributedHardware {
class FakeObject {
public:
    explicit FakeObject(std::string name) : name_(name), age_(0) {}
    virtual ~FakeObject() {}
    void SetAge(int32_t age)
    {
        age_ = age;
    }
    int32_t GetAge() const
    {
        return age_;
    }

private:
    std::string name_;
    int32_t age_;
};

class FakeEvent : public OHOS::DistributedHardware::Event {
    TYPEINDENT(FakeEvent)
public:
    FakeEvent(OHOS::DistributedHardware::EventSender &sender, FakeObject &object)
        : Event(sender), object_(object), action_(0)
    {}
    FakeEvent(OHOS::DistributedHardware::EventSender &sender, FakeObject &object, int32_t action)
        : Event(sender), object_(object), action_(action)
    {}
    virtual ~FakeEvent() {}

    FakeObject *GetObject() const
    {
        return &object_;
    }
    int32_t GetAction() const
    {
        return action_;
    }

private:
    FakeObject &object_;
    int32_t action_;
};

class FakeListener : public OHOS::DistributedHardware::EventBusHandler<FakeEvent> {
public:
    FakeListener() {}
    virtual ~FakeListener() {}

    virtual void OnEvent(FakeEvent &ev)
    {
        FakeObject *obj = ev.GetObject();
        obj->SetAge(ev.GetAction());
    }
};

class FakeSender : public OHOS::DistributedHardware::EventSender {
public:
    FakeSender() {}
    virtual ~FakeSender() {}
};

class EventbusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
