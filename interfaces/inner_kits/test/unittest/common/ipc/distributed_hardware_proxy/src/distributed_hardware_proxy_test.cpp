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

#include "distributed_hardware_proxy_test.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DistributedHardwareProxyTest::SetUpTestCase()
{
}

void DistributedHardwareProxyTest::TearDownTestCase()
{
}

void DistributedHardwareProxyTest::SetUp()
{
    sptr<IRemoteObject> impl = nullptr;
    hardwareProxy_ = std::make_shared<DistributedHardwareProxy>(impl);
}

void DistributedHardwareProxyTest::TearDown()
{
    hardwareProxy_ = nullptr;
}

/**
 * @tc.name: RegisterPublisherListener_001
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterPublisherListener_001, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = hardwareProxy_->RegisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL, ret);
}

/**
 * @tc.name: RegisterAbilityListener_002
 * @tc.desc: Verify the RegisterAbilityListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterAbilityListener_002, TestSize.Level0)
{
    int32_t invalid = 7;
    DHTopic topic = static_cast<DHTopic>(invalid);
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->RegisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: RegisterAbilityListener_003
 * @tc.desc: Verify the RegisterAbilityListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, RegisterAbilityListener_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->RegisterPublisherListener(topic, listener);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_001
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_001, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = nullptr;
    int32_t ret = hardwareProxy_->UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_PUBLISHER_LISTENER_IS_NULL, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_002
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_002, TestSize.Level0)
{
    int32_t invalid = 7;
    DHTopic topic = static_cast<DHTopic>(invalid);
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->UnregisterPublisherListener(topic, listener);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: UnregisterPublisherListener_003
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, UnregisterPublisherListener_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    sptr<IPublisherListener> listener = new MockIPublisherListener();
    int32_t ret = hardwareProxy_->UnregisterPublisherListener(topic, listener);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: PublishMessage_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_001, TestSize.Level0)
{
    int32_t invalid = 7;
    std::string msg;
    DHTopic topic = static_cast<DHTopic>(invalid);
    int32_t ret = hardwareProxy_->PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: PublishMessage_002
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_002, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    std::string msg;
    int32_t ret = hardwareProxy_->PublishMessage(topic, msg);
    EXPECT_EQ(ERR_DH_FWK_SERVICE_REMOTE_IS_NULL, ret);
}

/**
 * @tc.name: PublishMessage_003
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareProxyTest, PublishMessage_003, TestSize.Level0)
{
    DHTopic topic = DHTopic::TOPIC_STOP_DSCREEN;
    std::string msg = "msg";
    int32_t ret = hardwareProxy_->PublishMessage(topic, msg);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
