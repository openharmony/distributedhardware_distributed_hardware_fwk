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

#include "access_listener_stub.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing::ext;
using namespace testing;
using namespace OHOS;
using namespace OHOS::DistributedHardware;

class MockAccessListenerStub : public AccessListenerStub {
public:
    MOCK_METHOD4(OnRequestHardwareAccess, void(const std::string&, AuthDeviceInfo, DHType, const std::string&));
};

class AccessListenerStubTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(AccessListenerStubTest, test_on_remote_request_invalid_token, TestSize.Level1)
{
    auto stub = std::make_shared<MockAccessListenerStub>();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"invalid_token");
    int32_t ret = stub->OnRemoteRequest(0, data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}