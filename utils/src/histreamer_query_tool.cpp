/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "histreamer_query_tool.h"

#include <dlfcn.h>
#include <malloc.h>

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(HiStreamerQueryTool);
using QueryAudioEncoderFunc = int32_t (*)(char*);
using QueryAudioDecoderFunc = int32_t (*)(char*);
using QueryVideoEncoderFunc = int32_t (*)(char*);
using QueryVideoDecoderFunc = int32_t (*)(char*);

QueryAudioEncoderFunc queryAudioEncoderFunc = nullptr;
QueryAudioDecoderFunc queryAudioDecoderFunc = nullptr;
QueryVideoEncoderFunc queryVideoEncoderFunc = nullptr;
QueryVideoDecoderFunc queryVideoDecoderFunc = nullptr;

constexpr const char *QUERY_AUDIO_ENCODER_FUNC_NAME = "QueryAudioEncoderAbilityStr";
constexpr const char *QUERY_AUDIO_DECODER_FUNC_NAME = "QueryAudioDecoderAbilityStr";
constexpr const char *QUERY_VIDEO_ENCODER_FUNC_NAME = "QueryVideoEncoderAbilityStr";
constexpr const char *QUERY_VIDEO_DECODER_FUNC_NAME = "QueryVideoDecoderAbilityStr";
constexpr const char *LOAD_SO = "libhistreamer_ability_querier.z.so";

constexpr uint32_t MAX_MESSAGES_LEN = 1 * 1024 * 1024;

void HiStreamerQueryTool::Init()
{
    if (isInit) {
        return;
    }
    DHLOGI("Start Init HiStreamer Query SO");
    void *pHandler = dlopen(LOAD_SO, RTLD_LAZY | RTLD_NODELETE);
    if (pHandler == nullptr) {
        DHLOGE("libhistreamer_ability_querier.z.so handler load failed, failed reason : %{public}s", dlerror());
        return;
    }
    
    queryAudioEncoderFunc = (QueryAudioEncoderFunc)dlsym(pHandler, QUERY_AUDIO_ENCODER_FUNC_NAME);
    if (queryAudioEncoderFunc == nullptr) {
        DHLOGE("get QueryAudioEncoderAbilityStr is nullptr, failed reason : %{public}s", dlerror());
        dlclose(pHandler);
        pHandler = nullptr;
        return;
    }

    queryAudioDecoderFunc = (QueryAudioDecoderFunc)dlsym(pHandler, QUERY_AUDIO_DECODER_FUNC_NAME);
    if (queryAudioDecoderFunc == nullptr) {
        DHLOGE("get QueryAudioDecoderAbilityStr is null, failed reason : %{public}s", dlerror());
        dlclose(pHandler);
        pHandler = nullptr;
        return;
    }

    queryVideoEncoderFunc = (QueryVideoEncoderFunc)dlsym(pHandler, QUERY_VIDEO_ENCODER_FUNC_NAME);
    if (queryVideoEncoderFunc == nullptr) {
        DHLOGE("get QueryVideoEncoderAbilityStr is null, failed reason : %{public}s", dlerror());
        dlclose(pHandler);
        pHandler = nullptr;
        return;
    }

    queryVideoDecoderFunc = (QueryVideoDecoderFunc)dlsym(pHandler, QUERY_VIDEO_DECODER_FUNC_NAME);
    if (queryVideoDecoderFunc == nullptr) {
        DHLOGE("get QueryVideoDecoderAbilityStr is null, failed reason : %{public}s", dlerror());
        dlclose(pHandler);
        pHandler = nullptr;
        return;
    }

    DHLOGI("Init Query HiStreamer Tool Success");
    isInit = true;
}

std::string HiStreamerQueryTool::QueryHiStreamerPluginInfo(HISTREAM_PLUGIN_TYPE type)
{
    Init();
    if (!isInit || queryAudioEncoderFunc == nullptr || queryAudioDecoderFunc == nullptr ||
        queryVideoEncoderFunc == nullptr || queryVideoDecoderFunc == nullptr) {
        DHLOGE("Query HiStreamer Tool Init failed");
        return "";
    }

    int32_t len = 0;
    char* res = reinterpret_cast<char *>(malloc(MAX_MESSAGES_LEN));
    if (res == nullptr) {
        DHLOGE("Malloc memory failed");
        return "";
    }
    switch (type) {
        case HISTREAM_PLUGIN_TYPE::AUDIO_ENCODER: {
            len = queryAudioEncoderFunc(res);
            break;
        }
        case HISTREAM_PLUGIN_TYPE::AUDIO_DECODER: {
            len = queryAudioDecoderFunc(res);
            break;
        }
        case HISTREAM_PLUGIN_TYPE::VIDEO_ENCODER: {
            len = queryVideoEncoderFunc(res);
            break;
        }
        case HISTREAM_PLUGIN_TYPE::VIDEO_DECODER: {
            len = queryVideoDecoderFunc(res);
            break;
        }
        default:
            break;
    }

    std::string result(res, len);
    free(res);
    res = nullptr;
    return result;
}
}
}