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

#ifndef OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_DSOFTBUS_H
#define OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_DSOFTBUS_H

#include <algorithm>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "av_trans_buffer.h"
#include "av_trans_errno.h"
#include "av_trans_constants.h"
#include "av_trans_log.h"
#include "av_trans_meta.h"
#include "av_trans_types.h"
#include "av_trans_utils.h"
#include "nlohmann/json.hpp"
#include "foundation/osal/thread/task.h"
#include "plugin_types.h"
#include "plugin_manager.h"
#include "softbus_channel_adapter.h"

namespace OHOS {
namespace DistributedHardware {

using namespace Media::Plugin;

using json = nlohmann::json;
using AVDataCallback = std::function<void(std::shared_ptr<Buffer>)>;

} // namespace DistributedHardware
} // namespace OHOS

#endif // OHOS_AV_TRANS_ENGINE_PLUGINS_OUTPUT_DSOFTBUS_H