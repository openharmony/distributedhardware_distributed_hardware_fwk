/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_ENABLE_TASK_H
#define OHOS_DISTRIBUTED_HARDWARE_ENABLE_TASK_H

#include "task.h"

namespace OHOS {
namespace DistributedHardware {
class EnableTask : public Task {
public:
    EnableTask() = delete;
    EnableTask(const std::string &networkId, const std::string &uuid, const std::string &udid,
        const std::string &dhId, const DHType dhType);
    virtual ~EnableTask();
    virtual void DoTask();

    void SetEffectSink(bool isEffect);
    bool GetEffectSink();
    void SetEffectSource(bool isEffect);
    bool GetEffectSource();
    void SetCallingUid(int32_t callingUid);
    int32_t GetCallingUid();
    void SetCallingPid(int32_t callingPid);
    int32_t GetCallingPid();

private:
    /* synchronous function for register distributed hardware, return on asynchronous register finish */
    int32_t RegisterHardware();
    void DoTaskInner();
    int32_t DoAutoEnable();
    int32_t DoActiveEnable();

private:
    // effect sink
    bool effectSink_{ false };
    // effect source
    bool effectSource_{ false };
    // enable or disable calling uid
    int32_t callingUid_{ 0 };
    // enable or disable calling pid
    int32_t callingPid_{ 0 };
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
