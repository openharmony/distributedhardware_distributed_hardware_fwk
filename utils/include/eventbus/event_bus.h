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

#ifndef OHOS_DISTRIBUTED_HARDWARE_EVENT_BUS_H
#define OHOS_DISTRIBUTED_HARDWARE_EVENT_BUS_H

#include <memory>
#include <set>
#include <unordered_map>
#include <mutex>

#include "event_handler.h"

#include "dh_log.h"
#include "anonymous_string.h"
#include "event.h"
#include "eventbus_handler.h"
#include "event_registration.h"

#ifndef DH_LOG_TAG
#define DH_LOG_TAG "DHEventBus"
#endif

namespace OHOS {
namespace DistributedHardware {
enum POSTMODE : uint32_t {
    POST_ASYNC = 0,
    POST_SYNC,
};

class EventBus final {
public:
    EventBus()
    {
        ULOGI("ctor EventBus");
        if (!eventbusHandler_) {
            auto busRunner = OHOS::AppExecFwk::EventRunner::Create("DHEventbusHandler");
            eventbusHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(busRunner);
        }
    }
    ~EventBus()
    {
        ULOGI("dtor EventBus");
    }

    template<class T>
    std::shared_ptr<EventRegistration> AddHandler(const std::string &typeId,
        DistributedHardware::EventBusHandler<T> &handler)
    {
        std::lock_guard<std::mutex> lock(handlerMtx);
        Registrations *registrations = handlers[typeId];

        if (registrations == nullptr) {
            registrations = new EventRegistration::Registrations();
            handlers[typeId] = registrations;
        }

        for (auto &reg : *registrations) {
            if (reg->GetHandler() == static_cast<void *>(&handler) && reg->GetSender() == nullptr) {
                return reg;
            }
        }

        std::shared_ptr<EventRegistration> registration =
            std::make_shared<EventRegistration>(static_cast<void *>(&handler), nullptr);
        registrations->insert(registration);

        return registration;
    }

    template<class T>
    std::shared_ptr<EventRegistration> AddHandler(const std::string &typeId, EventBusHandler<T> &handler,
        EventSender &sender)
    {
        std::lock_guard<std::mutex> lock(handlerMtx);
        Registrations *registrations = handlers[typeId];

        if (registrations == nullptr) {
            registrations = new EventRegistration::Registrations();
            handlers[typeId] = registrations;
        }

        for (auto &reg : *registrations) {
            if (reg->GetHandler() == static_cast<void *>(&handler) && reg->GetSender() == &sender) {
                return reg;
            }
        }

        std::shared_ptr<EventRegistration> registration =
            std::make_shared<EventRegistration>(static_cast<void *>(&handler), &sender);
        registrations->insert(registration);

        return registration;
    }

    template<class T>
    bool RemoveHandler(const std::string &typeId, std::shared_ptr<EventRegistration> &EvenReg)
    {
        std::lock_guard<std::mutex> lock(handlerMtx);
        Registrations *registrations = handlers[typeId];
        if (registrations == nullptr) {
            return false;
        }

        bool ret = false;
        auto regIter = registrations->find(EvenReg);
        if (regIter != registrations->end()) {
            registrations->erase(regIter);
            ret = true;
        }

        return ret;
    }

    template<class T>
    void PostEvent(T &e, POSTMODE mode = POSTMODE::POST_ASYNC)
    {
        if (mode == POSTMODE::POST_SYNC) {
            PostEventInner(e);
        } else {
            auto eventFunc = [this, e]() mutable {
                PostEventInner(e);
            };
            if (!(eventbusHandler_ && eventbusHandler_->PostTask(eventFunc))) {
                ULOGE("Eventbus::PostEvent Async PostTask fail");
            }
        }
    }

    template<class T>
    void PostEvent(T &e, int64_t delayTime)
    {
        auto eventFunc = [this, e]() mutable {
            PostEventInner(e);
        };
        if (!(eventbusHandler_ && eventbusHandler_->PostTask(eventFunc, e->getType(), delayTime))) {
            ULOGE("Eventbus::PostEvent Async PostTask fail");
        }
    }

    template<class T>
    void RemoveEvent(T &e)
    {
        if (!(eventbusHandler_ && eventbusHandler_->RemoveTask(e->getType()))) {
            ULOGE("Eventbus::RemoveEvent fail");
        }
    }

    void PostTask(const OHOS::AppExecFwk::InnerEvent::Callback& callback,
        const std::string& name,
        int64_t delayTimeInMs = 0)
    {
        ULOGI("Eventbus::PostTask Async PostTask, taskName:%{public}s.", GetAnonyString(name).c_str());
        if (eventbusHandler_ != nullptr) {
            eventbusHandler_->PostTask(callback, name, delayTimeInMs);
        }
    }

    void RemoveTask(const std::string& name)
    {
        ULOGI("Eventbus::RemoveTask, taskName:%{public}s.", GetAnonyString(name).c_str());
        if (eventbusHandler_ != nullptr) {
            eventbusHandler_->RemoveTask(name);
        }
    }

private:
    template<class T>
    void PostEventInner(T &e)
    {
        std::lock_guard<std::mutex> lock(handlerMtx);
        Registrations *registrations = handlers[e.GetType()];
        if (registrations == nullptr) {
            return;
        }

        for (auto &reg : *registrations) {
            if ((reg->GetSender() == nullptr) || (reg->GetSender() == &e.GetSender())) {
                static_cast<EventBusHandler<Event> *>(const_cast<void *>(reg->GetHandler()))->Dispatch(e);
            }
        }
    }

private:
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventbusHandler_;

    using Registrations = std::set<std::shared_ptr<EventRegistration>>;
    std::mutex handlerMtx;
    using TypeMap = std::unordered_map<std::string, std::set<std::shared_ptr<EventRegistration>> *>;
    TypeMap handlers;
};
}
}
#endif