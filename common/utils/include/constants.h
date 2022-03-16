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
    const std::u16string DHMS_STUB_INTERFACE_TOKEN = u"ohos.distributedhardware.accessToken";
    const std::string COMPONENTSLOAD_PROFILE_PATH = R"(/etc/distributed_hardware_components_cfg.json)";
    const std::string APP_ID = "dtbhardware_manager_service";
    const std::string GLOBAL_CAPABILITY_ID = "global_capability_info";
    const std::string RESOURCE_SEPARATOR = "###";
    const std::string DH_ID = "dh_id";
    const std::string DEV_ID = "dev_id";
    const std::string DEV_NAME = "dev_name";
    const std::string DEV_TYPE = "dev_type";
    const std::string DH_TYPE = "dh_type";
    const std::string DH_ATTRS = "dh_attrs";
    const std::string DH_LOG_TITLE_TAG = "DHFWK";
    const std::string DH_TASK_NAME_PREFIX = "Task_";
    const std::string DH_FWK_PKG_NAME = "ohos.dhardware";
    const std::string DH_COMPONENT_VERSIONS = "componentVersions";
    const std::string DH_COMPONENT_TYPE = "dhType";
    const std::string DH_COMPONENT_SINK_VER = "version";
    const std::string DH_COMPONENT_DEFAULT_VERSION = "1.0";
}
}
#endif