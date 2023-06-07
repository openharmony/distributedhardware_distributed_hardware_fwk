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

#ifndef OHOS_I_AV_ENGINE_PROVIDER_CALLBACK_H
#define OHOS_I_AV_ENGINE_PROVIDER_CALLBACK_H

#include "av_trans_errno.h"
#include "av_trans_types.h"

namespace OHOS {
namespace DistributedHardware {
/**
 * @brief AV engine provider callback interface.
 *
 * AV engine provider callback is used to receive the engine provider state change events.
 *
 * @since 1.0
 * @version 1.0
 */
class IAVEngineProviderCallback {
public:
    /**
     * @brief Destructor.
     * @return No return value.
     */
    virtual ~IAVEngineProviderCallback() = default;

    /**
     * @brief Report the engine provider state change events to the distributed service.
     * @param event  event content.
     * @return Returns DH_AVT_SUCCESS(0) if successful, otherwise returns other error code.
     */
    virtual int32_t OnProviderEvent(const AVTransEvent &event) = 0;
};
} // DistributedHardware
} // OHOS
#endif // OHOS_I_AV_ENGINE_PROVIDER_CALLBACK_H