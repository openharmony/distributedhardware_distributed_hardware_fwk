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

#ifndef OHOS_DISTRIBUTED_HARDWARE_CONSTANTS_H
#define OHOS_DISTRIBUTED_HARDWARE_CONSTANTS_H

#include <string>
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
    constexpr int32_t LOG_MAX_LEN = 4096;
    constexpr uint32_t MAX_MESSAGE_LEN = 40 * 1024 * 1024;
    constexpr uint32_t MIN_MESSAGE_LEN = 0;
    constexpr uint32_t MAX_ID_LEN = 256;
    constexpr uint32_t MAX_TOPIC_SIZE = 128;
    constexpr uint32_t MAX_LISTENER_SIZE = 256;
    constexpr uint32_t MAX_COMP_SIZE = 128;
    constexpr uint32_t MAX_ARR_SIZE = 10000;
    constexpr uint32_t MAX_DB_RECORD_SIZE = 10000;
    constexpr uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
    constexpr int32_t MODE_ENABLE = 0;
    constexpr int32_t MODE_DISABLE = 1;
    constexpr uint32_t MAX_SWITCH_SIZE = 256;
    constexpr uint32_t MAX_ROUND_SIZE = 1000;
    constexpr uint32_t MAX_JSON_SIZE = 40 * 1024 * 1024;
    constexpr uint32_t MAX_HASH_SIZE = 64;
    constexpr uint32_t MAX_KEY_SIZE = 256;
    constexpr uint32_t MAX_DH_DESCRIPTOR_ARRAY_SIZE = 4096;
    const uint32_t EVENT_VERSION_INFO_DB_RECOVER = 101;
    const uint32_t EVENT_CAPABILITY_INFO_DB_RECOVER = 201;
    const uint32_t EVENT_DATA_SYNC_MANUAL = 301;
    const uint32_t EVENT_META_INFO_DB_RECOVER = 401;

    const std::string LOW_LATENCY_KEY = "identity";
    const std::string APP_ID = "dtbhardware_manager_service";
    const std::string GLOBAL_CAPABILITY_ID = "global_capability_info";
    const std::string GLOBAL_VERSION_ID = "global_version_info";
    const std::string LOCAL_CAPABILITY_ID = "local_capability_info";
    const std::string GLOBAL_META_INFO = "global_meta_info";
    const std::string RESOURCE_SEPARATOR = "###";
    const std::string DH_FWK_PKG_NAME = "ohos.dhardware";

    constexpr const char *DH_ID = "dh_id";
    constexpr const char *DEV_ID = "dev_id";
    constexpr const char *DEV_NAME = "dev_name";
    constexpr const char *DEV_TYPE = "dev_type";
    constexpr const char *DH_TYPE = "dh_type";
    constexpr const char *DH_ATTRS = "dh_attrs";
    constexpr const char *DH_SUBTYPE = "dh_subtype";
    constexpr const char *DEV_UDID_HASH = "udid_hash";
    constexpr const char *DH_VER = "dh_ver";
    constexpr const char *COMP_VER = "comp_ver";
    constexpr const char *NAME = "name";
    constexpr const char *TYPE = "type";
    constexpr const char *HANDLER = "handler";
    constexpr const char *SOURCE_VER = "source_ver";
    constexpr const char *SINK_VER = "sink_ver";
    constexpr const char *SOURCE_FEATURE_FILTER = "source_feature_filter";
    constexpr const char *SINK_SUPPORTED_FEATURE = "sink_supported_feature";
    constexpr const char *LOW_LATENCY_ENABLE = "low_latency_enable";
    constexpr const char *DO_RECOVER = "DoRecover";
    constexpr const char *SEND_ONLINE = "SendOnLine";
    constexpr const char *COMPONENTSLOAD_PROFILE_PATH =
        "etc/distributedhardware/distributed_hardware_components_cfg.json";
} // namespace DistributedHardware
} // namespace OHOS
#endif
