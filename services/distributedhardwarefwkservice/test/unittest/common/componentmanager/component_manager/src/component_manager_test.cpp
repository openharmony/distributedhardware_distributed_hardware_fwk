/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "component_manager_test.h"

#include <chrono>
#include <cerrno>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <vector>

#include "accesstoken_kit.h"
#include "component_disable.h"
#include "component_enable.h"
#include "component_loader.h"
#include "capability_info.h"
#include "capability_info_manager.h"
#include "component_manager.h"
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "local_capability_info_manager.h"
#include "mock_idistributed_hardware_sink.h"
#include "mock_idistributed_hardware_source.h"
#include "meta_capability_info.h"
#include "meta_info_manager.h"
#include "nativetoken_kit.h"
#include "softbus_common.h"
#include "mock_softbus_bus_center.h"
#include "token_setproc.h"
#include "version_info_manager.h"
#include "version_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentManagerTest"

namespace {
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
const std::string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const std::string NAME_CAMERA = "distributed_camera";
const std::string VERSION_1 = "1.0";
const std::string DEV_ID_TEST = "123456";
const std::string DH_ID_TEST = "Camera_0";
const std::string NETWORK_TEST = "nt36a637105409e904d4da83790a4a8";
const std::string UUID_TEST = "bb536a637105409e904d4da78290ab1";
const std::string UDID_TEST = "45da205792dsad47f5444871161c124";
const std::string UDIDHASH_TEST = "12dsds4785df176a134fvd551czf454";
const std::string DH_ATTR_1 = "attr1";
const std::string DEVICE_NAME = "Dev1";
const std::string DH_ID_1 = "Camera_1";
const std::string DH_SUBTYPE_TEST = "camera";
const std::string TEST_SOURCE_VERSION_1 = "2.2";
const std::string TEST_SINK_VERSION_1 = "2.4";
const std::string TEST_DH_VERSION = "3.1";
const std::shared_ptr<CapabilityInfo> CAP_INFO_TEST =
    std::make_shared<CapabilityInfo>(DH_ID_TEST, DEV_ID_TEST, DEVICE_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1,
    DH_SUBTYPE_TEST);
const std::shared_ptr<MetaCapabilityInfo> META_INFO_PTR_TEST = std::make_shared<MetaCapabilityInfo>(
    DH_ID_TEST, DEV_ID_TEST, DEVICE_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1, DH_SUBTYPE_TEST, UDIDHASH_TEST,
    CompVersion{ .sinkVersion = TEST_SINK_VERSION_1 });
}

