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

#ifndef OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_TEST_EXT_H
#define OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_TEST_EXT_H

#include "gtest/gtest.h"
#include "component_manager.h"
#include "mock_capability_info_manager.h"
#include "mock_component_loader.h"
#include "mock_device_manager.h"
#include "mock_dh_context.h"
#include "mock_dh_utils_tool.h"
#include "mock_meta_info_manager.h"
#include "mock_version_manager.h"

namespace OHOS {
namespace DistributedHardware {
class ComponentManagerTestExt : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
    std::shared_ptr<MockCapabilityInfoManager> capabilityInfoManager_;
    std::shared_ptr<MockComponentLoader> componentLoader_;
    std::shared_ptr<MockDHContext> dhContext_;
    std::shared_ptr<MockDHUtilTool> utilTool_;
    std::shared_ptr<MockMetaInfoManager> metaInfoManager_;
    std::shared_ptr<MockVersionManager> versionManager_;
    std::shared_ptr<MockDeviceManager> deviceManager_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_HARDWARE_COMPONENT_MANAGER_TEST_EXT_H
