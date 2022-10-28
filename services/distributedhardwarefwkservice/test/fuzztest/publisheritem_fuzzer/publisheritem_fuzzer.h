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

#ifndef TEST_PUBLISHER_ITEM_FUZZER_H
#define TEST_PUBLISHER_ITEM_FUZZER_H

#define FUZZ_PROJECT_NAME "publisheritem_fuzzer"

#include <string>

#include "iremote_stub.h"

#include "ipublisher_listener.h"

namespace OHOS {
namespace DistributedHardware {

#define REMOVE_NO_USE_CONSTRUCTOR(className)            \
private:                                                \
    className(const className&) = delete;               \
    className& operator= (const className&) = delete;   \
    className(className&&) = delete;                    \
    className& operator= (className&&) = delete;        \

class MockPublisherItemListener : public IRemoteStub<IPublisherListener> {
public:
    MockPublisherItemListener() = default;
    virtual ~MockPublisherItemListener() = default;
    void OnMessage(const DHTopic topic, const std::string &message) override;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif

