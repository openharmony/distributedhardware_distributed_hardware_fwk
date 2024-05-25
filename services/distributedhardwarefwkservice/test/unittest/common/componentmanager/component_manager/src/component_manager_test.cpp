/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "mock_idistributed_hardware_sink.h"
#include "mock_idistributed_hardware_source.h"
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
    auto ret = ComponentManager::GetInstance().Init();
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_INIT_SOURCE_FAILED, ret);
}

/**
 * @tc.name: init_test_002
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, init_test_002, TestSize.Level0)
{
    MockIDistributedHardwareSource cameraSource;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::CAMERA, &cameraSource));

    auto ret = ComponentManager::GetInstance().Init();
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_INIT_SINK_FAILED, ret);
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
 * @tc.name: enable_test_005
 * @tc.desc: Verify the Enable for Multi-thread
 * @tc.type: FUNC
 * @tc.require: AR000GHSK7
 */
HWTEST_F(ComponentManagerTest, Enable_test_005, TestSize.Level0)
{
    std::string networkId = "";
    std::string uuid = "";
    std::string dhId = "";
    ComponentManager::GetInstance().compSource_.clear();
    auto ret = ComponentManager::GetInstance().Enable(networkId, uuid, dhId, DHType::CAMERA);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

/**
 * @tc.name: disable_test_005
 * @tc.desc: Verify the Disable for Multi-thread
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, Disable_test_005, TestSize.Level0)
{
    std::string networkId = "";
    std::string uuid = "";
    std::string dhId = "";
    ComponentManager::GetInstance().compSource_.clear();
    auto ret = ComponentManager::GetInstance().Disable(networkId, uuid, dhId, DHType::CAMERA);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
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
    auto ret = ComponentManager::GetInstance().InitCompSource();
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
    auto ret = ComponentManager::GetInstance().InitCompSink();
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
    DeviceInfo devInfo { "", "", "", "", 0 };
    auto info = std::make_unique<MockNodeBasicInfo>();
    MockGetLocalNodeDeviceInfo(DH_FWK_PKG_NAME.c_str(), info.get());
    devInfo.uuid = GetUUIDBySoftBus(info->networkId);
    devInfo.deviceId = GetDeviceIdByUUID(devInfo.uuid);

    DHContext::GetInstance().AddOnlineDevice(info->networkId, devInfo.uuid);

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
    auto ret = ComponentManager::GetInstance().StartSource();
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: StartSource_002
 * @tc.desc: Verify the StartSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSource_002, TestSize.Level0)
{
    DHType dhType = DHType::AUDIO;
    auto ret = ComponentManager::GetInstance().StartSource(dhType);
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: StartSink_001
 * @tc.desc: Verify the StartSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSink_001, TestSize.Level0)
{
    auto ret = ComponentManager::GetInstance().StartSink();
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: StartSink_002
 * @tc.desc: Verify the StartSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSink_002, TestSize.Level0)
{
    DHType dhType = DHType::AUDIO;
    auto ret = ComponentManager::GetInstance().StartSink(dhType);
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: StopSource_001
 * @tc.desc: Verify the StopSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StopSource_001, TestSize.Level0)
{
    auto ret = ComponentManager::GetInstance().StopSource();
    EXPECT_EQ(true, ret.empty());
}

/**
 * @tc.name: StopSink_001
 * @tc.desc: Verify the StopSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StopSink_001, TestSize.Level0)
{
    auto ret = ComponentManager::GetInstance().StopSink();
    EXPECT_EQ(true, ret.empty());
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

/**
 * @tc.name: InitCompSource_001
 * @tc.desc: Verify the InitCompSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, InitCompSource_001, TestSize.Level0)
{
    bool ret = ComponentManager::GetInstance().InitCompSource();
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: InitCompSink_001
 * @tc.desc: Verify the InitCompSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, InitCompSink_001, TestSize.Level0)
{
    bool ret = ComponentManager::GetInstance().InitCompSink();
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
}

/**
 * @tc.name: Enable_002
 * @tc.desc: Verify the Enable function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, Enable_002, TestSize.Level0)
{
    std::string networkId;
    std::string uuid;
    std::string dhId;
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    int32_t ret = ComponentManager::GetInstance().Enable(networkId, uuid, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_GET_ENABLE_PARAM_FAILED, ret);
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
    DHType dhType = DHType::INPUT;
    int32_t ret = ComponentManager::GetInstance().Disable(networkId, uuid, dhId, dhType);
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
    std::string networkId;
    std::string uuid;
    std::string dhId;
    DHType dhType = DHType::INPUT;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    int32_t ret = ComponentManager::GetInstance().Disable(networkId, uuid, dhId, dhType);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
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
    EXPECT_NE(DH_FWK_SUCCESS, ret);
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
HWTEST_F(ComponentManagerTest, GetSinkVersion_001, TestSize.Level0)
{
    std::string networkId;
    std::string uuid;
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersion(uuid, dhType, sinkVersion, true);
    EXPECT_NE(DH_FWK_SUCCESS, ret);
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
    ComponentManager::GetInstance().UpdateVersionCache(uuid, versionInfo);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: DumpLoadedComps_001
 * @tc.desc: Verify the DumpLoadedComps function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, DumpLoadedComps_001, TestSize.Level0)
{
    std::set<DHType> compSourceType;
    std::set<DHType> compSinkType;
    ComponentManager::GetInstance().DumpLoadedComps(compSourceType, compSinkType);
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
    DHType dhType = DHType::CAMERA;
    ComponentManager::GetInstance().RecoverDistributedHardware(dhType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
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
    DHContext::GetInstance().onlineDeviceMap_.clear();
    auto ret = ComponentManager::GetInstance().RetryGetEnableParam(NETWORK_TEST, UUID_TEST, DH_ID_1, dhType, param);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_ENABLE_FAILED);
}

/**
 * @tc.name: RetryGetEnableParam_002
 * @tc.desc: Verify the RetryGetEnableParam function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, RetryGetEnableParam_002, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    DHContext::GetInstance().onlineDeviceMap_[UUID_TEST] = NETWORK_TEST;
    auto ret = ComponentManager::GetInstance().RetryGetEnableParam(NETWORK_TEST, UUID_TEST, DH_ID_1, dhType, param);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    std::string key = Sha256(UUID_TEST);
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_TEST;
    ret = ComponentManager::GetInstance().RetryGetEnableParam(NETWORK_TEST, UUID_TEST, DH_ID_1, dhType, param);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: IsIdenticalAccount_001
 * @tc.desc: Verify the IsIdenticalAccount function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, IsIdenticalAccount_001, TestSize.Level0)
{
    DHType dhType = DHType::CAMERA;
    ComponentManager::GetInstance().RecoverDistributedHardware(dhType);
    auto ret = ComponentManager::GetInstance().IsIdenticalAccount(NETWORK_TEST);
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

HWTEST_F(ComponentManagerTest, QueryBusinessState_001, TestSize.Level0)
{
    std::string networkId = "networkId_test";
    std::string dhId = "dhId_test";
    BusinessState state = BusinessState::UNKNOWN;
    ComponentManager::GetInstance().UpdateBusinessState(networkId, dhId, state);

    state = BusinessState::IDLE;
    ComponentManager::GetInstance().needRefreshTaskParams_.clear();
    ComponentManager::GetInstance().UpdateBusinessState(networkId, dhId, state);

    TaskParam taskParam;
    ComponentManager::GetInstance().needRefreshTaskParams_[{networkId, dhId}] = taskParam;
    ComponentManager::GetInstance().UpdateBusinessState(networkId, dhId, state);

    std::string uuid = "uuid_test";
    ComponentManager::GetInstance().dhBizStates_[{uuid, dhId}] = state;
    BusinessState ret = ComponentManager::GetInstance().QueryBusinessState(uuid, dhId);
    EXPECT_EQ(BusinessState::IDLE, ret);

    ComponentManager::GetInstance().dhBizStates_.clear();
    ret = ComponentManager::GetInstance().QueryBusinessState(uuid, dhId);
    EXPECT_EQ(BusinessState::UNKNOWN, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
