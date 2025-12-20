/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_H
#define OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <set>
#include "refbase.h"

#include "distributed_hardware_fwk_kit_paras.h"
#include "dhardware_descriptor.h"
#include "ipublisher_listener.h"
#include "idistributed_hardware.h"
#include "iget_dh_descriptors_callback.h"
#include "ihardware_status_listener.h"
#include "device_type.h"
#include "av_trans_types.h"
#include "iav_trans_control_center_callback.h"

#ifndef API_EXPORT
#define API_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace DistributedHardware {
class DistributedHardwareFwkKit final {
public:
    /**
     * @brief Constructor.
     * @return No return value.
     */
    API_EXPORT DistributedHardwareFwkKit();

    /**
     * @brief Destructor.
     * @return No return value.
     */
    API_EXPORT ~DistributedHardwareFwkKit();

    /**
     * @brief Register publisher listener.
     * @param topic    Distributed hardware topic.
     * @param listener Publisher listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t RegisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener);

    /**
     * @brief Unregister publisher listener.
     * @param topic    Distributed hardware topic.
     * @param listener Publisher listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t UnregisterPublisherListener(const DHTopic topic, sptr<IPublisherListener> listener);

    /**
     * @brief Publish message.
     * @param topic   Distributed hardware topic.
     * @param message Message content.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t PublishMessage(const DHTopic topic, const std::string &message);

    /**
     * @brief Query Local system specifications
     *
     * @param spec specification type
     * @return specification in string format
     */
    API_EXPORT std::string QueryLocalSysSpec(QueryLocalSysSpecType spec);

    /**
     * @brief Initialize distributed av control center
     *
     * @param transRole transport role, eg. sender or receiver
     * @param engineId  transport engine id
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t InitializeAVCenter(const TransRole &transRole, int32_t &engineId);

    /**
     * @brief Release distributed av control center
     *
     * @param engineId  transport engine id
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t ReleaseAVCenter(int32_t engineId);

    /**
     * @brief Create control channel betweent the local and the remote av control center
     *
     * @param engineId  transport engine id
     * @param peerDevId the peer device id
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t CreateControlChannel(int32_t engineId, const std::string &peerDevId);

    /**
     * @brief Notify event from transport engine to av control center
     *
     * @param engineId  transport engine id
     * @param event the event content
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t NotifyAVCenter(int32_t engineId, const AVTransEvent &event);

    /**
     * @brief Register av control center callback.
     *
     * @param engineId  transport engine id
     * @param callback av control center callback.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t RegisterCtlCenterCallback(int32_t engineId, const sptr<IAvTransControlCenterCallback> callback);

    /**
     * @brief Pause distributed hardware.
     *
     * @param dhType  distributed hardware type
     * @param networkId distributed hardware networkId.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t PauseDistributedHardware(DHType dhType, const std::string &networkId);

    /**
     * @brief Resume distributed hardware.
     *
     * @param dhType  distributed hardware type
     * @param networkId distributed hardware networkId.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t ResumeDistributedHardware(DHType dhType, const std::string &networkId);

    /**
     * @brief Stop distributed hardware.
     *
     * @param dhType  distributed hardware type
     * @param networkId distributed hardware networkId.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t StopDistributedHardware(DHType dhType, const std::string &networkId);

    /**
     * @brief Get distributed hardware.
     *
     * @param networkId distributed hardware networkId.
     * @param descriptors distributed hardware descriptor list.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t GetDistributedHardware(const std::string &networkId, EnableStep enableStep,
        const sptr<IGetDhDescriptorsCallback> callback);

    /**
     * @brief Register distributed hardware status listener.
     *
     * @param listener distributed hardware status listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t RegisterDHStatusListener(sptr<IHDSinkStatusListener> listener);

    /**
     * @brief Unregister distributed hardware status listener.
     *
     * @param listener distributed hardware status listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t UnregisterDHStatusListener(sptr<IHDSinkStatusListener> listener);

    /**
     * @brief Register distributed hardware status listener.
     *
     * @param networkId distributed hardware networkId.
     * @param listener distributed hardware status listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t RegisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener);

    /**
     * @brief Unregister distributed hardware status listener.
     *
     * @param networkId distributed hardware networkId.
     * @param listener distributed hardware status listener.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t UnregisterDHStatusListener(const std::string &networkId, sptr<IHDSourceStatusListener> listener);

    /**
     * @brief Enable distributed hardware sink.
     *
     * @param descriptors distributed hardware descriptor list.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t EnableSink(const std::vector<DHDescriptor> &descriptors);

    /**
     * @brief Disable distributed hardware sink.
     *
     * @param descriptors distributed hardware descriptor list.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t DisableSink(const std::vector<DHDescriptor> &descriptors);

    /**
     * @brief Enable distributed hardware source.
     *
     * @param networkId distributed hardware networkId.
     * @param descriptors distributed hardware descriptor list.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t EnableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors);

    /**
     * @brief Disable distributed hardware source.
     *
     * @param networkId distributed hardware networkId.
     * @param descriptors distributed hardware descriptor list.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t DisableSource(const std::string &networkId, const std::vector<DHDescriptor> &descriptors);

    /**
     * @brief Load distributed HDF.
     *
     * @param dhType distributed hardware type.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t LoadDistributedHDF(const DHType dhType);

    /**
     * @brief Unload distributed HDF.
     *
     * @param dhType distributed hardware type.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t UnLoadDistributedHDF(const DHType dhType);

    /**
     * @brief Register hardware access listener.
     *
     * @param dhType distributed hardware type.
     * @param callback hardware access listener.
     * @param timeOut time out ms.
     * @param pkgName owner info.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t RegisterHardwareAccessListener(const DHType dhType, sptr<IAuthorizationResultCallback> callback,
        int32_t timeOut, const std::string &pkgName);

    /**
     * @brief Unregister hardware access listener.
     *
     * @param dhType distributed hardware type.
     * @param callback hardware access listener.
     * @param pkgName owner info.
     * @return Returns 0 if success.
     */
    API_EXPORT int32_t UnregisterHardwareAccessListener(const DHType dhType,
        sptr<IAuthorizationResultCallback> callback, const std::string &pkgName);

    /**
     * @brief Set authorization result.
     *
     * @param dhType distributed hardware type.
     * @param requestId distributed hardware request id.
     * @param granted authorization result.
     */
    API_EXPORT void SetAuthorizationResult(const DHType dhType, const std::string &requestId, bool granted);

private:
    /**
     * @brief Determine whether the topic is valid.
     * @param topic Distributed hardware topic.
     * @return Returns true if success.
     */
    bool IsDHTopicValid(DHTopic topic);

    /**
     * @brief Determine whether the QueryLocalSysSpecType is valid.
     * @param topic Query Local Sys Spec Type.
     * @return Returns true if success.
     */
    bool IsQueryLocalSysSpecTypeValid(QueryLocalSysSpecType spec);

    /**
     * @brief Load distributed hardware SA.
     *
     * @return Returns 0 if success.
     */
    int32_t LoadDistributedHardwareSA();

private:
    std::mutex dfwkLoadServiceMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_H