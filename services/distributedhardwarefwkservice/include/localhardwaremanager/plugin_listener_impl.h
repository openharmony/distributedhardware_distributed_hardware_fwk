/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_PLUGIN_LISTENER_IMPL_H
#define OHOS_DISTRIBUTED_HARDWARE_PLUGIN_LISTENER_IMPL_H

#include "device_type.h"
#include "ihardware_handler.h"

namespace OHOS {
namespace DistributedHardware {
class PluginListenerImpl : public PluginListener {
public:
    explicit PluginListenerImpl(const DHType type) : dhType_(type) {}
    virtual ~PluginListenerImpl() = default;

    virtual void PluginHardware(const std::string &dhId, const std::string &attrs, const std::string &subtype) override;
    virtual void UnPluginHardware(const std::string &dhId) override;

private:
    DHType dhType_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif
