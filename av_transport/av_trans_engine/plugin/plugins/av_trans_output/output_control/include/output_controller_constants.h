/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_OUTPUT_CONTROLLER_CONSTANTS_H
#define OHOS_OUTPUT_CONTROLLER_CONSTANTS_H

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    NOTIFY_SUCCESS = 0,
    NOTIFY_FAILED = 1,
} ControllerListenerState;

typedef enum {
    HANDLE_SUCCESS = 0,
    HANDLE_FAILED = 1,
} ControllerHandlerState;

typedef enum {
    OUTPUT_FRAME = 0,
    DROP_FRAME = 1,
    REPEAT_FREAM = 2,
} ControllerControlResult;

const std::string LOOPER_CONTROL_THREAD_NAME = "looperControl";
const std::string OUTPUT_HANDLE_THREAD_NAME = "outputHandle";

const int64_t INVALID_TIMESTAMP = 0;
const int64_t INVALID_INTERVAL = 0;
const int64_t FACTOR_DOUBLE = 2;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_OUTPUT_CONTROLLER_CONSTANTS_H