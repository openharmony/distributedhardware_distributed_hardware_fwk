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

#include "component_manager_test_ext.h"
#include "distributed_hardware_errno.h"
#include "mock_hardware_handler.h"
#include "mock_hardware_status_listener.h"
#include "mock_idistributed_hardware_sink.h"
#include "mock_idistributed_hardware_source.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {
namespace {
const DeviceInfo VALUABLE_DEVICE_INFO(
    "networkId-1",
    "uuid-1",
    "deviceId-1",
    "udid-1",
    "udidHash-1",
    "deviceName: com.ohos.helloworld",
    1
);
constexpr int32_t CURRENT_DEVICE_UID = 10010;
constexpr int32_t CAMERA_UID = CURRENT_DEVICE_UID;
constexpr int32_t CAMERA_PID = 4083;
constexpr int32_t AUDIO_UID = CURRENT_DEVICE_UID;
constexpr int32_t AUDIO_PID = 4085;
constexpr uint16_t DEV_TYPE_TEST = 14;
const CompVersion VERSION = { .sinkVersion = "1.0", .sourceVersion = "1.0" };
const DHDescriptor CAMERA_DESCRIPTOR = { .id = "camera_1", .dhType = DHType::CAMERA };
const DHDescriptor AUDIO_DESCRIPTOR = { .id = "audio_1", .dhType = DHType::AUDIO };

std::shared_ptr<IDistributedHardwareSink> CreateDHSinkPtrWithSetExpectation()
{
    auto sinkPtr = std::make_shared<MockIDistributedHardwareSink>();
    EXPECT_CALL(*sinkPtr, InitSink(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    EXPECT_CALL(*sinkPtr, ReleaseSink()).WillRepeatedly(Return(DH_FWK_SUCCESS));
    EXPECT_CALL(*sinkPtr, RegisterPrivacyResources(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    return sinkPtr;
}

std::shared_ptr<IDistributedHardwareSource> CreateDHSourcePtrWithSetExpectation()
{
    auto sourcePtr = std::make_shared<MockIDistributedHardwareSource>();
    EXPECT_CALL(*sourcePtr, InitSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    EXPECT_CALL(*sourcePtr, ReleaseSource()).WillRepeatedly(Return(DH_FWK_SUCCESS));
    EXPECT_CALL(*sourcePtr, RegisterDistributedHardwareStateListener(_)).Times(AtLeast(1));
    EXPECT_CALL(*sourcePtr, RegisterDataSyncTriggerListener(_)).Times(AtLeast(1));
    EXPECT_CALL(*sourcePtr, UnregisterDistributedHardwareStateListener()).Times(AtLeast(1));
    EXPECT_CALL(*sourcePtr, UnregisterDataSyncTriggerListener()).Times(AtLeast(1));
    return sourcePtr;
}
} // namespace

void ComponentManagerTestExt::SetUpTestCase(void)
{}

void ComponentManagerTestExt::TearDownTestCase(void)
{}

void ComponentManagerTestExt::SetUp()
{
    auto capabilityInfoManager = ICapabilityInfoManager::GetOrCreateInstance();
    capabilityInfoManager_ = std::static_pointer_cast<MockCapabilityInfoManager>(capabilityInfoManager);
    auto componentLoader = IComponentLoader::GetOrCreateInstance();
    componentLoader_ = std::static_pointer_cast<MockComponentLoader>(componentLoader);
    auto dhContext = IDHContext::GetOrCreateInstance();
    dhContext_ = std::static_pointer_cast<MockDHContext>(dhContext);
    auto utilTool = IDHUtilTool::GetOrCreateInstance();
    utilTool_ = std::static_pointer_cast<MockDHUtilTool>(utilTool);
    auto metaInfoManager = IMetaInfoManager::GetOrCreateInstance();
    metaInfoManager_ = std::static_pointer_cast<MockMetaInfoManager>(metaInfoManager);
    auto versionManager = IVersionManager::GetOrCreateInstance();
    versionManager_ = std::static_pointer_cast<MockVersionManager>(versionManager);
    auto deviceManager = IDeviceManager::GetOrCreateInstance();
    deviceManager_ = std::static_pointer_cast<MockDeviceManager>(deviceManager);
}

void ComponentManagerTestExt::TearDown()
{
    ICapabilityInfoManager::ReleaseInstance();
    IComponentLoader::ReleaseInstance();
    IDHContext::ReleaseInstance();
    IDHUtilTool::ReleaseInstance();
    IMetaInfoManager::ReleaseInstance();
    IVersionManager::ReleaseInstance();
    IDeviceManager::ReleaseInstance();
    capabilityInfoManager_ = nullptr;
    componentLoader_ = nullptr;
    dhContext_ = nullptr;
    metaInfoManager_ = nullptr;
    versionManager_ = nullptr;
    utilTool_ = nullptr;
    deviceManager_ = nullptr;
}

HWTEST_F(ComponentManagerTestExt, EnableSinkAndDisableSink_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);

    auto sinkPtr = CreateDHSinkPtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*componentLoader_, GetSink(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sinkPtr.get()), Return(DH_FWK_SUCCESS)));

    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _)).Times(AtLeast(1));

