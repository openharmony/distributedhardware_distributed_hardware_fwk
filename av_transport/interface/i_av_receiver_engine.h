/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_I_AV_RECEIVER_ENGINE_H
#define OHOS_I_AV_RECEIVER_ENGINE_H

#include <memory>
#include <string>

#include "av_trans_errno.h"
#include "av_trans_message.h"
#include "av_trans_types.h"
#include "i_av_receiver_engine_callback.h"

namespace OHOS {
namespace DistributedHardware {
/**
 * @brief AV receiver engine interface.
 *
 * AV receiver engine is loaded and running on the sink device.
 * It supports the management and control the histreamer pipeline, as well as processing of video and audio data.
 *
 * @since 1.0
 * @version 1.0
 */
class IAVReceiverEngine {
public:
    /**
     * @brief Destructor.
     * @return No return value.
     */
    virtual ~IAVReceiverEngine() = default;

    /**
     * @brief Initialize the av receiver engine.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t Initialize() = 0;

    /**
     * @brief Release the av receiver engine.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t Release() = 0;

    /**
     * @brief Start the pipeline and plugins in the receiver engine.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t Start() = 0;

    /**
     * @brief Stop the pipeline and plugins in the receiver engine.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t Stop() = 0;

    /**
     * @brief Set parameter to the receiver engine.
     * @param tag    parameter key.
     * @param value  parameter value.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t SetParameter(AVTransTag tag, const std::string &value) = 0;

    /**
     * @brief Send message to the receiver engine or the source device.
     * @param message  message content.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t SendMessage(const std::shared_ptr<AVTransMessage> &message) = 0;

    /**
     * @brief Create control channel for the receiver engine.
     * @param dstDevIds    ids of the target devices.
     * @param attribution  channel attributes.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t CreateControlChannel(const std::vector<std::string> &dstDevIds,
        const ChannelAttribute &attribution) = 0;

    /**
     * @brief Register interface callback to the receiver engine.
     * @param callback  interface callback.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t RegisterReceiverCallback(const std::shared_ptr<IAVReceiverEngineCallback> &callback) = 0;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_I_AV_RECEIVER_ENGINE_H