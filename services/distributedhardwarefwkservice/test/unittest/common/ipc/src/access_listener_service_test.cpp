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

#include "access_listener_service.h"
#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class AccessListenerServiceTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(AccessListenerServiceTest, test_on_request_hardware_access_success, TestSize.Level1)
{
    auto service = std::make_shared<AccessListenerService>();
    AuthDeviceInfo info;
    info.networkId = "test_network_id";
    info.deviceName = "test_device_name";
    info.deviceType = 1;
    EXPECT_NO_FATAL_FAILURE(service->OnRequestHardwareAccess("test_request_id", info, DHType::CAMERA, "test_pkg_name"));
}