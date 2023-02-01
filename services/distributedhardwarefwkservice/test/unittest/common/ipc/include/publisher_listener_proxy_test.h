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

#ifndef OHOS_PUBLISHER_LISTENER_PROXY_TEST_H
#define OHOS_PUBLISHER_LISTENER_PROXY_TEST_H

#include <string>
#include <memory>
#include <gtest/gtest.h>

#define private public
#include "publisher_listener_proxy.h"
#undef private
#include "distributed_hardware_errno.h"

namespace OHOS {
namespace DistributedHardware {
class PublisherListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<PublisherListenerProxy> proxy_ = nullptr;
};

class MockIRemoteObject : public IRemoteObject {
public:
sptr<IRemoteObject> AsObject()
{
    return nullptr;
}

void OnMessage(const DHTopic topic, const std::string& message)
{
    (void)topic;
    (void)message;
}

int32_t GetObjectRefCount()
{
    return DH_FWK_SUCCESS;
}

int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    (void)code;
    (void)data;
    (void)reply;
    (void)option;
    return DH_FWK_SUCCESS;
}

bool AddDeathRecipient(const sptr<DeathRecipient> &recipient)
{
    (void)recipient;
    return true;
}

bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient)
{
    (void)recipient;
    return true;
}

int Dump(int fd, const std::vector<std::u16string> &args)
{
    (void)fd;
    (void)args;
    return DH_FWK_SUCCESS;
}
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_PUBLISHER_LISTENER_PROXY_TEST_H