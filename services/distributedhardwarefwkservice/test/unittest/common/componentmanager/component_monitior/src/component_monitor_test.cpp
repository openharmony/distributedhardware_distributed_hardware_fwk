/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "component_monitor_test.h"

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <refbase.h>

#include "component_loader.h"
#include "iservice_registry.h"
#include "system_ability_status_change_stub.h"
#include "single_instance.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
void ComponentMonitorTest::SetUpTestCase(void) {}

void ComponentMonitorTest::TearDownTestCase(void) {}

void ComponentMonitorTest::SetUp()
{
    compMonitorPtr_ = std::make_shared<ComponentMonitor>();
}

void ComponentMonitorTest::TearDown()
{
    compMonitorPtr_->saListeners_.clear();
    compMonitorPtr_ = nullptr;
}

/**
 * @tc.name: AddSAMonitor_001
 * @tc.desc: Verify the AddSAMonitor function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentMonitorTest, AddSAMonitor_001, TestSize.Level0)
{
    int32_t saId = static_cast<int32_t>(DHType::AUDIO);
    compMonitorPtr_->AddSAMonitor(saId);
    EXPECT_EQ(false, compMonitorPtr_->saListeners_.empty());
}

/**
 * @tc.name: AddSAMonitor_002
 * @tc.desc: Verify the AddSAMonitor function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentMonitorTest, AddSAMonitor_002, TestSize.Level0)
{
    int32_t saId = static_cast<int32_t>(DHType::CAMERA);
    sptr<ComponentMonitor::CompSystemAbilityListener> listener = new ComponentMonitor::CompSystemAbilityListener();
    compMonitorPtr_->saListeners_.insert(std::make_pair(saId, listener));
    compMonitorPtr_->AddSAMonitor(saId);
    EXPECT_EQ(false, compMonitorPtr_->saListeners_.empty());
}

/**
 * @tc.name: RemoveSAMonitor_001
 * @tc.desc: Verify the RemoveSAMonitor function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentMonitorTest, RemoveSAMonitor_001, TestSize.Level0)
{
    int32_t saId = static_cast<int32_t>(DHType::GPS);
    compMonitorPtr_->RemoveSAMonitor(saId);
    EXPECT_EQ(true, compMonitorPtr_->saListeners_.empty());
}

/**
 * @tc.name: RemoveSAMonitor_002
 * @tc.desc: Verify the RemoveSAMonitor function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentMonitorTest, RemoveSAMonitor_002, TestSize.Level0)
{
    int32_t saId = static_cast<int32_t>(DHType::CAMERA);
    sptr<ComponentMonitor::CompSystemAbilityListener> listener = new ComponentMonitor::CompSystemAbilityListener();
    compMonitorPtr_->saListeners_.insert(std::make_pair(saId, listener));
    compMonitorPtr_->RemoveSAMonitor(saId);
    EXPECT_EQ(true, compMonitorPtr_->saListeners_.empty());
}

/**
 * @tc.name: OnRemoveSystemAbility_001
 * @tc.desc: Verify the OnRemoveSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentMonitorTest, OnRemoveSystemAbility_001, TestSize.Level0)
{
    sptr<ComponentMonitor::CompSystemAbilityListener> listener = new ComponentMonitor::CompSystemAbilityListener();
    int32_t saId = static_cast<int32_t>(DHType::UNKNOWN);
    std::string deviceId;
    compMonitorPtr_->AddSAMonitor(saId);
    listener->OnRemoveSystemAbility(saId, deviceId);
    EXPECT_EQ(DHType::UNKNOWN, ComponentLoader::GetInstance().GetDHTypeBySrcSaId(saId));
}

/**
 * @tc.name: OnRemoveSystemAbility_002
 * @tc.desc: Verify the OnRemoveSystemAbility function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentMonitorTest, OnRemoveSystemAbility_002, TestSize.Level0)
{
    sptr<ComponentMonitor::CompSystemAbilityListener> listener = new ComponentMonitor::CompSystemAbilityListener();
    int32_t saId = static_cast<int32_t>(DHType::CAMERA);
    std::string deviceId;
    compMonitorPtr_->AddSAMonitor(saId);
    listener->OnRemoveSystemAbility(saId, deviceId);
    EXPECT_EQ(DHType::UNKNOWN, ComponentLoader::GetInstance().GetDHTypeBySrcSaId(saId));
}
} // namespace DistributedHardware
} // namespace OHOS
