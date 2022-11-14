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

#ifndef OHOS_DISTRIBUTED_HARDWARE_ONLINE_TASK_H
#define OHOS_DISTRIBUTED_HARDWARE_ONLINE_TASK_H

#include "task.h"

namespace OHOS {
namespace DistributedHardware {
class OnLineTask : public Task {
public:
    OnLineTask() = delete;
    OnLineTask(const std::string &networkId, const std::string &uuid, const std::string &dhId, const DHType dhType);
    virtual ~OnLineTask();
    virtual void DoTask();

private:
    void DoSyncInfo();
    void CreateEnableTask();
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