    auto sinkListener = sptr<MockHDSinkStatusListenerStub>(new (std::nothrow) MockHDSinkStatusListenerStub());
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    ComponentManager::GetInstance().RegisterDHStatusListener(sinkListener, CAMERA_UID, CAMERA_PID);

    auto ret = ComponentManager::GetInstance().EnableSink(CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().EnableSink(CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    auto handler = std::make_shared<MockHardwareHandler>();
    EXPECT_CALL(*componentLoader_, GetHardwareHandler(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(handler.get()), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*componentLoader_, ReleaseSink(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));

    ret = ComponentManager::GetInstance().DisableSink(CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().DisableSink(CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().UnregisterDHStatusListener(sinkListener, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, EnableSinkAndDisableSink_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);

    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _)).Times(AtLeast(1));

    auto sinkPtr = CreateDHSinkPtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*componentLoader_, GetSink(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sinkPtr.get()), Return(DH_FWK_SUCCESS)));

    auto ret = ComponentManager::GetInstance().EnableSink(AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    DHDescriptor newAudio = { .id = "aduio_2", .dhType = DHType::AUDIO };
    int32_t newAudioPid = AUDIO_PID + 1;
    ComponentManager::GetInstance().EnableSink(newAudio, AUDIO_UID, newAudioPid);

    auto handler = std::make_shared<MockHardwareHandler>();
    EXPECT_CALL(*componentLoader_, GetHardwareHandler(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(handler.get()), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*componentLoader_, ReleaseSink(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));

    ComponentManager::GetInstance().DisableSink(newAudio, AUDIO_UID, newAudioPid);
    ret = ComponentManager::GetInstance().DisableSink(AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, EnableSink_failed_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    size_t isDHTypeSupportCallCount = 0;
    size_t getSinkCallCount = 0;
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_))
        .WillRepeatedly(Invoke([&isDHTypeSupportCallCount](DHType) {
            return (isDHTypeSupportCallCount++ > 0) ? true : false;
        }));
    EXPECT_CALL(*componentLoader_, GetSink(_, _))
        .WillRepeatedly(Invoke([&getSinkCallCount](const DHType dhType, IDistributedHardwareSink *&sinkPtr) {
            sinkPtr = nullptr;
            return (getSinkCallCount++ > 0) ? DH_FWK_SUCCESS : ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
        }));
    for (size_t i = 0; i < 3; ++i) {
        auto ret = ComponentManager::GetInstance().EnableSink(CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
        if (isDHTypeSupportCallCount > 0 && getSinkCallCount == 0) {
            EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
        } else {
            EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
        }
    }
}

HWTEST_F(ComponentManagerTestExt, DisableSink_failed_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(false));
    auto ret = ComponentManager::GetInstance().DisableSink(CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTestExt, EnableSourceAndDisableSource_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(capabilityInfoManager_ != nullptr);
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);
    ASSERT_TRUE(utilTool_ != nullptr);

    auto sourceListener = sptr<MockHDSourceStatusListenerStub>(new (std::nothrow) MockHDSourceStatusListenerStub());
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    ComponentManager::GetInstance().RegisterDHStatusListener(VALUABLE_DEVICE_INFO.networkId,
        sourceListener, CAMERA_UID, CAMERA_PID);

    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*componentLoader_, GetSourceSaId(_)).WillRepeatedly(Return(CAMERA_PID));

    auto capabilityInfo = std::make_shared<CapabilityInfo>();
    EXPECT_CALL(*capabilityInfoManager_, GetCapability(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(capabilityInfo), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*capabilityInfoManager_, GetCapabilitiesByDeviceId(_, _))
        .Times(AtLeast(1));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(VERSION), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));

