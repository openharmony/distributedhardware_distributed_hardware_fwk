/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
    GET_DISTRIBUTED_HARDWARE = 480013,
    REG_DH_SINK_STATUS_LISTNER = 480014,
    UNREG_DH_SINK_STATUS_LISTNER = 480015,
    REG_DH_SOURCE_STATUS_LISTNER = 480016,
    UNREG_DH_SOURCE_STATUS_LISTNER = 480017,
    ENABLE_SINK = 480018,
    DISABLE_SINK = 480019,
    ENABLE_SOURCE = 480020,
    DISABLE_SOURCE = 480021,
    LOAD_HDF = 480022,
    UNLOAD_HDF = 480023,
    NOTIFY_SOURCE_DEVICE_REMOTE_DMSDP_STARTED = 1,
    INIT_SINK_DMSDP = 0,
    NOTIFY_SINK_DEVICE_REMOTE_DMSDP_STARTED = 1212,
    REGISTER_HARDWARE_ACCESS_LISTENER = 480024,
    UNREGISTER_HARDWARE_ACCESS_LISTENER = 480025,
    SET_AUTHORIZATION_RESULT = 480026,
};
} // namespace DistributedHardware
} // namespace OHOS
#endif