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

#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
    const std::string DH_FWK_HANDLER = "dh_fwk_handler";
}
#undef DH_LOG_TAG
#define DH_LOG_TAG "EventHandlerFactory"

IMPLEMENT_SINGLE_INSTANCE(EventHandlerFactory);
int32_t EventHandlerFactory::Init()
{
    DHLOGI("Init start!");
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    if (eventHandler_ != nullptr) {
        return DH_FWK_SUCCESS;
    }
    auto runner = AppExecFwk::EventRunner::Create(DH_FWK_HANDLER);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    return DH_FWK_SUCCESS;
}

int32_t EventHandlerFactory::UnInit()
{
    DHLOGI("UnInit start");
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveAllEvents();
        eventHandler_ = nullptr;
    }
    return DH_FWK_SUCCESS;
}

std::shared_ptr<AppExecFwk::EventHandler> EventHandlerFactory::GetEventHandler()
{
    std::lock_guard<std::mutex> lock(eventHandlerMutex_);
    return eventHandler_;
}
} // namespace DistributedHardware
} // namespace OHOS
