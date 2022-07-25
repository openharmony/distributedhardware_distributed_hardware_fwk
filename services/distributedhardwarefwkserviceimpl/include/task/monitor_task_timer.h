/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_HARDWARE_FWK_MONITOR_TASK_TIMER_H
#define DISTRIBUTED_HARDWARE_FWK_MONITOR_TASK_TIMER_H

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

#include "event_handler.h"

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class MonitorTaskTimer {
DECLARE_SINGLE_INSTANCE_BASE(MonitorTaskTimer);
public:
    ~MonitorTaskTimer();
    void StartTimer();
    void StopTimer();
    void StartEventRunner();

private:
    MonitorTaskTimer();
    void Execute(const std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler);
    void InitTimer();
    void ReleaseTimer();

private:
    std::thread eventHandlerThread_;
    std::mutex monitorTaskTimerMutex_;
    std::condition_variable monitorTaskTimerCond_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> eventHandler_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
