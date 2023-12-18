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

#ifndef OHOS_IPUBLISHER_LISTENER_H
#define OHOS_IPUBLISHER_LISTENER_H

#include <cstdint>
#include <string>

#include <iremote_broker.h>

namespace OHOS {
namespace DistributedHardware {
enum class DHTopic : uint32_t {
    // Topic min border, not use for real topic
    TOPIC_MIN = 0,
    // Start project distributed screen
    TOPIC_START_DSCREEN = 1,
    // Publish Sink Project Window Info
    TOPIC_SINK_PROJECT_WINDOW_INFO = 2,
    // Stop distributed screen project
    TOPIC_STOP_DSCREEN = 3,
    // publish device offline message
    TOPIC_DEV_OFFLINE = 4,
    // publish low latency message
    TOPIC_LOW_LATENCY = 5,
    // Topic init DHMS is ready
    TOPIC_INIT_DHMS_READY = 6,
    // Topic for physical input devices plugin event
    TOPIC_PHY_DEV_PLUGIN = 7,
    // Topic max border, not use for real topic
    TOPIC_MAX = 8
};

class IPublisherListener : public IRemoteBroker {
public:
    virtual void OnMessage(const DHTopic topic, const std::string& message) = 0;

    enum class Message : uint32_t {
        ON_MESSAGE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.DistributedHardware.DistributedHardwareFwk.IPublisherListener");
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IPUBLISHER_LISTENER_H