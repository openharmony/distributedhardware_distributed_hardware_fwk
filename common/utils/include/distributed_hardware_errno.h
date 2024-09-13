/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_ERRNO
#define OHOS_DISTRIBUTED_HARDWARE_ERRNO

#include <cstdint>

namespace OHOS {
namespace DistributedHardware {
    /*
     * The distributed hardware module define errno, range: [-10000, -19999]
     * Here's common errno, range: [-10000, -10199]
     */
    constexpr int32_t DH_FWK_SUCCESS = 0;
    constexpr int32_t ERR_DH_FWK_PARA_INVALID = -10000;
    constexpr int32_t ERR_DH_FWK_REMOTE_NETWORK_ID_IS_EMPTY = -10001;
    constexpr int32_t ERR_DH_FWK_REMOTE_DEVICE_ID_IS_EMPTY = -10002;
    constexpr int32_t ERR_DH_FWK_TYPE_NOT_EXIST = -10003;
    constexpr int32_t ERR_DH_FWK_JSON_PARSE_FAILED = -10004;
    constexpr int32_t ERR_DH_FWK_POINTER_IS_NULL = -10005;

    /* VersionManager errno, range: [-10200, -10299] */
    constexpr int32_t ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST = -10200;

    /* ComponentManager errno, range: [-10302, -10399] */
    constexpr int32_t ERR_DH_FWK_COMPONENT_ENABLE_FAILED = -10302;
    constexpr int32_t ERR_DH_FWK_COMPONENT_DISABLE_FAILED = -10303;
    constexpr int32_t ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT = -10304;
    constexpr int32_t ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT = -10305;
    constexpr int32_t ERR_DH_FWK_COMPONENT_REGISTER_FAILED = -10306;
    constexpr int32_t ERR_DH_FWK_COMPONENT_UNREGISTER_FAILED = -10007;
    constexpr int32_t ERR_DH_FWK_COMPONENT_GET_REMOTE_SA_FAILED = -10008;
    constexpr int32_t ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED = -10009;
    constexpr int32_t ERR_DH_FWK_COMPONENT_DHTYPE_NOT_FOUND = -10010;
    constexpr int32_t ERR_DH_FWK_COMPONENT_MONITOR_NULL = -10011;
    constexpr int32_t ERR_DH_FWK_COMPONENT_TRANSPORT_OPT_FAILED = -10012;
    constexpr int32_t ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED = -10013;

    /* ResourceManager errno, range: [-10400, -10499] */
    constexpr int32_t ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL = -10400;
    constexpr int32_t ERR_DH_FWK_RESOURCE_KV_STORAGE_POINTER_NULL = -10401;
    constexpr int32_t ERR_DH_FWK_RESOURCE_DB_ADAPTER_OPERATION_FAIL = -10402;
    constexpr int32_t ERR_DH_FWK_RESOURCE_KV_STORAGE_OPERATION_FAIL = -10403;
    constexpr int32_t ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND = -10404;
    constexpr int32_t ERR_DH_FWK_RESOURCE_DB_MANUAL_SYNC_FAIL = -10405;
    constexpr int32_t ERR_DH_FWK_RESOURCE_INIT_DB_FAILED = -10406;
    constexpr int32_t ERR_DH_FWK_RESOURCE_UNINIT_DB_FAILED = -10407;
    constexpr int32_t ERR_DH_FWK_RESOURCE_REGISTER_DB_FAILED = -10408;
    constexpr int32_t ERR_DH_FWK_RESOURCE_UNREGISTER_DB_FAILED = -10409;
    constexpr int32_t ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY = -10410;
    constexpr int32_t ERR_DH_FWK_RESOURCE_SYNC_VERSIONINFO_FAIL = -10411;
    constexpr int32_t ERR_DH_FWK_RESOURCE_DEVICE_ID_NOT_EXIST = -10412;
    constexpr int32_t ERR_DH_FWK_RESOURCE_UUID_NOT_FOUND = -10413;
    constexpr int32_t ERR_DH_FWK_RESOURCE_CAPINFO_POINTER_NULL = -10414;
    constexpr int32_t ERR_DH_FWK_RESOURCE_RES_DB_DATA_INVALID = -10415;

    /* DistributedHardwareManager errno, range: [-10500, -10599] */
    constexpr int32_t ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_ONLINE = -10500;
    constexpr int32_t ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_OFFLINE = -10501;
    constexpr int32_t ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED = -10502;

    /* ComponentLoader errno, range: [-10600, -10699] */
    constexpr int32_t ERR_DH_FWK_LOADER_HANDLER_IS_NULL = -10600;
    constexpr int32_t ERR_DH_FWK_LOADER_CONFIG_JSON_INVALID = -10601;
    constexpr int32_t ERR_DH_FWK_LOADER_GET_LOCAL_VERSION_FAIL = -10602;
    constexpr int32_t ERR_DH_FWK_LOADER_DLCLOSE_FAIL = -10603;
    constexpr int32_t ERR_DH_FWK_LOADER_PROFILE_PATH_IS_NULL = -10604;

    /* Task errno, range: [-10700, -10799] */
    constexpr int32_t ERR_DH_FWK_TASK_TIMEOUT = -10700;

    /* DistributedHardwareService errno, range: [-10800, -10899] */
    constexpr int32_t ERR_DH_FWK_SERVICE_IPC_WRITE_PARA_FAIL = -10800;
    constexpr int32_t ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL = -10801;
    constexpr int32_t ERR_DH_FWK_SERVICE_IPC_READ_PARA_FAIL = -10802;
    constexpr int32_t ERR_DH_FWK_SERVICE_STRING_IS_EMPTY = -10803;
    constexpr int32_t ERR_DH_FWK_SERVICE_LOCAL_VERSION_NOT_EXIST = -10804;
    constexpr int32_t ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL = -10805;
    constexpr int32_t ERR_DH_FWK_SERVICE_REMOTE_IS_NULL = -10806;
    constexpr int32_t ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL = -10807;
    constexpr int32_t ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL = -10808;
    constexpr int32_t ERR_DH_FWK_SERVICE_MSG_INVALID = -10809;
    constexpr int32_t ERR_DH_FWK_AVTRANS_CALLBACK_IS_NULL = -10810;

    /* AccessManager errno, range: [-10900, -10999] */
    constexpr int32_t ERR_DH_FWK_ACCESS_INIT_DM_FAILED = -10900;
    constexpr int32_t ERR_DH_FWK_ACCESS_UNINIT_DM_FAILED = -10901;
    constexpr int32_t ERR_DH_FWK_ACCESS_REGISTER_DM_FAILED = -10902;
    constexpr int32_t ERR_DH_FWK_ACCESS_UNREGISTER_DM_FAILED = -10903;

    /* Hidump errno, range: [-11000, -11099] */
    constexpr int32_t ERR_DH_FWK_HIDUMP_ERROR = -11000;
    constexpr int32_t ERR_DH_FWK_HIDUMP_INVALID_ARGS = -11001;
    constexpr int32_t ERR_DH_FWK_HIDUMP_DPRINTF_ERROR = -11002;

    /* DHFWK Publisher errno, range: [-11100, -11199] */
    constexpr int32_t ERR_DH_FWK_PUBLISH_MSG_FAILED = -11100;
    constexpr uint32_t ERR_DH_FWK_PUBLISH_LISTENER_OVER_SIZE = -11101;

    /* DHFWK Check Permission errno, range: [-11200, -11299] */
    constexpr int32_t ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL = -11200;
    constexpr int32_t ERR_DH_FWK_IS_SYSTEM_HAP_CHECK_FAIL = -11201;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_ERRNO
