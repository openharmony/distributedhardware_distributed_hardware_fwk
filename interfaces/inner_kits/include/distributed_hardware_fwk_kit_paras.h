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

#ifndef OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_PARAS_H
#define OHOS_DISTRIBUTED_HARDWARE_FWK_KIT_PARAS_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
    const std::string KEY_HISTREAMER_VIDEO_ENCODER = "histmVidEnc";
    const std::string KEY_HISTREAMER_VIDEO_DECODER = "histmVidDec";
    const std::string KEY_HISTREAMER_AUDIO_ENCODER = "histmAudEnc";
    const std::string KEY_HISTREAMER_AUDIO_DECODER = "histmAudDec";
    /**
     * @brief query local system common specifications,
     * such as Audio/Video Encoder/Decoder
     */
    enum class QueryLocalSysSpecType : uint32_t {
        MIN = 0,
        HISTREAMER_AUDIO_ENCODER,
        HISTREAMER_AUDIO_DECODER,
        HISTREAMER_VIDEO_ENCODER,
        HISTREAMER_VIDEO_DECODER,
        MAX = 5
    };
}
}

#endif