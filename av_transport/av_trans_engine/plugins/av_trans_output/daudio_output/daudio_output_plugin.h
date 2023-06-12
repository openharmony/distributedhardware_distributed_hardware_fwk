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

#ifndef OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_AUDIO_H
#define OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_AUDIO_H

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>

#include "av_trans_buffer.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_meta.h"
#include "av_trans_types.h"
#include "av_trans_utils.h"
#include "foundation/osal/thread/mutex.h"
#include "foundation/osal/thread/scoped_lock.h"
#include "foundation/osal/thread/task.h"
#include "nlohmann/json.hpp"
#include "plugin_manager.h"
#include "plugin_types.h"
#include "plugin/plugins/ffmpeg_adapter/utils/ffmpeg_utils.h"

namespace OHOS {
namespace DistributedHardware {

using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;

using json = nlohmann::json;
using AVDataCallback = std::function<void(std::shared_ptr<Plugin::Buffer>)>;

} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_AUDIO_H