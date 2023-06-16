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
    REG_PUBLISHER_LISTNER = 1,
    UNREG_PUBLISHER_LISTENER = 2,
    PUBLISH_MESSAGE = 3,
    INIT_CTL_CEN = 4,
    RELEASE_CTL_CEN = 5,
    CREATE_CTL_CEN_CHANNEL = 6,
    NOTIFY_AV_EVENT = 7,
    REGISTER_CTL_CEN_CALLBACK = 8
};
} // namespace DistributedHardware
} // namespace OHOS
#endif