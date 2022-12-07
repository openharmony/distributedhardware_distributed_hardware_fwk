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

#include "publisher_listener_stub_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void PublisherListenerStubTest::SetUpTestCase()
{
}

void PublisherListenerStubTest::TearDownTestCase()
{
}

void PublisherListenerStubTest::SetUp()
{
    listenerStub_ = std::make_shared<MockPublisherListenerStub>();
}

void PublisherListenerStubTest::TearDown()
{
    listenerStub_ = nullptr;
}

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(PublisherListenerStubTest, OnRemoteRequest_001, TestSize.Level0)
{
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_EQ(ERR_INVALID_DATA, listenerStub_->OnRemoteRequest(code, data, reply, option));
}
} // namespace DistributedHardware
} // namespace OHOS
