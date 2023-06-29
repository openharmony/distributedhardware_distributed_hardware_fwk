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

#ifndef OHOS_OUTPUT_CONTROLLER_LISTENER_H
#define OHOS_OUTPUT_CONTROLLER_LISTENER_H
#include "controllable_output.h"
#include "plugin_buffer.h"
#include <memory>

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;
class OutputControllerListener {
public:
    OutputControllerListener(std::shared_ptr<ControllableOutput> output)
        : output_(output) {}
    ~OutputControllerListener() = default;

public:
    int32_t OnOutput(const std::shared_ptr<Plugin::Buffer>& data);

private:
    std::weak_ptr<ControllableOutput> output_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_OUTPUT_CONTROLLER_LISTENER_H