    auto ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId,
        CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId,
        CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    ret = ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId,
        CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId,
        CAMERA_DESCRIPTOR, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    ret = ComponentManager::GetInstance().UnregisterDHStatusListener(VALUABLE_DEVICE_INFO.networkId,
        sourceListener, CAMERA_UID, CAMERA_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, EnableSourceAndDisableSource_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(capabilityInfoManager_ != nullptr);
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);
    ASSERT_TRUE(utilTool_ != nullptr);

    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));

    EXPECT_CALL(*componentLoader_, GetSourceSaId(_)).WillRepeatedly(Return(CAMERA_PID));

    auto capabilityInfo = std::make_shared<CapabilityInfo>();
    EXPECT_CALL(*capabilityInfoManager_, GetCapability(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(capabilityInfo), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*capabilityInfoManager_, GetCapabilitiesByDeviceId(_, _))
        .Times(AtLeast(1));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(VERSION), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));

    auto ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    DHDescriptor newAudio = { .id = "aduio_2", .dhType = DHType::AUDIO };
    int32_t newAudioPid = AUDIO_PID + 1;
    ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId, newAudio, AUDIO_UID, newAudioPid);

    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId, newAudio, AUDIO_UID, newAudioPid);
    ret = ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, EnableSourceAndDisableSource_003, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(capabilityInfoManager_ != nullptr);
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);
    ASSERT_TRUE(utilTool_ != nullptr);

    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));

    EXPECT_CALL(*componentLoader_, GetSourceSaId(_)).WillRepeatedly(Return(CAMERA_PID));

    auto capabilityInfo = std::make_shared<CapabilityInfo>();
    EXPECT_CALL(*capabilityInfoManager_, GetCapability(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(capabilityInfo), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*capabilityInfoManager_, GetCapabilitiesByDeviceId(_, _))
        .Times(AtLeast(1));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(VERSION), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));

    auto ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    int32_t newAudioPid = AUDIO_PID + 1;
    ret = ComponentManager::GetInstance().EnableSource(
        VALUABLE_DEVICE_INFO.networkId, AUDIO_DESCRIPTOR, AUDIO_UID, newAudioPid);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    ComponentManager::GetInstance().DisableSource(
        VALUABLE_DEVICE_INFO.networkId, AUDIO_DESCRIPTOR, AUDIO_UID, newAudioPid);
    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(ERR_DH_FWK_LOADER_SOURCE_UNLOAD));
    ret = ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_SOURCE_UNLOAD);
    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    ret = ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

#ifdef DHARDWARE_CHECK_RESOURCE
HWTEST_F(ComponentManagerTestExt, EnableSourceAndDisableSource_004, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(capabilityInfoManager_ != nullptr);
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);
    ASSERT_TRUE(utilTool_ != nullptr);

    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));

    EXPECT_CALL(*componentLoader_, GetSourceSaId(_)).WillRepeatedly(Return(CAMERA_PID));

    auto capabilityInfo = std::make_shared<CapabilityInfo>(
        "dhid", "devId", "devName", 1, DHType::AUDIO, "dhAttrs", "mic");
    EXPECT_CALL(*capabilityInfoManager_, GetCapability(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(capabilityInfo), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*capabilityInfoManager_, GetCapabilitiesByDeviceId(_, _))
        .Times(AtLeast(1));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(VERSION), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));

    std::map<std::string, bool> resDescMap;
    resDescMap["mic"] = false;
    EXPECT_CALL(*componentLoader_, GetCompResourceDesc()).WillRepeatedly(Return(resDescMap));

    auto ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId, AUDIO_DESCRIPTOR, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}
#endif

