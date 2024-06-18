/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_IMPL_UTILS_H
#define OHOS_DISTRIBUTED_HARDWARE_IMPL_UTILS_H

#include <unordered_map>

#include "device_type.h"
#include "constants.h"

namespace OHOS {
namespace DistributedHardware {
enum class TaskType : int32_t {
    UNKNOWN = 0,
    ENABLE = 1,
    DISABLE = 2,
    ON_LINE = 3,
    OFF_LINE = 4
};

enum class TaskStep : int32_t {
    DO_ENABLE = 1,
    DO_DISABLE = 2,
    SYNC_ONLINE_INFO = 3,
    REGISTER_ONLINE_DISTRIBUTED_HARDWARE = 4,
    UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE = 5,
    CLEAR_OFFLINE_INFO = 6,
    WAIT_UNREGISTGER_COMPLETE = 7
};

enum class TaskState : int32_t {
    INIT = 0,
    RUNNING = 1,
    SUCCESS = 2,
    FAIL = 3
};

struct CompVersion {
    std::string name;
    DHType dhType;
    std::string handlerVersion;
    std::string sourceVersion;
    std::string sinkVersion;
};

struct DHVersion {
    std::string uuid;
    std::string dhVersion;
    std::unordered_map<DHType, CompVersion> compVersions;
};

struct TaskParam {
    // remote device networkid
    std::string networkId;
    // remote device uuid
    std::string uuid;
    // remote device udid
    std::string udid;
    // remote device dhid
    std::string dhId;
    // remote device dh type
    DHType dhType;
};

struct TaskDump {
    std::string id;
    TaskType taskType;
    TaskParam taskParm;
    TaskState taskState;
    std::vector<TaskStep> taskSteps;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
