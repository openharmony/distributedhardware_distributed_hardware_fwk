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

#ifndef OHOS_DISTRIBUTED_HARDWARE_OFFLINE_TASK_H
#define OHOS_DISTRIBUTED_HARDWARE_OFFLINE_TASK_H

#include <condition_variable>
#include <mutex>

#include "task.h"

namespace OHOS {
namespace DistributedHardware {
class OffLineTask : public Task {
public:
    OffLineTask() = delete;
    OffLineTask(const std::string &networkId, const std::string &uuid, const std::string &dhId);
    virtual ~OffLineTask();
    virtual void DoTask();

    virtual void NotifyFatherFinish(std::string taskId);
    virtual void AddChildrenTask(std::shared_ptr<Task> childrenTask);

private:
    /* OffLineTask should wait until all Disable task finish, we run it in a independent thread */
    void DoTaskInner();
    /* create disable tasks for off line device */
    void CreateDisableTask();
    /* wait until all disable tasks finish or timeout */
    void WaitDisableTaskFinish();
    /* clear off line device info */
    void ClearOffLineInfo();

private:
    /* condition for children task finish */
    std::condition_variable finishCondVar_;
    std::mutex unFinishTaskMtx_;
    std::set<std::string> unFinishChildrenTasks_;
};
}
}
#endif
