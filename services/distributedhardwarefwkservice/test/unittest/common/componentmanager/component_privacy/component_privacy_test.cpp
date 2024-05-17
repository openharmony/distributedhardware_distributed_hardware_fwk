/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <string>

#include "ability_manager_errors.h"
#include "component_privacy.h"
#include "component_loader.h"
#include "distributed_hardware_errno.h"
#include "idistributed_hardware_sink.h"

using namespace testing::ext;
namespace OHOS {
namespace DistributedHardware {
using namespace std;
namespace {

}
class ComponentPrivacyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<ComponentPrivacy> compPrivacy_ = nullptr;
};

void ComponentPrivacyTest::SetUpTestCase()
{
}

void ComponentPrivacyTest::TearDownTestCase()
{
}

void ComponentPrivacyTest::SetUp()
{
    compPrivacy_ = std::make_shared<ComponentPrivacy>();
}

void ComponentPrivacyTest::TearDown()
{
    compPrivacy_ = nullptr;
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_001, TestSize.Level0)
{
    ResourceEventType type = ResourceEventType::EVENT_TYPE_QUERY_RESOURCE;
    std::string subtype = "mic";
    std::string networkId = "networkId_test";
    bool isSensitive = true;
    bool isSameAccout = true;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, subtype, networkId, isSensitive, isSameAccout);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY, ret);
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_002, TestSize.Level0)
{
    ResourceEventType type = ResourceEventType::EVENT_TYPE_PULL_UP_PAGE;
    std::string subtype = "mic";
    std::string networkId = "networkId_test";
    bool isSensitive = true;
    bool isSameAccout = true;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, subtype, networkId, isSensitive, isSameAccout);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, OnPrivaceResourceMessage_003, TestSize.Level0)
{
    ResourceEventType type = ResourceEventType::EVENT_TYPE_CLOSE_PAGE;
    std::string subtype = "mic";
    std::string networkId = "networkId_test";
    bool isSensitive = true;
    bool isSameAccout = true;
    int32_t ret = compPrivacy_->OnPrivaceResourceMessage(type, subtype, networkId, isSensitive, isSameAccout);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, OnResourceInfoCallback_001, TestSize.Level0)
{
    std::string subtype = "mic";
    std::string networkId = "networkId_test";
    bool isSensitive = true;
    bool isSameAccout = true;
    ComponentLoader::GetInstance().resDescMap_[subtype] = true;
    int32_t ret = compPrivacy_->OnResourceInfoCallback(subtype, networkId, isSensitive, isSameAccout);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY, ret);
}

HWTEST_F(ComponentPrivacyTest, StartPrivacePage_001, TestSize.Level0)
{
    std::string subtype = "mic";
    std::string networkId = "networkId_test";
    int32_t ret = compPrivacy_->StartPrivacePage(subtype, networkId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    subtype = "camera";
    ret = compPrivacy_->StartPrivacePage(subtype, networkId);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, StopPrivacePage_001, TestSize.Level0)
{
    std::string subtype = "mic";
    int32_t ret = compPrivacy_->StopPrivacePage(subtype);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
    subtype = "camera";
    ret = compPrivacy_->StopPrivacePage(subtype);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentPrivacyTest, DeviceTypeToString_001, TestSize.Level0)
{
    uint16_t deviceTypeId = 0x08;
    std::string ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("camera", ret);

    deviceTypeId = 0x0A;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("audio", ret);

    deviceTypeId = 0x0C;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("pc", ret);

    deviceTypeId = 0x0E;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("phone", ret);

    deviceTypeId = 0x11;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("pad", ret);

    deviceTypeId = 0x6D;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("watch", ret);

    deviceTypeId = 0x83;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("car", ret);

    deviceTypeId = 0x9C;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("tv", ret);

    deviceTypeId = 0xA02;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("display", ret);

    deviceTypeId = 0xA2F;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("2in1", ret);

    deviceTypeId = 0x00;
    ret = compPrivacy_->DeviceTypeToString(deviceTypeId);
    EXPECT_EQ("unknown", ret);
}
}
}