HWTEST_F(ComponentManagerTestExt, EnableSource_failed_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);

    size_t isDHTypeSupportCallCount = 0;
    size_t getSourceCallCount = 0;
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_))
        .WillRepeatedly(Invoke([&isDHTypeSupportCallCount](DHType) {
            return (isDHTypeSupportCallCount++ > 0) ? true : false;
        }));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).Times(AtLeast(1));
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(Invoke([&getSourceCallCount](const DHType dhType, IDistributedHardwareSource *&sourcePtr) {
            sourcePtr = nullptr;
            return (getSourceCallCount++ > 0) ? DH_FWK_SUCCESS : ERR_DH_FWK_LOADER_HANDLER_IS_NULL;
        }));
    for (size_t i = 0; i < 3; ++i) {
        auto ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId,
            AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
        if (isDHTypeSupportCallCount > 0 && getSourceCallCount == 0) {
            EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
        } else {
            EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
        }
    }
}

#ifdef DHARDWARE_CHECK_RESOURCE
HWTEST_F(ComponentManagerTestExt, EnableSource_failed_002, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(capabilityInfoManager_ != nullptr);
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);
    ASSERT_TRUE(utilTool_ != nullptr);

    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));

    EXPECT_CALL(*componentLoader_, GetSourceSaId(_)).WillRepeatedly(Return(CAMERA_PID));

    auto capabilityInfo = std::make_shared<CapabilityInfo>();
    EXPECT_CALL(*capabilityInfoManager_, GetCapability(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(capabilityInfo), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*capabilityInfoManager_, GetCapabilitiesByDeviceId(_, _))
        .Times(AtLeast(1));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(VERSION), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));

    auto ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId, AUDIO_DESCRIPTOR, 0, 0);
    EXPECT_EQ(ret, ERR_DH_FWK_RESOURCE_KEY_IS_EMPTY);
}

HWTEST_F(ComponentManagerTestExt, EnableSource_failed_003, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(capabilityInfoManager_ != nullptr);
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);
    ASSERT_TRUE(utilTool_ != nullptr);

    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));

    EXPECT_CALL(*componentLoader_, GetSourceSaId(_)).WillRepeatedly(Return(CAMERA_PID));

    auto capabilityInfo = std::make_shared<CapabilityInfo>(
        "dhid", "devId", "devName", 1, DHType::AUDIO, "dhAttrs", "mic");
    EXPECT_CALL(*capabilityInfoManager_, GetCapability(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(capabilityInfo), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*capabilityInfoManager_, GetCapabilitiesByDeviceId(_, _))
        .Times(AtLeast(1));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(VERSION), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));

    std::map<std::string, bool> resDescMap;
    resDescMap["mic"] = true;
    EXPECT_CALL(*componentLoader_, GetCompResourceDesc()).WillRepeatedly(Return(resDescMap));

    auto ret = ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId, AUDIO_DESCRIPTOR, 0, 0);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_ENABLE_FAILED);
}
#endif

