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
#include "av_trans_log.h"
#include "output_controller_listener.h"
#include "output_controller_constants.h"

namespace OHOS {
namespace DistributedHardware {
int32_t OutputControllerListener::OnOutput(const std::shared_ptr<Plugin::Buffer>& data)
{
    std::shared_ptr<ControllableOutput> output = output_.lock();
    TRUE_RETURN_V_MSG_E((!output), NOTIFY_FAILED, "Output is nullptr, notify failed.");
    output->OnOutPut(data);
    return NOTIFY_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS