/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "distributed_hardware_proxy.h"

#include <unordered_set>

#include "anonymous_string.h"
#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "nlohmann/json.hpp"
#include "parcel.h"

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "DistributedHardwareProxy"
const std::unordered_set<DHType> DH_TYPE_SET {
    DHType::UNKNOWN, DHType::CAMERA, DHType::AUDIO, DHType::SCREEN, DHType::GPS, DHType::INPUT,
    DHType::HFP, DHType::A2D, DHType::VIRMODEM_MIC, DHType::VIRMODEM_SPEAKER, DHType::MAX_DH,
   };

int32_t DistributedHardwareProxy::RegisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_FWK_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)topic)) {
        DHLOGE("DistributedHardwareProxy write topic failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("DistributedHardwareProxy write listener failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest((uint32_t)IDistributedHardware::Message::REG_PUBLISHER_LISTNER,
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Register Publisher Listener failed, ret: %d", ret);
    }

    return ret;
}

int32_t DistributedHardwareProxy::UnregisterPublisherListener(const DHTopic topic,
    const sptr<IPublisherListener> &listener)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_FWK_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)topic)) {
        DHLOGE("DistributedHardwareProxy write topic failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("DistributedHardwareProxy write listener failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest((uint32_t)IDistributedHardware::Message::UNREG_PUBLISHER_LISTENER,
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Unregister Publisher Listener failed, ret: %d", ret);
    }

    return ret;
}

int32_t DistributedHardwareProxy::PublishMessage(const DHTopic topic, const std::string &msg)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return ERR_DH_FWK_SERVICE_REMOTE_IS_NULL;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        DHLOGE("WriteInterfaceToken fail!");
        return ERR_DH_FWK_SERVICE_WRITE_TOKEN_FAIL;
    }
    if (!data.WriteUint32((uint32_t)topic)) {
        DHLOGE("DistributedHardwareProxy write topic failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    if (!data.WriteString(msg)) {
        DHLOGE("DistributedHardwareProxy write listener failed");
        return ERR_DH_FWK_SERVICE_WRITE_INFO_FAIL;
    }
    int32_t ret = remote->SendRequest((uint32_t)IDistributedHardware::Message::PUBLISH_MESSAGE,
        data, reply, option);
    if (ret != NO_ERROR) {
        DHLOGE("Send Request failed, ret: %d", ret);
        return ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL;
    }

    ret = reply.ReadInt32();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("PublishMessage failed, ret: %d", ret);
    }

    return ret;
}

void from_json(const nlohmann::json &jsonObj, std::unordered_map<DHType, std::string> &versionMap)
{
    for (const auto &item : jsonObj.value(DH_COMPONENT_VERSIONS, nlohmann::json {})) {
        DHType dhType = (DH_TYPE_SET.find(item.value(DH_COMPONENT_TYPE, DHType::UNKNOWN)) != DH_TYPE_SET.end()) ?
            item.value(DH_COMPONENT_TYPE, DHType::UNKNOWN) :
            DHType::UNKNOWN;
        std::string sinkVersion = item.value(DH_COMPONENT_SINK_VER, DH_COMPONENT_DEFAULT_VERSION);
        versionMap.emplace(std::pair<DHType, std::string>(dhType, sinkVersion));
    }
}

std::unordered_map<DHType, std::string> DistributedHardwareProxy::FromJson(const std::string &json) const
{
    return nlohmann::json::parse(json).get<std::unordered_map<DHType, std::string>>();
}
} // namespace DistributedHardware
} // namespace OHOS
