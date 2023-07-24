/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_AV_TRANSPORT_ERRNO_H
#define OHOS_AV_TRANSPORT_ERRNO_H

#include <cstdint>

namespace OHOS {
namespace DistributedHardware {
    /*
     * The av transport module define errno, range: [-80000; -89999]
     * Here's external errno, range: [-80000; -80199]
     */
    constexpr int32_t DH_AVT_SUCCESS = 0;
    constexpr int32_t ERR_DH_AVT_INVALID_PARAM = -80001;
    constexpr int32_t ERR_DH_AVT_INIT_FAILED = -80002;
    constexpr int32_t ERR_DH_AVT_START_FAILED = -80003;
    constexpr int32_t ERR_DH_AVT_STOP_FAILED = -80004;
    constexpr int32_t ERR_DH_AVT_RELEASE_FAILED = -80005;
    constexpr int32_t ERR_DH_AVT_SETUP_FAILED = -80006;
    constexpr int32_t ERR_DH_AVT_PUSH_DATA_FAILED = -80007;
    constexpr int32_t ERR_DH_AVT_PULL_DATA_FAILED = -80008;
    constexpr int32_t ERR_DH_AVT_SEND_MESSAGE_FAILED = -80009;
    constexpr int32_t ERR_DH_AVT_CREATE_CHANNEL_FAILED = -80010;
    constexpr int32_t ERR_DH_AVT_CHANNEL_ALREADY_CREATED = -80011;
    constexpr int32_t ERR_DH_AVT_UNIMPLEMENTED = -80012;
    constexpr int32_t ERR_DH_AVT_COMMON_FAILED = -80013;
    constexpr int32_t ERR_DH_AVT_PAUSE_FAILED = -80014;
    constexpr int32_t ERR_DH_AVT_RESUME_FAILED = -80015;

    /* Here's internal errno, range: [-81000; -89999] */
    constexpr int32_t ERR_DH_AVT_INVALID_PARAM_VALUE = -81000;
    constexpr int32_t ERR_DH_AVT_INVALID_PARAM_TYPE = -81001;
    constexpr int32_t ERR_DH_AVT_INVALID_OPERATION = -81002;
    constexpr int32_t ERR_DH_AVT_UNSUPPORTED_FORMAT = -81003;
    constexpr int32_t ERR_DH_AVT_NOT_EXISTED = -81004;
    constexpr int32_t ERR_DH_AVT_TIMED_OUT = -81005;
    constexpr int32_t ERR_DH_AVT_NO_MEMORY = -81006;
    constexpr int32_t ERR_DH_AVT_INVALID_STATE = -81007;
    constexpr int32_t ERR_DH_AVT_PERMISSION_DENIED = -81008;
    constexpr int32_t ERR_DH_AVT_NO_NOTIFY = -81009;
    constexpr int32_t ERR_DH_AVT_NULL_POINTER = -81010;
    constexpr int32_t ERR_DH_AVT_SESSION_ERROR = -81011;
    constexpr int32_t ERR_DH_AVT_SEND_DATA_FAILED = -81012;
    constexpr int32_t ERR_DH_AVT_PREPARE_FAILED = -81013;
    constexpr int32_t ERR_DH_AVT_SET_PARAM_FAILED = -81014;
    constexpr int32_t ERR_DH_AVT_OUTPUT_DATA_FAILED = -80015;
    constexpr int32_t ERR_DH_AVT_TIME_SYNC_FAILED = -80016;
    constexpr int32_t ERR_DH_AVT_SERVICE_REMOTE_IS_NULL = -80017;
    constexpr int32_t ERR_DH_AVT_SERVICE_WRITE_TOKEN_FAIL = -80018;
    constexpr int32_t ERR_DH_AVT_SERVICE_IPC_SEND_REQUEST_FAIL = -80019;
    constexpr int32_t ERR_DH_AVT_SERVICE_WRITE_INFO_FAIL = -80019;
    constexpr int32_t ERR_DH_AVT_CTRL_CENTER_INIT_FAIL = -80020;
    constexpr int32_t ERR_DH_AVT_REGISTER_CALLBACK_FAIL = -80021;
    constexpr int32_t ERR_DH_AVT_FWK_INNER_KIT_NULL = -80022;
    constexpr int32_t ERR_DH_AVT_SHARED_MEMORY_FAILED = -80023;
    constexpr int32_t ERR_DH_AVT_MASTER_NOT_READY = -80024;
    constexpr int32_t ERR_DH_AVT_SESSION_HAS_OPENED = -80025;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_AV_TRANSPORT_ERRNO_H