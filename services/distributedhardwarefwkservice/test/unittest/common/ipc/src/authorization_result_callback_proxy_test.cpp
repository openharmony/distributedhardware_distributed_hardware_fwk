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

#include "authorization_result_callback_proxy_test.h"
#include "authorization_result_callback_proxy.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

void AuthorizationResultCallbackProxyTest::SetUp() {}
void AuthorizationResultCallbackProxyTest::TearDown() {}

HWTEST_F(AuthorizationResultCallbackProxyTest, test_on_authorization_result_success, TestSize.Level1)
{
    sptr<MockRemoteObject> remote = new MockRemoteObject();
    EXPECT_CALL(*remote, SendRequest(_, _, _, _)).Times(1);
    auto proxy = std::make_shared<AuthorizationResultCallbackProxy>(remote);
    proxy->OnAuthorizationResult("test_network_id", "test_request_id");
}

HWTEST_F(AuthorizationResultCallbackProxyTest, test_on_authorization_result_remote_null, TestSize.Level1)
{
    auto proxy = std::make_shared<AuthorizationResultCallbackProxy>(nullptr);
    proxy->OnAuthorizationResult("test_network_id", "test_request_id");
}