HWTEST_F(ComponentManagerTestExt, DisableSource_failed_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(false));
    auto ret = ComponentManager::GetInstance().DisableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTestExt, EnableSinkAndForceDisableSink_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);

    auto sinkPtr = CreateDHSinkPtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*componentLoader_, GetSink(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sinkPtr.get()), Return(DH_FWK_SUCCESS)));

    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _)).Times(AtLeast(1));

    auto sinkListener = sptr<MockHDSinkStatusListenerStub>(new (std::nothrow) MockHDSinkStatusListenerStub());
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    ComponentManager::GetInstance().RegisterDHStatusListener(sinkListener, AUDIO_UID, AUDIO_PID);

    ComponentManager::GetInstance().EnableSink(AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    DHDescriptor newAudio = { .id = "aduio_2", .dhType = DHType::AUDIO };
    int32_t newAudioPid = AUDIO_PID + 1;
    ComponentManager::GetInstance().EnableSink(newAudio, AUDIO_UID, newAudioPid);

    auto handler = std::make_shared<MockHardwareHandler>();
    EXPECT_CALL(*componentLoader_, GetHardwareHandler(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(handler.get()), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*componentLoader_, ReleaseSink(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));

    auto ret = ComponentManager::GetInstance().ForceDisableSink(newAudio);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().ForceDisableSink(AUDIO_DESCRIPTOR);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().UnregisterDHStatusListener(sinkListener, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, ForceDisableSink_failed_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(false));
    auto ret = ComponentManager::GetInstance().ForceDisableSink(CAMERA_DESCRIPTOR);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTestExt, EnableSourceAndForceDisableSource_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(capabilityInfoManager_ != nullptr);
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(versionManager_ != nullptr);
    ASSERT_TRUE(utilTool_ != nullptr);

    auto sourceListener = sptr<MockHDSourceStatusListenerStub>(new (std::nothrow) MockHDSourceStatusListenerStub());
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    ComponentManager::GetInstance().RegisterDHStatusListener(VALUABLE_DEVICE_INFO.networkId,
        sourceListener, AUDIO_UID, AUDIO_PID);

    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(true));
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*componentLoader_, GetSourceSaId(_)).WillRepeatedly(Return(CAMERA_PID));
    auto capabilityInfo = std::make_shared<CapabilityInfo>();
    EXPECT_CALL(*capabilityInfoManager_, GetCapability(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(capabilityInfo), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*capabilityInfoManager_, GetCapabilitiesByDeviceId(_, _))
        .Times(AtLeast(1));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*dhContext_, GetUUIDByNetworkId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*versionManager_, GetCompVersion(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(VERSION), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));

    ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId,
        AUDIO_DESCRIPTOR, AUDIO_UID, AUDIO_PID);
    DHDescriptor newAudio = { .id = "aduio_2", .dhType = DHType::AUDIO };
    int32_t newAudioPid = AUDIO_PID + 1;
    ComponentManager::GetInstance().EnableSource(VALUABLE_DEVICE_INFO.networkId, newAudio, AUDIO_UID, newAudioPid);

    EXPECT_CALL(*componentLoader_, ReleaseSource(_)).WillRepeatedly(Return(DH_FWK_SUCCESS));
    auto ret = ComponentManager::GetInstance().ForceDisableSource(VALUABLE_DEVICE_INFO.networkId, newAudio);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().ForceDisableSource(VALUABLE_DEVICE_INFO.networkId, AUDIO_DESCRIPTOR);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().UnregisterDHStatusListener(VALUABLE_DEVICE_INFO.networkId,
        sourceListener, AUDIO_UID, AUDIO_PID);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, ForceDisableSource_failed_001, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    EXPECT_CALL(*componentLoader_, IsDHTypeSupport(_)).WillRepeatedly(Return(false));
    auto ret = ComponentManager::GetInstance().ForceDisableSource(VALUABLE_DEVICE_INFO.networkId, CAMERA_DESCRIPTOR);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTestExt, CheckDemandStart_001, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(metaInfoManager_ != nullptr);

    DHType targetType = DHType::CAMERA;
    CompVersion localVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = true,
        .sourceFeatureFilters = { "yes" },
        .sinkSupportedFeatures = { "yes" }
    };
    CompVersion remoteVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = true,
        .sourceFeatureFilters = { "yes" },
        .sinkSupportedFeatures = { "yes" }
    };
    DHVersion dhVersion = {
        .uuid = VALUABLE_DEVICE_INFO.uuid,
        .dhVersion = "1.0",
        .compVersions = {{ targetType, localVersion }}
    };
    auto metaCapabilityInfo = std::make_shared<MetaCapabilityInfo>();
    metaCapabilityInfo->SetCompVersion(remoteVersion);
    MetaCapInfoMap infoMap = {{ VALUABLE_DEVICE_INFO.uuid, metaCapabilityInfo }};
    EXPECT_CALL(*metaInfoManager_, GetMetaDataByDHType(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(infoMap), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*dhContext_, GetUUIDByDeviceId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*componentLoader_, GetLocalDHVersion(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(dhVersion), Return(DH_FWK_SUCCESS)));

    bool isEnableSource = false;
    auto ret = ComponentManager::GetInstance().CheckDemandStart(VALUABLE_DEVICE_INFO.uuid, targetType, isEnableSource);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, CheckDemandStart_002, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(metaInfoManager_ != nullptr);

    size_t getMetaDataByDHTypeCallCount = 0;
    EXPECT_CALL(*metaInfoManager_, GetMetaDataByDHType(_, _))
        .WillRepeatedly(Invoke([&getMetaDataByDHTypeCallCount](const DHType dhType, MetaCapInfoMap &metaInfoMap) {
            MetaCapInfoMap emptyInfo;
            metaInfoMap = emptyInfo;
            return (getMetaDataByDHTypeCallCount++ > 0) ? DH_FWK_SUCCESS : ERR_DH_FWK_COMPONENT_COMPVERSION_NOT_FOUND;
        }));

    for (size_t i = 0; i < 2; ++i) {
        bool isEnableSource = false;
        auto ret = ComponentManager::GetInstance().CheckDemandStart(
            VALUABLE_DEVICE_INFO.uuid, DHType::CAMERA, isEnableSource);
        EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_COMPVERSION_NOT_FOUND);
    }
}

