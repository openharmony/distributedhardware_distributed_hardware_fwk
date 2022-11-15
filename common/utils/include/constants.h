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
    constexpr uint32_t MAX_ID_LEN = 256;
    constexpr uint32_t MAX_TOPIC_SIZE = 128;
    constexpr uint32_t MAX_LISTENER_SIZE = 256;
    constexpr uint32_t MAX_COMP_SIZE = 128;
    constexpr uint32_t MAX_DB_RECORD_SIZE = 10000;
    constexpr uint32_t MAX_ONLINE_DEVICE_SIZE = 10000;
    const std::u16string DHMS_STUB_INTERFACE_TOKEN = u"ohos.distributedhardware.accessToken";
    const std::string COMPONENTSLOAD_PROFILE_PATH =
        R"(/vendor/etc/distributedhardware/distributed_hardware_components_cfg.json)";
    const std::string APP_ID = "dtbhardware_manager_service";
    const std::string GLOBAL_CAPABILITY_ID = "global_capability_info";
    const std::string GLOBAL_VERSION_ID = "global_version_info";
    const std::string RESOURCE_SEPARATOR = "###";
    const std::string DH_ID = "dh_id";
    const std::string DEV_ID = "dev_id";
    const std::string DEV_NAME = "dev_name";
    const std::string DEV_TYPE = "dev_type";
    const std::string DH_TYPE = "dh_type";
    const std::string DH_ATTRS = "dh_attrs";
    const std::string DH_LOG_TITLE_TAG = "DHFWK";
    const std::string DH_VER = "dh_ver";
    const std::string COMP_VER = "comp_ver";
    const std::string NAME = "name";
    const std::string TYPE = "type";
    const std::string HANDLER = "handler";
    const std::string SOURCE_VER = "source_ver";
    const std::string SINK_VER = "sink_ver";
    const std::string DH_TASK_NAME_PREFIX = "Task_";
    const std::string DH_FWK_PKG_NAME = "ohos.dhardware";
    const std::string DH_COMPONENT_VERSIONS = "componentVersions";
    const std::string DH_COMPONENT_TYPE = "dhType";
    const std::string DH_COMPONENT_SINK_VER = "version";
    const std::string DH_COMPONENT_DEFAULT_VERSION = "1.0";
    const std::string LOW_LATENCY_ENABLE = "low_latency_enable";
} // namespace DistributedHardware
} // namespace OHOS
#endif
