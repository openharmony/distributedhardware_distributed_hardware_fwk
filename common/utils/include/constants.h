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
    constexpr int32_t ENABLE_TIMEOUT_MS = 1000;
    constexpr int32_t DISABLE_TIMEOUT_MS = 500;
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

    const std::string LOW_LATENCY_KEY = "identity";
    const std::u16string DHMS_STUB_INTERFACE_TOKEN = u"ohos.distributedhardware.accessToken";
    const std::string APP_ID = "dtbhardware_manager_service";
    const std::string GLOBAL_CAPABILITY_ID = "global_capability_info";
    const std::string GLOBAL_VERSION_ID = "global_version_info";
    const std::string LOCAL_CAPABILITY_ID = "local_capability_info";
    const std::string GLOBAL_META_INFO = "global_meta_info";
    const std::string RESOURCE_SEPARATOR = "###";
    const std::string DH_ID = "dh_id";
    const std::string DEV_ID = "dev_id";
    const std::string DEV_NAME = "dev_name";
    const std::string DEV_TYPE = "dev_type";
    const std::string DH_TYPE = "dh_type";
    const std::string DH_ATTRS = "dh_attrs";
    const std::string DH_SUBTYPE = "dh_subtype";
    const std::string DEV_UDID_HASH = "udid_hash";
    const std::string DH_LOG_TITLE_TAG = "DHFWK";
    const std::string DH_VER = "dh_ver";
    const std::string COMP_VER = "comp_ver";
    const std::string NAME = "name";
    const std::string TYPE = "type";
    const std::string HANDLER = "handler";
    const std::string SOURCE_VER = "source_ver";
    const std::string SINK_VER = "sink_ver";
    const std::string SOURCE_FEATURE_FILTER = "source_feature_filter";
    const std::string SINK_SUPPORTED_FEATURE = "sink_supported_feature";
    const std::string DH_TASK_NAME_PREFIX = "Task_";
    const std::string DH_FWK_PKG_NAME = "ohos.dhardware";
    const std::string DH_FWK_SESSION_NAME = "ohos.dhardware.session_";
    const std::string DH_COMPONENT_VERSIONS = "componentVersions";
    const std::string DH_COMPONENT_TYPE = "dhType";
    const std::string DH_COMPONENT_SINK_VER = "version";
    const std::string DH_COMPONENT_DEFAULT_VERSION = "1.0";
    const std::string LOW_LATENCY_ENABLE = "low_latency_enable";
    constexpr const char *DO_RECOVER = "DoRecover";
    constexpr const char *SEND_ONLINE = "SendOnLine";
    constexpr const char *DISABLE_TASK_INNER = "DisableTask";
    constexpr const char *ENABLE_TASK_INNER = "EnableTask";
    constexpr const char *META_DISABLE_TASK_INNER = "MetaDisableTask";
    constexpr const char *META_ENABLE_TASK_INNER = "MetaEnableTask";
    constexpr const char *OFFLINE_TASK_INNER = "OffLineTask";
    constexpr const char *TRIGGER_TASK = "TriggerTask";
    constexpr const char *EVENT_RUN = "EventRun";
    constexpr const char *START_EVENT = "StartEvent";
    constexpr const char *COMPONENTSLOAD_PROFILE_PATH =
        "etc/distributedhardware/distributed_hardware_components_cfg.json";

    const uint32_t EVENT_VERSION_INFO_DB_RECOVER = 101;
    const uint32_t EVENT_CAPABILITY_INFO_DB_RECOVER = 201;
    const uint32_t EVENT_DATA_SYNC_MANUAL = 301;
    const uint32_t EVENT_META_INFO_DB_RECOVER = 401;

    const std::string ISOMERISM_EVENT_KEY = "isomerism_event";
    const std::string ISOMERISM_EVENT_CONNECT_VAL = "isomerism_connect";
    const std::string ISOMERISM_EVENT_DISCONNECT_VAL = "isomerism_disconnect";
} // namespace DistributedHardware
} // namespace OHOS
#endif
