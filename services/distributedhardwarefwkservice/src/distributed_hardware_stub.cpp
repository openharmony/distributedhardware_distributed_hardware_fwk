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

#include "distributed_hardware_stub.h"

#include "anonymous_string.h"
#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_manager_factory.h"
#include "nlohmann/json.hpp"
namespace OHOS {
namespace DistributedHardware {
int32_t DistributedHardwareStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        DHLOGE("ReadInterfaceToken fail!");
        return ERR_INVALID_DATA;
    }
    switch (code) {
        case QUERY_SINK_VERSION: {
            std::unordered_map<DHType, std::string> versionMap;
            auto ret = DistributedHardwareManagerFactory::GetInstance().GetComponentVersion(versionMap);
            if (ret != DH_FWK_SUCCESS) {
                DHLOGE("GetComponentVersion failed, errCode = %d", ret);
                return ret;
            }
            if (versionMap.empty()) {
                DHLOGE("versionMap is empty");
                return ERR_DH_FWK_SERVICE_LOCAL_VERSION_NOT_EXIST;
            }
            auto version = ToJson(versionMap);
            if (!reply.WriteString(version)) {
                DHLOGE("write version failed");
                return ERR_DH_FWK_SERVICE_IPC_WRITE_PARA_FAIL;
            }
            return DH_FWK_SUCCESS;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return 0;
}

std::string DistributedHardwareStub::ToJson(const std::unordered_map<DHType, std::string> &versionMap) const
{
    nlohmann::json jsonObj;
    for (const auto &item : versionMap) {
        nlohmann::json json;
        json[DH_COMPONENT_TYPE] = item.first;
        json[DH_COMPONENT_SINK_VER] = item.second;
        jsonObj[DH_COMPONENT_VERSIONS].emplace_back(json);
    }
    return jsonObj.dump();
}
}
}
