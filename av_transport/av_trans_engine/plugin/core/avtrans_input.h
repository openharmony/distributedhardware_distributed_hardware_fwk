/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef HISTREAMER_PLUGIN_CORE_AVTRANS_INPUT_H
#define HISTREAMER_PLUGIN_CORE_AVTRANS_INPUT_H

#include <cstddef>
#include <cstdint>
#include <memory>

#include "plugin/core/base.h"
#include "plugin/common/plugin_types.h"

namespace OHOS {
namespace DistributedHardware {
struct AvTransInputPlugin;

using namespace OHOS::Media;
using namespace OHOS::Media::Plugin;

class AvTransInput : public Base {
public:
    AvTransInput(const AvTransInput &) = delete;
    AvTransInput operator=(const AvTransInput &) = delete;
    ~AvTransInput() override = default;

    Status Pause();

    Status Resume();

    Status PushData(const std::string& inPort, std::shared_ptr<Plugin::Buffer> buffer, int32_t offset);

    Status SetDataCallback(std::function<void(std::shared_ptr<Plugin::Buffer>)> callback);

    AvTransInput(uint32_t pkgVer, uint32_t apiVer, std::shared_ptr<AvTransInputPlugin> plugin);

private:
    std::shared_ptr<AvTransInputPlugin> AvTransInputPlugin_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // HISTREAMER_PLUGIN_CORE_AVTRANS_INPUT_H