HWTEST_F(ComponentManagerTestExt, CheckDemandStart_004, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(metaInfoManager_ != nullptr);

    CompVersion localVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = true,
        .sourceFeatureFilters = { "yes" },
        .sinkSupportedFeatures = { "yes" }
    };
    CompVersion remoteVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = true,
        .sourceFeatureFilters = { "yes" },
        .sinkSupportedFeatures = { "yes" }
    };
    DHVersion dhVersion = {
        .uuid = VALUABLE_DEVICE_INFO.uuid,
        .dhVersion = "1.0",
        .compVersions = {{ DHType::CAMERA, localVersion }}
    };
    auto metaCapabilityInfo = std::make_shared<MetaCapabilityInfo>();
    metaCapabilityInfo->SetCompVersion(remoteVersion);
    MetaCapInfoMap infoMap = {{ VALUABLE_DEVICE_INFO.uuid, metaCapabilityInfo }};
    EXPECT_CALL(*metaInfoManager_, GetMetaDataByDHType(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(infoMap), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*dhContext_, GetUUIDByDeviceId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*componentLoader_, GetLocalDHVersion(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(dhVersion), Return(DH_FWK_SUCCESS)));

    bool isEnableSource = false;
    auto ret = ComponentManager::GetInstance().CheckDemandStart(
        VALUABLE_DEVICE_INFO.uuid, DHType::AUDIO, isEnableSource);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTestExt, CheckDemandStart_005, testing::ext::TestSize.Level2)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(metaInfoManager_ != nullptr);

    auto metaCapabilityInfo = std::make_shared<MetaCapabilityInfo>();
    metaCapabilityInfo->SetCompVersion(VERSION);
    MetaCapInfoMap infoMap = {{ VALUABLE_DEVICE_INFO.uuid, metaCapabilityInfo }};
    EXPECT_CALL(*metaInfoManager_, GetMetaDataByDHType(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(infoMap), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*dhContext_, GetUUIDByDeviceId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*componentLoader_, GetLocalDHVersion(_)).WillRepeatedly(Return(1));

    bool isEnableSource = false;
    auto ret = ComponentManager::GetInstance().CheckDemandStart(
        VALUABLE_DEVICE_INFO.uuid, DHType::CAMERA, isEnableSource);
    EXPECT_EQ(ret, 1);
}

