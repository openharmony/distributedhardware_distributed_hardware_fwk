/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_LOADER_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_LOADER_H

#include <atomic>
#include <map>
#include <vector>

#include "single_instance.h"
#include "distributed_hardware_errno.h"
#include "device_type.h"
#include "ihardware_handler.h"
#include "idistributed_hardware_sink.h"
#include "idistributed_hardware_source.h"
#include "utils/impl_utils.h"

namespace OHOS {
namespace DistributedHardware {
struct CompConfig {
    std::string name;
    DHType type;
    std::string compHandlerLoc;
    std::string compHandlerVersion;
    std::string compSourceLoc;
    std::string compSourceVersion;
    std::string compSinkLoc;
    std::string compSinkVersion;
};

struct CompHandler {
    void *sourceHandler;
    void *sinkHandler;
    void *hardwareHandler;
};

const std::string COMPONENTSLOAD_DISTRIBUTED_COMPONENTS = "distributed_components";

class ComponentLoader {
    DECLARE_SINGLE_INSTANCE_BASE(ComponentLoader);

public:
    ComponentLoader() : isLocalVersionInit_(false) {}
    ~ComponentLoader() {}

public:
    int32_t Init();
    int32_t GetHardwareHandler(const DHType dhType, IHardwareHandler *&hardwareHandlerPtr);
    int32_t GetSource(const DHType dhType, IDistributedHardwareSource *&sourcePtr);
    int32_t GetSink(const DHType dhType, IDistributedHardwareSink *&sinkPtr);
    int32_t UnInit();
    int32_t ReleaseHardwareHandler(const DHType dhType);
    int32_t ReleaseSource(const DHType dhType);
    int32_t ReleaseSink(const DHType dhType);
    std::vector<DHType> GetAllCompTypes();
    int32_t GetLocalDHVersion(DHVersion &dhVersion);

private:
    void *GetHandler(const std::string &soName);
    void GetAllHandler(std::map<DHType, CompConfig> &dhtypeMap);
    int32_t ReleaseHandler(void *&handler);
    int32_t GetCompPathAndVersion(const std::string &jsonStr, std::map<DHType, CompConfig> &dhtypeMap);
    CompVersion GetCompVersionFromComConfig(const CompConfig& cCfg);
    int32_t ParseConfig();
    bool IsDHTypeExist(DHType dhType);
    std::string Readfile(const std::string &filePath);

private:
    DHVersion localDHVersion_;
    std::map<DHType, CompHandler> compHandlerMap_;
    std::atomic<bool> isLocalVersionInit_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
