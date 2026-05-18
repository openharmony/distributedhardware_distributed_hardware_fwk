/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_HARDWARE_AUTHORIZATION_RESULT_CALLBACK_PROXY_TEST_H
#define OHOS_DISTRIBUTED_HARDWARE_AUTHORIZATION_RESULT_CALLBACK_PROXY_TEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "iremote_object.h"
#include "iremote_stub.h"

namespace OHOS {
namespace DistributedHardware {
class AuthorizationResultCallbackProxyTest : public testing::Test {
protected:
    void SetUp() override;
    void TearDown() override;
};

class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"") {}
    MOCK_METHOD4(SendRequest, int32_t(uint32_t, MessageParcel&, MessageParcel&, MessageOption&));
    MOCK_METHOD0(GetObjectRefPtr, sptr<IRemoteObject>());
    MOCK_METHOD1(AddDeathRecipient, bool(const sptr<IRemoteObject::DeathRecipient>&));
    MOCK_METHOD1(RemoveDeathRecipient, bool(const sptr<IRemoteObject::DeathRecipient>&));
};
}
}
#endif