HWTEST_F(ComponentManagerTestExt, CheckDemandStart_006, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(metaInfoManager_ != nullptr);

    DHType targetType = DHType::CAMERA;
    CompVersion localVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = false
    };
    CompVersion remoteVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = false,
    };
    DHVersion dhVersion = {
        .uuid = VALUABLE_DEVICE_INFO.uuid,
        .dhVersion = "1.0",
        .compVersions = {{ targetType, localVersion }}
    };
    auto metaCapabilityInfo = std::make_shared<MetaCapabilityInfo>();
    metaCapabilityInfo->SetCompVersion(remoteVersion);
    MetaCapInfoMap infoMap = {{ VALUABLE_DEVICE_INFO.uuid, metaCapabilityInfo }};
    EXPECT_CALL(*metaInfoManager_, GetMetaDataByDHType(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(infoMap), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*dhContext_, GetUUIDByDeviceId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*componentLoader_, GetLocalDHVersion(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(dhVersion), Return(DH_FWK_SUCCESS)));

    bool isEnableSource = false;
    auto ret = ComponentManager::GetInstance().CheckDemandStart(VALUABLE_DEVICE_INFO.uuid, targetType, isEnableSource);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, CheckDemandStart_007, testing::ext::TestSize.Level1)
{
    ASSERT_TRUE(componentLoader_ != nullptr);
    ASSERT_TRUE(dhContext_ != nullptr);
    ASSERT_TRUE(metaInfoManager_ != nullptr);

    DHType targetType = DHType::CAMERA;
    CompVersion localVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = true,
    };
    CompVersion remoteVersion = {
        .sourceVersion = "1.0",
        .sinkVersion = "1.0",
        .haveFeature = true,
    };
    DHVersion dhVersion = {
        .uuid = VALUABLE_DEVICE_INFO.uuid,
        .dhVersion = "1.0",
        .compVersions = {{ targetType, localVersion }}
    };
    auto metaCapabilityInfo = std::make_shared<MetaCapabilityInfo>();
    metaCapabilityInfo->SetCompVersion(remoteVersion);
    MetaCapInfoMap infoMap = {{ VALUABLE_DEVICE_INFO.uuid, metaCapabilityInfo }};
    EXPECT_CALL(*metaInfoManager_, GetMetaDataByDHType(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(infoMap), Return(DH_FWK_SUCCESS)));
    EXPECT_CALL(*dhContext_, GetUUIDByDeviceId(_)).WillRepeatedly(Return(VALUABLE_DEVICE_INFO.uuid));
    EXPECT_CALL(*componentLoader_, GetLocalDHVersion(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(dhVersion), Return(DH_FWK_SUCCESS)));

    bool isEnableSource = false;
    auto ret = ComponentManager::GetInstance().CheckDemandStart(VALUABLE_DEVICE_INFO.uuid, targetType, isEnableSource);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, RegisterDHStatusListener_001, testing::ext::TestSize.Level1)
{
    sptr<IHDSinkStatusListener> listener = nullptr;
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    auto ret = ComponentManager::GetInstance().RegisterDHStatusListener(listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, RegisterDHStatusListener_002, testing::ext::TestSize.Level1)
{
    sptr<IHDSourceStatusListener> listener = nullptr;
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    auto ret = ComponentManager::GetInstance().RegisterDHStatusListener(
        VALUABLE_DEVICE_INFO.networkId, listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, UnregisterDHStatusListener_001, testing::ext::TestSize.Level1)
{
    sptr<IHDSinkStatusListener> listener = nullptr;
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    auto ret = ComponentManager::GetInstance().UnregisterDHStatusListener(listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, UnregisterDHStatusListener_002, testing::ext::TestSize.Level1)
{
    sptr<IHDSourceStatusListener> listener = nullptr;
    std::vector<DHType> dhTypeVec;
    EXPECT_CALL(*componentLoader_, GetAllCompTypes(dhTypeVec)).WillRepeatedly(Return());
    auto ret = ComponentManager::GetInstance().UnregisterDHStatusListener(
        VALUABLE_DEVICE_INFO.networkId, listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, RealEnableSource_001, testing::ext::TestSize.Level1)
{
    ComponentManager::DHStatusCtrl statusCtrl;
    ComponentManager::DHStatusEnableInfo enableInfo;
    ComponentManager::DHSourceStatus status;
    auto sourcePtr = CreateDHSourcePtrWithSetExpectation();
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(sourcePtr.get()), Return(DH_FWK_SUCCESS)));
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    auto ret = ComponentManager::GetInstance().RealEnableSource(VALUABLE_DEVICE_INFO.networkId,
        VALUABLE_DEVICE_INFO.uuid, AUDIO_DESCRIPTOR, statusCtrl, enableInfo, status, false);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED);
}

HWTEST_F(ComponentManagerTestExt, RealDisableSource_001, testing::ext::TestSize.Level1)
{
    ComponentManager::DHStatusCtrl statusCtrl;
    ComponentManager::DHStatusEnableInfo enableInfo;
    ComponentManager::DHSourceStatus status;
    auto ret = ComponentManager::GetInstance().RealDisableSource(
        "", VALUABLE_DEVICE_INFO.uuid, AUDIO_DESCRIPTOR, statusCtrl, enableInfo, status);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(ComponentManagerTestExt, UninitCompSource_001, testing::ext::TestSize.Level1)
{
    ComponentManager::DHStatusCtrl statusCtrl;
    ComponentManager::DHStatusEnableInfo enableInfo;
    ComponentManager::DHSourceStatus status;
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(nullptr), Return(1)));
    auto ret = ComponentManager::GetInstance().UninitCompSource(DHType::AUDIO);
    EXPECT_EQ(ret, 1);
    EXPECT_CALL(*componentLoader_, GetSource(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(nullptr), Return(DH_FWK_SUCCESS)));
    ret = ComponentManager::GetInstance().UninitCompSource(DHType::AUDIO);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentManagerTestExt, GetMetaParam_001, testing::ext::TestSize.Level1)
{
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    EXPECT_CALL(*metaInfoManager_, GetMetaCapInfo(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(nullptr), Return(1)));
    auto ret = ComponentManager::GetInstance().GetMetaParam(
        VALUABLE_DEVICE_INFO.uuid, VALUABLE_DEVICE_INFO.udidHash, metaCapPtr);
    EXPECT_EQ(ret, 1);
    EXPECT_CALL(*metaInfoManager_, GetMetaCapInfo(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(nullptr), Return(DH_FWK_SUCCESS)));
    ret = ComponentManager::GetInstance().GetMetaParam(
        VALUABLE_DEVICE_INFO.uuid, VALUABLE_DEVICE_INFO.udidHash, metaCapPtr);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTestExt, GetEnableParam_001, testing::ext::TestSize.Level1)
{
    EnableParam param;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = std::make_shared<MetaCapabilityInfo>();
    DeviceInfo emptyInfo("", "", "", "", "", "", 0);
    EXPECT_CALL(*dhContext_, GetDeviceInfo()).WillRepeatedly(ReturnRef(emptyInfo));
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    EXPECT_CALL(*metaInfoManager_, GetMetaCapInfo(_, _, _))
        .WillRepeatedly(DoAll(SetArgReferee<2>(metaCapPtr), Return(DH_FWK_SUCCESS)));
    auto ret = ComponentManager::GetInstance().GetEnableParam(VALUABLE_DEVICE_INFO.networkId,
        VALUABLE_DEVICE_INFO.uuid, VALUABLE_DEVICE_INFO.udidHash, DHType::CAMERA, param);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED);
}