void ComponentManagerTest::SetUpTestCase(void)
{
    auto ret = mkdir(DATABASE_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0) {
        DHLOGE("mkdir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }
}

void ComponentManagerTest::TearDownTestCase(void)
{
    auto ret = remove(DATABASE_DIR.c_str());
    if (ret != 0) {
        DHLOGE("remove dir failed, path: %{public}s, errno : %{public}d", DATABASE_DIR.c_str(), errno);
    }
}

void ComponentManagerTest::SetUp()
{
    ComponentManager::GetInstance().compSource_.clear();
    ComponentManager::GetInstance().compSink_.clear();
    uint64_t tokenId;
    const char *perms[2];
    perms[0] = OHOS_PERMISSION_DISTRIBUTED_SOFTBUS_CENTER;
    perms[1] = OHOS_PERMISSION_DISTRIBUTED_DATASYNC;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dsoftbus_service",
        .aplStr = "system_core",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void ComponentManagerTest::TearDown()
{
    ComponentManager::GetInstance().compSource_.clear();
    ComponentManager::GetInstance().compSink_.clear();
}

/**
 * @tc.name: init_test_001
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, init_test_001, TestSize.Level0)
{
    ComponentManager::GetInstance().compMonitorPtr_ = std::make_shared<ComponentMonitor>();
    auto ret = ComponentManager::GetInstance().Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: init_test_002
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, init_test_002, TestSize.Level0)
{
    ComponentManager::GetInstance().compMonitorPtr_ = nullptr;
    auto ret = ComponentManager::GetInstance().Init();
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_MONITOR_NULL, ret);
}

/**
 * @tc.name: unInit_test_001
 * @tc.desc: Verify the UnInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, unInit_test_001, TestSize.Level0)
{
    ComponentManager::GetInstance().cameraCompPrivacy_ = std::make_shared<ComponentPrivacy>();
    ComponentManager::GetInstance().cameraCompPrivacy_->SetPageFlagTrue();
    auto ret = ComponentManager::GetInstance().UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: init_compSource_test_001
 * @tc.desc: Verify the InitCompSource
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, init_compSource_test_001, TestSize.Level0)
{
    ComponentLoader::GetInstance().Init();
    ComponentManager::GetInstance().compSource_.clear();
    auto ret = ComponentManager::GetInstance().InitCompSource(DHType::INPUT);
    EXPECT_NE(ret, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: init_compSink_test_001
 * @tc.desc: Verify the InitCompSink
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, init_compSink_test_001, TestSize.Level0)
{
    ComponentLoader::GetInstance().Init();
    ComponentManager::GetInstance().compSink_.clear();
    auto ret = ComponentManager::GetInstance().InitCompSink(DHType::INPUT);
    EXPECT_NE(ret, ComponentManager::GetInstance().compSink_.empty());
}

/**
 * @tc.name: get_enableparam_test_001
 * @tc.desc: Verify the GetEnableParam
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, get_enableparam_test_001, TestSize.Level0)
{
    DeviceInfo devInfo { "", "", "", "", "", "", 0 };
    auto info = std::make_unique<MockDmDeviceInfo>();
    MockGetLocalNodeDeviceInfo(DH_FWK_PKG_NAME.c_str(), info.get());
    devInfo.uuid = GetUUIDByDm(info->networkId);
    devInfo.udid = GetUDIDByDm(info->networkId);
    devInfo.deviceId = GetDeviceIdByUUID(devInfo.uuid);

    DHContext::GetInstance().AddOnlineDevice(devInfo.udid, devInfo.uuid, info->networkId);

    const std::shared_ptr<CapabilityInfo> CAP_INFO_1 =
        std::make_shared<CapabilityInfo>(DH_ID_1, devInfo.deviceId, DEVICE_NAME,
        TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1, DH_SUBTYPE_TEST);

    CapabilityInfoManager::GetInstance()->Init();
    std::vector<std::shared_ptr<CapabilityInfo>> resInfos { CAP_INFO_1 };
    CapabilityInfoManager::GetInstance()->AddCapability(resInfos);

    CompVersion compVersions1 = {
        .name = NAME_CAMERA,
        .dhType = DHType::CAMERA,
        .handlerVersion = VERSION_1,
        .sourceVersion = VERSION_1,
        .sinkVersion = VERSION_1
    };
    VersionInfo verInfo1;
    verInfo1.deviceId = devInfo.deviceId;
    verInfo1.dhVersion = VERSION_1;
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions1.dhType, compVersions1));

    VersionInfoManager::GetInstance()->Init();
    VersionInfoManager::GetInstance()->AddVersion(verInfo1);

    EnableParam param;
    auto ret = ComponentManager::GetInstance().GetEnableParam(info->networkId, devInfo.uuid,
        DH_ID_1, DHType::CAMERA, param);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED);
}

/**
 * @tc.name: get_sinkversionfromvermgr_test_001
 * @tc.desc: Verify the GetVersionFromVerMgr
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, get_sinkversion_fromvermgr_test_001, TestSize.Level0)
{
    CompVersion compVersions1 = {
        .name = NAME_CAMERA,
        .dhType = DHType::CAMERA,
        .handlerVersion = VERSION_1,
        .sourceVersion = VERSION_1,
        .sinkVersion = VERSION_1
    };
    DHVersion dhVersion;
    dhVersion.uuid = UUID_TEST;
    dhVersion.dhVersion = VERSION_1;
    dhVersion.compVersions.insert(std::pair<DHType, CompVersion>(compVersions1.dhType, compVersions1));

    VersionManager::GetInstance().AddDHVersion(UUID_TEST, dhVersion);
    std::string sinkVersion;
    auto ret = ComponentManager::GetInstance().GetVersionFromVerMgr(UUID_TEST, DHType::CAMERA, sinkVersion, true);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: GetDHType_test_001
 * @tc.desc: Verify the GetDHType
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, GetDHType_test_001, TestSize.Level0)
{
    std::string uuid = "distribueted_111222333";
    std::string dhId = "camera_1234567489";
    auto ret = ComponentManager::GetInstance().GetDHType(uuid, dhId);
    EXPECT_EQ(DHType::UNKNOWN, ret);
}

/**
 * @tc.name: UnInit_001
 * @tc.desc: Verify the UnInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, UnInit_001, TestSize.Level0)
{
    ComponentManager::GetInstance().compMonitorPtr_ = nullptr;
    int32_t ret = ComponentManager::GetInstance().UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: StartSource_001
 * @tc.desc: Verify the StartSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSource_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    auto ret = ComponentManager::GetInstance().StartSource(dhType);
    EXPECT_EQ(true, ret.empty());
    ComponentManager::GetInstance().compSource_.clear();
}

/**
 * @tc.name: StartSource_002
 * @tc.desc: Verify the StartSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSource_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    auto ret = ComponentManager::GetInstance().StartSource(DHType::AUDIO);
    EXPECT_EQ(true, ret.empty());

    ret = ComponentManager::GetInstance().StartSource(dhType);
    EXPECT_EQ(true, ret.empty());
    ComponentManager::GetInstance().compSource_.clear();
}

/**
 * @tc.name: StartSink_001
 * @tc.desc: Verify the StartSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSink_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(dhType, sinkPtr));
    auto ret = ComponentManager::GetInstance().StartSink(dhType);
    EXPECT_EQ(true, ret.empty());
    ComponentManager::GetInstance().compSink_.clear();
}

/**
 * @tc.name: StopSource_001
 * @tc.desc: Verify the StopSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StopSource_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    auto ret = ComponentManager::GetInstance().StopSource(dhType);
    EXPECT_EQ(true, ret.empty());
    ComponentManager::GetInstance().compSource_.clear();
}

/**
 * @tc.name: StopSink_001
 * @tc.desc: Verify the StopSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StopSink_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(dhType, sinkPtr));
    auto ret = ComponentManager::GetInstance().StopSink(dhType);
    EXPECT_EQ(true, ret.empty());
    ComponentManager::GetInstance().compSink_.clear();
}

/**
 * @tc.name: WaitForResult_001
 * @tc.desc: Verify the WaitForResult function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, WaitForResult_001, TestSize.Level0)
{
    ComponentManager::Action action = ComponentManager::Action::START_SINK;
    ActionResult actionsResult;
    bool ret = ComponentManager::GetInstance().WaitForResult(action, actionsResult);
    EXPECT_EQ(true, ret);
}

#ifdef DHARDWARE_CLOSE_UT
    /**
     * @tc.name: InitCompSource_001
     * @tc.desc: Verify the InitCompSource function
     * @tc.type: FUNC
     * @tc.require: AR000GHSJM
     */
    HWTEST_F(ComponentManagerTest, InitCompSource_001, TestSize.Level0)
    {
        bool ret = ComponentManager::GetInstance().InitCompSource(DHType::AUDIO);
        EXPECT_EQ(true, ret);
    }
#endif

/**
 * @tc.name: InitCompSink_001
 * @tc.desc: Verify the InitCompSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, InitCompSink_001, TestSize.Level0)
{
    bool ret = ComponentManager::GetInstance().InitCompSink(DHType::AUDIO);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: Enable_001
 * @tc.desc: Verify the Enable function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, Enable_001, TestSize.Level0)
{
    ComponentManager::GetInstance().compSource_.clear();
    std::string networkId;
    std::string uuid;
    std::string dhId;
    DHType dhType = DHType::CAMERA;
    int32_t ret = ComponentManager::GetInstance().Enable(networkId, uuid, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Enable(NETWORK_TEST, uuid, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Enable(NETWORK_TEST, UUID_TEST, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Enable(networkId, UUID_TEST, DH_ID_TEST, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Enable(networkId, uuid, DH_ID_TEST, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Enable(networkId, UUID_TEST, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: Enable_002
 * @tc.desc: Verify the Enable function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, Enable_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    int32_t ret = ComponentManager::GetInstance().Enable(NETWORK_TEST, UUID_TEST, DH_ID_TEST, DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Enable(NETWORK_TEST, UUID_TEST, DH_ID_TEST, dhType);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED, ret);
    ComponentManager::GetInstance().compSource_.clear();
}

/**
 * @tc.name: Disable_001
 * @tc.desc: Verify the Disable function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, Disable_001, TestSize.Level0)
{
    ComponentManager::GetInstance().compSource_.clear();
    std::string networkId;
    std::string uuid;
    std::string dhId;
    DHType dhType = DHType::CAMERA;
    int32_t ret = ComponentManager::GetInstance().Disable(networkId, uuid, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Disable(NETWORK_TEST, uuid, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Disable(NETWORK_TEST, UUID_TEST, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Disable(networkId, UUID_TEST, DH_ID_TEST, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Disable(networkId, uuid, DH_ID_TEST, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Disable(networkId, UUID_TEST, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: Disable_002
 * @tc.desc: Verify the Disable function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, Disable_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    int32_t ret = ComponentManager::GetInstance().Disable(NETWORK_TEST, UUID_TEST, DH_ID_TEST, DHType::AUDIO);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().Disable(NETWORK_TEST, UUID_TEST, DH_ID_TEST, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    ComponentManager::GetInstance().compSource_.clear();
}

HWTEST_F(ComponentManagerTest, Disable_003, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    std::string udIdTets = "123456789";
    DHContext::GetInstance().AddOnlineDevice(udIdTets, UUID_TEST, udIdTets);
    int32_t ret = ComponentManager::GetInstance().Disable(NETWORK_TEST, UUID_TEST, DH_ID_TEST, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
    DHContext::GetInstance().RemoveOnlineDeviceIdEntryByNetworkId(NETWORK_TEST);
}

/**
 * @tc.name: GetDHType_001
 * @tc.desc: Verify the GetDHType function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetDHType_001, TestSize.Level0)
{
    std::string uuid;
    std::string dhId;
    auto ret = ComponentManager::GetInstance().GetDHType(uuid, dhId);
    EXPECT_EQ(DHType::UNKNOWN, ret);
}

/**
 * @tc.name: GetDHType_002
 * @tc.desc: Verify the GetDHType function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetDHType_002, TestSize.Level0)
{
    std::string key = Sha256(UUID_TEST) + RESOURCE_SEPARATOR + DH_ID_1;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_TEST;
    auto ret = ComponentManager::GetInstance().GetDHType(UUID_TEST, DH_ID_1);
    EXPECT_EQ(DHType::CAMERA, ret);
}

HWTEST_F(ComponentManagerTest, GetEnableCapParam_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    int32_t ret = ComponentManager::GetInstance().GetEnableCapParam("", "", dhType, param, capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetEnableCapParam(NETWORK_TEST, "", dhType, param, capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetEnableCapParam("", UUID_TEST, dhType, param, capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetEnableCapParam_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[DEV_ID_TEST] = CAP_INFO_TEST;
    int32_t ret = ComponentManager::GetInstance().GetEnableCapParam(NETWORK_TEST, UUID_TEST, dhType, param, capability);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED, ret);
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
}

HWTEST_F(ComponentManagerTest, GetEnableMetaParam_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    int32_t ret = ComponentManager::GetInstance().GetEnableMetaParam("", "", dhType, param, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetEnableMetaParam(NETWORK_TEST, "", dhType, param, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetEnableMetaParam("", UUID_TEST, dhType, param, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetEnableMetaParam_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    std::shared_ptr<MetaCapabilityInfo> metaCapPtr = nullptr;
    std::string key = UDIDHASH_TEST + "###" + DH_ID_TEST;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = META_INFO_PTR_TEST;
    auto ret = ComponentManager::GetInstance().GetEnableMetaParam(NETWORK_TEST, UUID_TEST, dhType, param, metaCapPtr);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED, ret);
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

HWTEST_F(ComponentManagerTest, GetCapParam_001, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    int32_t ret = ComponentManager::GetInstance().GetCapParam("", "", capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetCapParam(UUID_TEST, "", capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetCapParam("", DH_ID_TEST, capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetCapParam_002, TestSize.Level0)
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    std::string deviceId = "123456789";
    std::string key = deviceId + "###" + DH_ID_TEST;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capability;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capability;
    int32_t ret = ComponentManager::GetInstance().GetCapParam(UUID_TEST, DH_ID_TEST, capability);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND, ret);

    deviceId = Sha256(UUID_TEST);
    key = deviceId + "###" + DH_ID_TEST;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capability;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capability;
    ret = ComponentManager::GetInstance().GetCapParam(UUID_TEST, DH_ID_TEST, capability);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capability;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_TEST;
    ret = ComponentManager::GetInstance().GetCapParam(UUID_TEST, DH_ID_TEST, capability);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_TEST;
    ret = ComponentManager::GetInstance().GetCapParam(UUID_TEST, DH_ID_TEST, capability);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(ComponentManagerTest, GetMetaParam_001, TestSize.Level0)
{
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = nullptr;
    int32_t ret = ComponentManager::GetInstance().GetMetaParam("", "", metaCapInfo);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetMetaParam(UUID_TEST, "", metaCapInfo);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetMetaParam("", DH_ID_TEST, metaCapInfo);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetMetaParam_002, TestSize.Level0)
{
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = nullptr;
    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, NETWORK_TEST);
    std::string key = UDIDHASH_TEST + "###" + DH_ID_TEST;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapInfo;
    int32_t ret = ComponentManager::GetInstance().GetMetaParam(UUID_TEST, DH_ID_TEST, metaCapInfo);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND, ret);
    MetaInfoManager::GetInstance()->globalMetaInfoMap_.clear();
}

/**
 * @tc.name: GetEnableParam_001
 * @tc.desc: Verify the GetEnableParam function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetEnableParam_001, TestSize.Level0)
{
    std::string networkId;
    std::string uuid;
    std::string dhId;
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    int32_t ret = ComponentManager::GetInstance().GetEnableParam(networkId, uuid, dhId, dhType, param);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED, ret);
}

/**
 * @tc.name: GetEnableParam_002
 * @tc.desc: Verify the GetEnableParam function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetEnableParam_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    int32_t ret = ComponentManager::GetInstance().GetEnableParam(NETWORK_TEST, UUID_TEST, DH_ID_1, dhType, param);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED, ret);
}

/**
 * @tc.name: GetVersionFromVerMgr_001
 * @tc.desc: Verify the GetVersionFromVerMgr function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetVersionFromVerMgr_001, TestSize.Level0)
{
    std::string uuid;
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersionFromVerMgr(uuid, dhType, sinkVersion, true);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: GetVersionFromVerMgr_002
 * @tc.desc: Verify the GetVersionFromVerMgr function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetVersionFromVerMgr_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    VersionManager::GetInstance().dhVersions_.clear();
    int32_t ret = ComponentManager::GetInstance().GetVersionFromVerMgr(UUID_TEST, dhType, sinkVersion, true);
    EXPECT_EQ(ret, ERR_DH_FWK_VERSION_DEVICE_ID_NOT_EXIST);
}

/**
 * @tc.name: GetVersionFromVerMgr_003
 * @tc.desc: Verify the GetVersionFromVerMgr function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetVersionFromVerMgr_003, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    DHVersion dhVersion;
    std::string sinkVersion;
    CompVersion cVs {NAME_CAMERA, dhType, VERSION_1, TEST_SOURCE_VERSION_1, TEST_SINK_VERSION_1};
    dhVersion.uuid = UUID_TEST;
    dhVersion.dhVersion = TEST_DH_VERSION;
    dhVersion.compVersions.insert(std::make_pair(cVs.dhType, cVs));
    VersionManager::GetInstance().AddDHVersion(dhVersion.uuid, dhVersion);
    int32_t ret = ComponentManager::GetInstance().GetVersionFromVerMgr(UUID_TEST, dhType, sinkVersion, true);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: GetVersionFromVerInfoMgr_001
 * @tc.desc: Verify the GetVersionFromVerInfoMgr function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetVersionFromVerInfoMgr_001, TestSize.Level0)
{
    std::string uuid;
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersionFromVerInfoMgr(uuid, dhType, sinkVersion, true);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: GetVersionFromVerInfoMgr_002
 * @tc.desc: Verify the GetVersionFromVerInfoMgr function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetVersionFromVerInfoMgr_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    VersionInfoManager::GetInstance()->dbAdapterPtr_ = nullptr;
    int32_t ret = ComponentManager::GetInstance().GetVersionFromVerInfoMgr(UUID_TEST, dhType, sinkVersion, true);
    EXPECT_EQ(ret, ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL);
}

/**
 * @tc.name: GetSinkVersion_001
 * @tc.desc: Verify the GetVersion function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, GetVersion_001, TestSize.Level0)
{
    std::string uuid;
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersion(uuid, dhType, sinkVersion, true);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetVersion_002, TestSize.Level0)
{
    std::string uuid = "123456798";
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersion(uuid, dhType, sinkVersion, true);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(ComponentManagerTest, GetVersion_003, TestSize.Level0)
{
    std::string uuid = "123456798";
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersion(UUID_TEST, dhType, sinkVersion, true);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: UpdateVersionCache_001
 * @tc.desc: Verify the UpdateVersionCache function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, UpdateVersionCache_001, TestSize.Level0)
{
    std::string uuid;
    VersionInfo versionInfo;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateVersionCache(uuid, versionInfo););

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateVersionCache(UUID_TEST, versionInfo));
}

/**
 * @tc.name: DumpLoadedCompsource_001
 * @tc.desc: Verify the DumpLoadedCompsource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, DumpLoadedCompsource_001, TestSize.Level0)
{
    std::set<DHType> compSourceType;
    ComponentManager::GetInstance().DumpLoadedCompsource(compSourceType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

HWTEST_F(ComponentManagerTest, DumpLoadedCompsink_001, TestSize.Level0)
{
    std::set<DHType> compSinkType;
    ComponentManager::GetInstance().DumpLoadedCompsink(compSinkType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: Recover_001
 * @tc.desc: Verify the Recover function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, Recover_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    ComponentManager::GetInstance().Recover(dhType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: DoRecover_001
 * @tc.desc: Verify the DoRecover function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, DoRecover_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    ComponentManager::GetInstance().DoRecover(dhType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: ReStartSA_001
 * @tc.desc: Verify the ReStartSA function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, ReStartSA_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    ComponentManager::GetInstance().ReStartSA(dhType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: RecoverDistributedHardware_001
 * @tc.desc: Verify the RecoverDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, RecoverDistributedHardware_001, TestSize.Level0)
{
    CompVersion compVersion;
    compVersion.sinkVersion = "1.0";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = std::make_shared<MetaCapabilityInfo>(
        "", "", "devName_test", TEST_DEV_TYPE_PAD, DHType::CAMERA, "attrs_test", "subtype", "", compVersion);
    DHType dhType = DHType::CAMERA;
    std::string udidHash = Sha256(UDID_TEST);
    std::string key = udidHash + "###" + DH_ID_TEST;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapInfo;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().RecoverDistributedHardware(dhType));

    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, "");
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = META_INFO_PTR_TEST;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().RecoverDistributedHardware(dhType));

    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, NETWORK_TEST);
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = META_INFO_PTR_TEST;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().RecoverDistributedHardware(dhType));
}

/**
 * @tc.name: RetryGetEnableParam_001
 * @tc.desc: Verify the RetryGetEnableParam function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, RetryGetEnableParam_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    DHContext::GetInstance().devIdEntrySet_.clear();
    auto ret = ComponentManager::GetInstance().RetryGetEnableParam(NETWORK_TEST, UUID_TEST, DH_ID_1, dhType, param);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_ENABLE_FAILED);
}

/**
 * @tc.name: IsIdenticalAccount_001
 * @tc.desc: Verify the IsIdenticalAccount function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, IsIdenticalAccount_001, TestSize.Level0)
{
    auto ret = ComponentManager::GetInstance().IsIdenticalAccount("");
    EXPECT_EQ(ret, false);

    ret = ComponentManager::GetInstance().IsIdenticalAccount(NETWORK_TEST);
    EXPECT_EQ(ret, false);
}

HWTEST_F(ComponentManagerTest, OnUnregisterResult_001, TestSize.Level0)
{
    std::string networkId = "networkId_test";
    std::string dhId = "dhId_test";
    int32_t status = DH_FWK_SUCCESS;
    std::string data = "data_test";
    auto compDisable = std::make_shared<ComponentDisable>();
    auto ret = compDisable->OnUnregisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    status = ERR_DH_FWK_COMPONENT_UNREGISTER_FAILED;
    ret = compDisable->OnUnregisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_UNREGISTER_FAILED);
}

HWTEST_F(ComponentManagerTest, OnRegisterResult_001, TestSize.Level0)
{
    std::string networkId = "networkId_test";
    std::string dhId = "dhId_test";
    int32_t status = DH_FWK_SUCCESS;
    std::string data = "data_test";
    auto compEnable = std::make_shared<ComponentEnable>();
    auto ret = compEnable->OnRegisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    status = ERR_DH_FWK_COMPONENT_UNREGISTER_FAILED;
    ret = compEnable->OnRegisterResult(networkId, dhId, status, data);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_UNREGISTER_FAILED);
}

HWTEST_F(ComponentManagerTest, UpdateBusinessState_001, TestSize.Level0)
{
    BusinessState state = BusinessState::UNKNOWN;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState("", "", state));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(NETWORK_TEST, "", state));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState("", DH_ID_TEST, state));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(NETWORK_TEST, DH_ID_TEST, state));
}

HWTEST_F(ComponentManagerTest, UpdateBusinessState_002, TestSize.Level0)
{
    BusinessState state = BusinessState::IDLE;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(NETWORK_TEST, DH_ID_TEST, state));

    TaskParam taskParam = {
        .networkId = NETWORK_TEST,
        .uuid = UUID_TEST,
        .udid = UDID_TEST,
        .dhId = DH_ID_TEST,
        .dhType = DHType::CAMERA,
    };
    ComponentManager::GetInstance().SaveNeedRefreshTask(taskParam);
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(NETWORK_TEST, DH_ID_TEST, state));
}

HWTEST_F(ComponentManagerTest, QueryBusinessState_001, TestSize.Level0)
{
    BusinessState ret = ComponentManager::GetInstance().QueryBusinessState("", "");
    EXPECT_EQ(BusinessState::UNKNOWN, ret);

    ret = ComponentManager::GetInstance().QueryBusinessState(UUID_TEST, "");
    EXPECT_EQ(BusinessState::UNKNOWN, ret);

    ret = ComponentManager::GetInstance().QueryBusinessState("", DH_ID_TEST);
    EXPECT_EQ(BusinessState::UNKNOWN, ret);
}

HWTEST_F(ComponentManagerTest, QueryBusinessState_002, TestSize.Level0)
{
    BusinessState ret = ComponentManager::GetInstance().QueryBusinessState(NETWORK_TEST, DH_ID_TEST);
    EXPECT_EQ(BusinessState::IDLE, ret);

    ComponentManager::GetInstance().dhBizStates_.clear();
    ret = ComponentManager::GetInstance().QueryBusinessState(NETWORK_TEST, DH_ID_TEST);
    EXPECT_EQ(BusinessState::UNKNOWN, ret);
}

HWTEST_F(ComponentManagerTest, TriggerFullCapsSync_001, TestSize.Level0)
{
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().TriggerFullCapsSync(""));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().TriggerFullCapsSync(NETWORK_TEST));

    ComponentManager::GetInstance().dhCommToolPtr_ = std::make_shared<DHCommTool>();
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().TriggerFullCapsSync(NETWORK_TEST));
}

HWTEST_F(ComponentManagerTest, GetDHSourceInstance_001, TestSize.Level0)
{
    const IDistributedHardwareSource *sourcePtr = ComponentManager::GetInstance().GetDHSourceInstance(DHType::UNKNOWN);
    EXPECT_EQ(nullptr, sourcePtr);
}

HWTEST_F(ComponentManagerTest, GetDHSourceInstance_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    IDistributedHardwareSource *dhSourcePtr = ComponentManager::GetInstance().GetDHSourceInstance(dhType);
    EXPECT_EQ(nullptr, dhSourcePtr);
    ComponentManager::GetInstance().compSource_.clear();
}
} // namespace DistributedHardware
} // namespace OHOS
