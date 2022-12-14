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

#include "distributed_hardware_stub_test.h"

#include <memory>

#include "iremote_stub.h"
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DistributedHardwareStubTest::SetUpTestCase(void) {}

void DistributedHardwareStubTest::TearDownTestCase(void) {}

void DistributedHardwareStubTest::SetUp()
{
    stubTest_ = std::make_shared<MockDistributedHardwareStub>();
}

void DistributedHardwareStubTest::TearDown()
{
    stubTest_ = nullptr;
}

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: Verify the OnRemoteRequest function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareStubTest, OnRemoteRequest_001, TestSize.Level0)
{
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_NE(DH_FWK_SUCCESS, stubTest_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: RegisterPublisherListenerInner_001
 * @tc.desc: Verify the RegisterPublisherListenerInner function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareStubTest, RegisterPublisherListenerInner_001, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_NE(DH_FWK_SUCCESS, stubTest_->RegisterPublisherListenerInner(data, reply));
}

/**
 * @tc.name: UnregisterPublisherListenerInner_001
 * @tc.desc: Verify the UnregisterPublisherListenerInner function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareStubTest, UnregisterPublisherListenerInner_001, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_NE(DH_FWK_SUCCESS, stubTest_->UnregisterPublisherListenerInner(data, reply));
}

/**
 * @tc.name: PublishMessageInner_001
 * @tc.desc: Verify the PublishMessageInner function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareStubTest, PublishMessageInner_001, TestSize.Level0)
{
    MessageParcel data;
    MessageParcel reply;
    EXPECT_NE(DH_FWK_SUCCESS, stubTest_->PublishMessageInner(data, reply));
}
} // namespace DistributedHardware
} // namespace OHOS
