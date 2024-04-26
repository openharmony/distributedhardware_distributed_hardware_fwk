/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "dh_timer.h"

#include <pthread.h>

#include "anonymous_string.h"
#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DHTimer"

DHTimer::DHTimer(std::string timerId, int32_t delayTimeMs) : timerId_(timerId), delayTimeMs_(delayTimeMs)
{
    DHLOGI("DHTimer ctor!");
}

DHTimer::~DHTimer()
{
    DHLOGI("DHTimer dtor!");
    ReleaseTimer();
}

void DHTimer::InitTimer()
{
    DHLOGI("start");
    std::unique_lock<std::mutex> lock(timerMutex_);
    if (eventHandler_ == nullptr) {
        eventHandlerThread_ = std::thread(&DHTimer::StartEventRunner, this);
        timerCond_.wait(lock, [this] { return eventHandler_ != nullptr; });
    }
    DHLOGI("end");
}

void DHTimer::ReleaseTimer()
{
    DHLOGI("start");
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(timerId_);
        if (eventHandler_->GetEventRunner() != nullptr) {
            eventHandler_->GetEventRunner()->Stop();
        }
    }
    if (eventHandlerThread_.joinable()) {
        eventHandlerThread_.join();
    }
    eventHandler_ = nullptr;
    DHLOGI("end");
}

void DHTimer::StartEventRunner()
{
    DHLOGI("start");
    int32_t ret = pthread_setname_np(pthread_self(), EVENT_RUN);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("StartEventRunner setname failed.");
    }
    auto busRunner = AppExecFwk::EventRunner::Create(false);
    if (busRunner == nullptr) {
        DHLOGE("busRunner is nullptr!");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(timerMutex_);
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(busRunner);
    }
    timerCond_.notify_all();
    busRunner->Run();
    DHLOGI("end");
}

void DHTimer::StartTimer()
{
    DHLOGI("start");
    InitTimer();
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (eventHandler_ == nullptr) {
        DHLOGE("eventHandler is nullptr");
        return;
    }
    auto executeFunc = [this] { Execute(); };
    eventHandler_->PostTask(executeFunc, timerId_, delayTimeMs_);
}

void DHTimer::StopTimer()
{
    DHLOGI("start");
    ReleaseTimer();
    HandleStopTimer();
    DHLOGI("end");
}

void DHTimer::Execute()
{
    DHLOGI("start");
    std::lock_guard<std::mutex> lock(timerMutex_);
    if (eventHandler_ == nullptr) {
        DHLOGE("eventHandler is nullptr!");
        return;
    }
    ExecuteInner();
    auto executeInnerFunc = [this] { Execute(); };
    eventHandler_->PostTask(executeInnerFunc, timerId_, delayTimeMs_);
}
} // namespace DistributedHardware
} // namespace OHOS
