
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
#include <string>
#include <atomic>
#include "single_instance.h"
namespace OHOS {
namespace DistributedHardware {
enum class HISTREAM_PLUGIN_TYPE : uint8_t {
    AUDIO_ENCODER,
    AUDIO_DECODER,
    VIDEO_ENCODER,
    VIDEO_DECODER
};

class HiStreamerQueryTool {
DECLARE_SINGLE_INSTANCE(HiStreamerQueryTool);
public:
    std::string QueryHiStreamerPluginInfo(HISTREAM_PLUGIN_TYPE type);
private:
    std::atomic<bool> isInit = false;
    void Init();
};
}
}