HWTEST_F(ComponentManagerTestExt, WaitForResult_001, testing::ext::TestSize.Level1)
{
    ActionResult actionsResult;
    std::promise<int32_t> p;
    std::future<int32_t> f = p.get_future();
    std::thread([p = std::move(p), this] () mutable {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }).detach();
    actionsResult.emplace(DHType::CAMERA, f.share());
    auto ret = ComponentManager::GetInstance().WaitForResult(ComponentManager::Action::START_SOURCE, actionsResult);
    EXPECT_EQ(ret, true);
}

HWTEST_F(ComponentManagerTestExt, RecoverAutoEnableSink_001, testing::ext::TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string udidHash = "udidHash-1";
    std::string dhId = "dhId_1";
    std::string deviceId = "deviceId-1";
    CompVersion compVersion{ .sinkVersion = "1.0" };
    std::shared_ptr<MetaCapabilityInfo> dhMetaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, dhType, "attrs_test", "subtype", udidHash, compVersion);
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = dhMetaCapInfo;
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().RecoverAutoEnableSink(dhType));
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(ComponentManagerTestExt, RecoverAutoEnableSink_002, testing::ext::TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string udidHash = "udidHash-1";
    std::string dhId = "dhId_1";
    std::string deviceId = "deviceId-1";
    CompVersion compVersion{ .sinkVersion = "1.0" };
    std::shared_ptr<MetaCapabilityInfo> dhMetaCapInfo = std::make_shared<MetaCapabilityInfo>(
        dhId, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs_test", "subtype", udidHash, compVersion);
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = dhMetaCapInfo;
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().RecoverAutoEnableSink(dhType));
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(ComponentManagerTestExt, RecoverAutoEnableSink_003, testing::ext::TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string udidHash = "udidHash-1";
    std::string dhId = "dhId_test";
    std::shared_ptr<MetaCapabilityInfo> dhMetaCapInfo = nullptr;
    std::string key = udidHash + "###" + dhId;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = dhMetaCapInfo;
    EXPECT_CALL(*utilTool_, GetLocalDeviceInfo()).WillRepeatedly(Return(VALUABLE_DEVICE_INFO));
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().RecoverAutoEnableSink(dhType));
}
} // namespace DistributedHardware
} // namespace OHOS
