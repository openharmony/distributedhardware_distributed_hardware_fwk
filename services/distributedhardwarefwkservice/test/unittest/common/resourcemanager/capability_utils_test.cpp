/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "capability_utils.h"
#include <gtest/gtest.h>

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

class CapabilityUtilsTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(CapabilityUtilsTest, test_get_capability_key_success, TestSize.Level1)
{
    std::string deviceId = "device_123";
    std::string dhId = "dh_456";
    std::string key = GetCapabilityKey(deviceId, dhId);
    EXPECT_EQ(key, "device_123_dh_456");
}

HWTEST_F(CapabilityUtilsTest, test_get_capability_key_empty_deviceid, TestSize.Level1)
{
    std::string deviceId = "";
    std::string dhId = "dh_456";
    std::string key = GetCapabilityKey(deviceId, dhId);
    EXPECT_EQ(key, "_dh_456");
}

HWTEST_F(CapabilityUtilsTest, test_get_capability_key_empty_dhid, TestSize.Level1)
{
    std::string deviceId = "device_123";
    std::string dhId = "";
    std::string key = GetCapabilityKey(deviceId, dhId);
    EXPECT_EQ(key, "device_123_");
}

HWTEST_F(CapabilityUtilsTest, test_is_cap_key_match_deviceid_success, TestSize.Level1)
{
    std::string key = "device_123_dh_456";
    std::string deviceId = "device_123";
    EXPECT_TRUE(IsCapKeyMatchDeviceId(key, deviceId));
}

HWTEST_F(CapabilityUtilsTest, test_is_cap_key_match_deviceid_not_match, TestSize.Level1)
{
    std::string key = "device_123_dh_456";
    std::string deviceId = "device_789";
    EXPECT_FALSE(IsCapKeyMatchDeviceId(key, deviceId));
}

HWTEST_F(CapabilityUtilsTest, test_is_cap_key_match_deviceid_no_separator, TestSize.Level1)
{
    std::string key = "device_123";
    std::string deviceId = "device_123";
    EXPECT_FALSE(IsCapKeyMatchDeviceId(key, deviceId));
}