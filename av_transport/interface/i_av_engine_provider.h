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

#ifndef OHOS_I_AV_ENGINE_PROVIDER_H
#define OHOS_I_AV_ENGINE_PROVIDER_H

#include <memory>
#include <string>
#include <vector>

#include "i_av_engine_provider_callback.h"
#include "i_av_receiver_engine.h"
#include "i_av_sender_engine.h"

namespace OHOS {
namespace DistributedHardware {
/**
 * @brief AV engine provider interface.
 *
 * AV engine provider is used to create or query the sender engines and receiver engines.
 * It is loaded and running both on the source and the sink device.
 *
 * @since 1.0
 * @version 1.0
 */
class IAVEngineProvider {
public:
    /**
     * @brief Destructor.
     * @return No return value.
     */
    virtual ~IAVEngineProvider() = default;

    /**
     * @brief Create an av sender engine.
     * @param peerDevId  id of the remote target device.
     * @return Returns a IAVSenderEngine shared pointer if successful, otherwise returns null pointer.
     */
    virtual std::shared_ptr<IAVSenderEngine> CreateAVSenderEngine(const std::string &peerDevId)
    {
        (void)peerDevId;
        return nullptr;
    }

    /**
     * @brief Get the av sender engine list.
     * @return Returns the av sender engine list if successful, otherwise returns empty vector.
     */
    virtual std::vector<std::shared_ptr<IAVSenderEngine>> GetAVSenderEngineList()
    {
        std::vector<std::shared_ptr<IAVSenderEngine>> list;
        return list;
    }

    /**
     * @brief Create an av receiver engine.
     * @param peerDevId  id of the remote target device.
     * @return Returns a IAVReceiverEngine shared pointer if successful, otherwise returns null pointer.
     */
    virtual std::shared_ptr<IAVReceiverEngine> CreateAVReceiverEngine(const std::string &peerDevId)
    {
        (void)peerDevId;
        return nullptr;
    }

    /**
     * @brief Get the av receiver engine list.
     * @return Returns the av receiver engine list if successful, otherwise returns empty vector.
     */
    virtual std::vector<std::shared_ptr<IAVReceiverEngine>> GetAVReceiverEngineList()
    {
        std::vector<std::shared_ptr<IAVReceiverEngine>> list;
        return list;
    }

    /**
     * @brief Register interface callback to the engine provider.
     * @param callback  interface callback.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t RegisterProviderCallback(const std::shared_ptr<IAVEngineProviderCallback> &callback)
    {
        (void)callback;
        return DH_AVT_SUCCESS;
    }
};
} // DistributedHardware
} // OHOS
#endif // OHOS_I_AV_ENGINE_PROVIDER_H