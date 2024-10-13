/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "dhfwk_sa_manager_test.h"

#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void DHFWKSAManagerTest::SetUpTestCase()
{
}

void DHFWKSAManagerTest::TearDownTestCase()
{
}

void DHFWKSAManagerTest::SetUp()
{
}

void DHFWKSAManagerTest::TearDown()
{
}

void DHFWKSaStateCallback(bool callback)
{
}

/**
 * @tc.name: RegisterAbilityListener_001
 * @tc.desc: Verify the RegisterAbilityListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, RegisterAbilityListener_001, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.store(false);
    DHFWKSAManager::GetInstance().saListener_ = nullptr;
    DHFWKSAManager::GetInstance().RegisterAbilityListener();
    EXPECT_EQ(false, DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.load());
}

/**
 * @tc.name: RegisterAbilityListener_002
 * @tc.desc: Verify the RegisterAbilityListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, RegisterAbilityListener_002, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.store(true);
    DHFWKSAManager::GetInstance().RegisterAbilityListener();
    EXPECT_EQ(true, DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.load());
}

/**
 * @tc.name: GetDHFWKProxy_001
 * @tc.desc: Verify the GetDHFWKProxy function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, GetDHFWKProxy_001, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.store(false);
    DHFWKSAManager::GetInstance().dhfwkProxy_ = nullptr;
    DHFWKSAManager::GetInstance().GetDHFWKProxy();
    EXPECT_EQ(nullptr, DHFWKSAManager::GetInstance().dhfwkProxy_);
}

/**
 * @tc.name: GetDHFWKProxy_002
 * @tc.desc: Verify the GetDHFWKProxy function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, GetDHFWKProxy_002, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.store(true);
    DHFWKSAManager::GetInstance().dhfwkProxy_ = sptr<MockIDistributedHardware>(new MockIDistributedHardware());
    DHFWKSAManager::GetInstance().GetDHFWKProxy();
    EXPECT_NE(nullptr, DHFWKSAManager::GetInstance().dhfwkProxy_);
}

/**
 * @tc.name: RegisterSAStateCallback_001
 * @tc.desc: Verify the RegisterSAStateCallback function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, RegisterSAStateCallback_001, TestSize.Level0)
{
    DHFWKSAStateCb callback;
    DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.store(true);
    DHFWKSAManager::GetInstance().RegisterSAStateCallback(callback);
    EXPECT_EQ(true, DHFWKSAManager::GetInstance().isSubscribeDHFWKSAChangeListener_.load());
}

/**
 * @tc.name: OnAddSystemAbility_001
 * @tc.desc: Verify the OnAddSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, OnAddSystemAbility_001, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().saListener_ =
        sptr<DHFWKSAManager::SystemAbilityListener>(new DHFWKSAManager::SystemAbilityListener());
    if (DHFWKSAManager::GetInstance().saListener_ == nullptr) {
        return;
    }
    int32_t systemAbilityId = 1;
    std::string deviceId = "deviceId";
    DHFWKSAManager::GetInstance().saListener_->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_NE(nullptr, DHFWKSAManager::GetInstance().saListener_);
}

/**
 * @tc.name: OnAddSystemAbility_002
 * @tc.desc: Verify the OnAddSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, OnAddSystemAbility_002, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().saListener_ =
        sptr<DHFWKSAManager::SystemAbilityListener>(new DHFWKSAManager::SystemAbilityListener());
    if (DHFWKSAManager::GetInstance().saListener_ == nullptr) {
        return;
    }
    int32_t systemAbilityId = DISTRIBUTED_HARDWARE_SA_ID;
    std::string deviceId = "deviceId";
    DHFWKSAManager::GetInstance().saListener_->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_NE(nullptr, DHFWKSAManager::GetInstance().saListener_);
}

/**
 * @tc.name: OnAddSystemAbility_003
 * @tc.desc: Verify the OnAddSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, OnAddSystemAbility_003, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().saListener_ =
        sptr<DHFWKSAManager::SystemAbilityListener>(new DHFWKSAManager::SystemAbilityListener());
    if (DHFWKSAManager::GetInstance().saListener_ == nullptr) {
        return;
    }
    int32_t systemAbilityId = DISTRIBUTED_HARDWARE_SA_ID;
    std::string deviceId = "deviceId";
    DHFWKSAManager::GetInstance().saStateCallback_ = DHFWKSaStateCallback;
    DHFWKSAManager::GetInstance().saListener_->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_NE(nullptr, DHFWKSAManager::GetInstance().saListener_);
}

/**
 * @tc.name: OnRemoveSystemAbility_001
 * @tc.desc: Verify the OnRemoveSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, OnRemoveSystemAbility_001, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().saListener_ =
        sptr<DHFWKSAManager::SystemAbilityListener>(new DHFWKSAManager::SystemAbilityListener());
    if (DHFWKSAManager::GetInstance().saListener_ == nullptr) {
        return;
    }
    int32_t systemAbilityId = 1;
    std::string deviceId = "deviceId";
    DHFWKSAManager::GetInstance().saListener_->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_NE(nullptr, DHFWKSAManager::GetInstance().saListener_);
}

/**
 * @tc.name: OnRemoveSystemAbility_002
 * @tc.desc: Verify the OnRemoveSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, OnRemoveSystemAbility_002, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().saListener_ =
        sptr<DHFWKSAManager::SystemAbilityListener>(new DHFWKSAManager::SystemAbilityListener());
    if (DHFWKSAManager::GetInstance().saListener_ == nullptr) {
        return;
    }
    int32_t systemAbilityId = DISTRIBUTED_HARDWARE_SA_ID;
    std::string deviceId = "deviceId";
    DHFWKSAManager::GetInstance().saListener_->OnRemoveSystemAbility(systemAbilityId, deviceId);
    EXPECT_NE(nullptr, DHFWKSAManager::GetInstance().saListener_);
}

/**
 * @tc.name: OnRemoveSystemAbility_003
 * @tc.desc: Verify the OnRemoveSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DHFWKSAManagerTest, OnRemoveSystemAbility_003, TestSize.Level0)
{
    DHFWKSAManager::GetInstance().saListener_ =
        sptr<DHFWKSAManager::SystemAbilityListener>(new DHFWKSAManager::SystemAbilityListener());
    if (DHFWKSAManager::GetInstance().saListener_ == nullptr) {
        return;
    }
    int32_t systemAbilityId = DISTRIBUTED_HARDWARE_SA_ID;
    std::string deviceId = "deviceId";
    DHFWKSAManager::GetInstance().saStateCallback_ = DHFWKSaStateCallback;
    DHFWKSAManager::GetInstance().saListener_->OnAddSystemAbility(systemAbilityId, deviceId);
    EXPECT_NE(nullptr, DHFWKSAManager::GetInstance().saListener_);
}
} // namespace DistributedHardware
} // namespace OHOS
