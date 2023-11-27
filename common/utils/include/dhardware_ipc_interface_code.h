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

#ifndef OHOS_DHARDWARE_IPC_INTERFACE_CODE_H
#define OHOS_DHARDWARE_IPC_INTERFACE_CODE_H

#include <cstdint>

/* SAID: 4801 */
namespace OHOS {
namespace DistributedHardware {
enum class DHMsgInterfaceCode : uint32_t {
    REG_PUBLISHER_LISTNER = 48001,
    UNREG_PUBLISHER_LISTENER = 48002,
    PUBLISH_MESSAGE = 48003,
    INIT_CTL_CEN = 48004,
    RELEASE_CTL_CEN = 48005,
    CREATE_CTL_CEN_CHANNEL = 48006,
    NOTIFY_AV_EVENT = 48007,
    REGISTER_CTL_CEN_CALLBACK = 48008,
    QUERY_LOCAL_SYS_SPEC = 48009,
    PAUSE_DISTRIBUTED_HARDWARE = 480010,
    RESUME_DISTRIBUTED_HARDWARE = 480011,
    STOP_DISTRIBUTED_HARDWARE = 480012,
    NOTIFY_SOURCE_DEVICE_REMOTE_DMSDP_STARTED = 1,
};
} // namespace DistributedHardware
} // namespace OHOS
#endif