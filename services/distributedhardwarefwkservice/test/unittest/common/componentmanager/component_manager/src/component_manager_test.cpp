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
#include "dh_data_sync_trigger_listener.h"
#include "dh_state_listener.h"
#include "local_capability_info_manager.h"
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
constexpr int32_t TEST_COMP_SINK_SA_ID = 4804;
constexpr int32_t TEST_SINK_SA_ID = 12345;
constexpr int32_t TEST_SOURCE_SA_ID = 12345;
const std::string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const std::string NAME_CAMERA = "distributed_camera";
const std::string VERSION_1 = "1.0";
const std::string DEV_ID_TEST = "123456";
const std::string DH_ID_TEST = "Camera_0";
const std::string AUDIO_ID_TEST = "1";
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
constexpr uint16_t DEV_TYPE_TEST = 14;
const std::shared_ptr<CapabilityInfo> CAP_INFO_TEST =
    std::make_shared<CapabilityInfo>(DH_ID_TEST, DEV_ID_TEST, DEVICE_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1,
    DH_SUBTYPE_TEST);
const std::shared_ptr<MetaCapabilityInfo> META_INFO_PTR_TEST = std::make_shared<MetaCapabilityInfo>(
    DH_ID_TEST, DEV_ID_TEST, DEVICE_NAME, TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1, DH_SUBTYPE_TEST, UDIDHASH_TEST,
    CompVersion{ .sinkVersion = TEST_SINK_VERSION_1 });

static std::string g_mocklocalNetworkId = "123456789";
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

void SetUpComponentLoaderConfig()
{
    CompHandler handler;
    handler.compConfig.name = "distributed_audio";
    handler.compConfig.type = DHType::AUDIO;
    handler.compConfig.compHandlerLoc = "libdistributed_camera_handler.z.so";
    handler.compConfig.compHandlerVersion = "1.0";
    handler.compConfig.compSourceLoc = "libdistributed_camera_source_sdk.z.so";
    handler.compConfig.compSourceVersion = "1.0";
    handler.compConfig.compSinkLoc = "libdistributed_camera_sink_sdk.z.so";
    handler.compConfig.compSinkVersion = "2.0";
    handler.compConfig.compSinkSaId = TEST_COMP_SINK_SA_ID;
    handler.compConfig.haveFeature = false;
    handler.hardwareHandler = nullptr;
    handler.sourceHandler = nullptr;
    handler.sinkHandler = nullptr;
    handler.type = DHType::AUDIO;
    handler.sinkSaId = TEST_SINK_SA_ID;
    handler.sourceSaId = TEST_SOURCE_SA_ID;
    ComponentLoader::GetInstance().compHandlerMap_[DHType::AUDIO] = handler;
}

void SetDownComponentLoaderConfig()
{
    auto itHandler = ComponentLoader::GetInstance().compHandlerMap_.find(DHType::AUDIO);
    if (itHandler != ComponentLoader::GetInstance().compHandlerMap_.end()) {
        CompHandler &handler = itHandler->second;
        if (handler.sinkSaId == TEST_SINK_SA_ID && handler.sourceSaId == TEST_SOURCE_SA_ID) {
            ComponentLoader::GetInstance().compHandlerMap_.erase(itHandler);
        }
    }
}

std::string GetLocalNetworkId()
{
    return g_mocklocalNetworkId;
}

void ComponentManagerTest::TestGetDistributedHardwareCallback::OnSuccess(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    (void)networkId;
    (void)descriptors;
    (void)enableStep;
}

void ComponentManagerTest::TestGetDistributedHardwareCallback::OnError(
    const std::string &networkId, int32_t error)
{
    (void)networkId;
    (void)error;
}

/**
 * @tc.name: init_test_001
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, init_test_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, init_test_002, TestSize.Level1)
{
    ComponentManager::GetInstance().compMonitorPtr_ = nullptr;
    auto ret = ComponentManager::GetInstance().Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: unInit_test_001
 * @tc.desc: Verify the UnInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, unInit_test_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, init_compSource_test_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().Init();
    ComponentManager::GetInstance().compSource_.clear();
    ComponentManager::GetInstance().compMonitorPtr_ = nullptr;
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().InitCompSource(DHType::AUDIO);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_MONITOR_NULL);
}

/**
 * @tc.name: init_compSink_test_001
 * @tc.desc: Verify the InitCompSink
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, init_compSink_test_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().Init();
    ComponentManager::GetInstance().compSink_.clear();
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().InitCompSink(DHType::AUDIO);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    EXPECT_EQ(ComponentManager::GetInstance().compSink_.empty(), false);
}

/**
 * @tc.name: get_enableparam_test_001
 * @tc.desc: Verify the GetEnableParam
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, get_enableparam_test_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, get_sinkversion_fromvermgr_test_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetDHType_test_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, UnInit_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, StartSource_001, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    auto ret = ComponentManager::GetInstance().StartSource(dhType, sourceResult);
    EXPECT_EQ(ERR_DH_FWK_SA_HANDLER_IS_NULL, ret);
    ComponentManager::GetInstance().compSource_.clear();
}

/**
 * @tc.name: StartSource_002
 * @tc.desc: Verify the StartSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSource_002, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    auto ret = ComponentManager::GetInstance().StartSource(DHType::AUDIO, sourceResult);
    EXPECT_EQ(ERR_DH_FWK_TYPE_NOT_EXIST, ret);

    ret = ComponentManager::GetInstance().StartSource(dhType, sourceResult);
    EXPECT_EQ(ERR_DH_FWK_SA_HANDLER_IS_NULL, ret);
    ComponentManager::GetInstance().compSource_.clear();
}

/**
 * @tc.name: StartSink_001
 * @tc.desc: Verify the StartSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StartSink_001, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(dhType, sinkPtr));
    std::unordered_map<DHType, std::shared_future<int32_t>> sinkResult;
    auto ret = ComponentManager::GetInstance().StartSink(dhType, sinkResult);
    EXPECT_EQ(ERR_DH_FWK_SA_HANDLER_IS_NULL, ret);
    ComponentManager::GetInstance().compSink_.clear();
}

/**
 * @tc.name: StopSource_001
 * @tc.desc: Verify the StopSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StopSource_001, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    std::unordered_map<DHType, std::shared_future<int32_t>> sourceResult;
    auto ret = ComponentManager::GetInstance().StopSource(dhType, sourceResult);
    EXPECT_EQ(ERR_DH_FWK_SA_HANDLER_IS_NULL, ret);
    ComponentManager::GetInstance().compSource_.clear();
}
/**
 * @tc.name: StopSink_001
 * @tc.desc: Verify the StopSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, StopSink_001, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(dhType, sinkPtr));
    std::unordered_map<DHType, std::shared_future<int32_t>> sinkResult;
    auto ret = ComponentManager::GetInstance().StopSink(dhType, sinkResult);
    EXPECT_EQ(ERR_DH_FWK_SA_HANDLER_IS_NULL, ret);
    ComponentManager::GetInstance().compSink_.clear();
}

/**
 * @tc.name: WaitForResult_001
 * @tc.desc: Verify the WaitForResult function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, WaitForResult_001, TestSize.Level1)
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
    HWTEST_F(ComponentManagerTest, InitCompSource_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, InitCompSink_001, TestSize.Level1)
{
    SetUpComponentLoaderConfig();
    bool ret = ComponentManager::GetInstance().InitCompSink(DHType::AUDIO);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

/**
 * @tc.name: Enable_001
 * @tc.desc: Verify the Enable function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, Enable_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, Enable_002, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, Disable_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, Disable_002, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, Disable_003, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetDHType_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetDHType_002, TestSize.Level1)
{
    std::string key = Sha256(UUID_TEST) + RESOURCE_SEPARATOR + DH_ID_1;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = CAP_INFO_TEST;
    auto ret = ComponentManager::GetInstance().GetDHType(UUID_TEST, DH_ID_1);
    EXPECT_EQ(DHType::CAMERA, ret);
}

HWTEST_F(ComponentManagerTest, GetEnableCapParam_001, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, GetEnableCapParam_002, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    EnableParam param;
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_[DEV_ID_TEST] = CAP_INFO_TEST;
    int32_t ret = ComponentManager::GetInstance().GetEnableCapParam(NETWORK_TEST, UUID_TEST, dhType, param, capability);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_GET_SINK_VERSION_FAILED, ret);
    CapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
}

HWTEST_F(ComponentManagerTest, GetEnableMetaParam_001, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, GetEnableMetaParam_002, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, GetCapParam_001, TestSize.Level1)
{
    std::shared_ptr<CapabilityInfo> capability = nullptr;
    int32_t ret = ComponentManager::GetInstance().GetCapParam("", "", capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetCapParam(UUID_TEST, "", capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetCapParam("", DH_ID_TEST, capability);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetCapParam_002, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, GetMetaParam_001, TestSize.Level1)
{
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = nullptr;
    int32_t ret = ComponentManager::GetInstance().GetMetaParam("", "", metaCapInfo);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetMetaParam(UUID_TEST, "", metaCapInfo);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    ret = ComponentManager::GetInstance().GetMetaParam("", DH_ID_TEST, metaCapInfo);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetMetaParam_002, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetEnableParam_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetEnableParam_002, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetVersionFromVerMgr_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetVersionFromVerMgr_002, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetVersionFromVerMgr_003, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetVersionFromVerInfoMgr_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetVersionFromVerInfoMgr_002, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, GetVersion_001, TestSize.Level1)
{
    std::string uuid;
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersion(uuid, dhType, sinkVersion, true);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);
}

HWTEST_F(ComponentManagerTest, GetVersion_002, TestSize.Level1)
{
    std::string uuid = "123456798";
    DHType dhType = DHType::CAMERA;
    std::string sinkVersion;
    int32_t ret = ComponentManager::GetInstance().GetVersion(uuid, dhType, sinkVersion, true);
    EXPECT_EQ(ERR_DH_FWK_RESOURCE_DB_ADAPTER_POINTER_NULL, ret);
}

HWTEST_F(ComponentManagerTest, GetVersion_003, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, UpdateVersionCache_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, DumpLoadedCompsource_001, TestSize.Level1)
{
    std::set<DHType> compSourceType;
    ComponentManager::GetInstance().DumpLoadedCompsource(compSourceType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

HWTEST_F(ComponentManagerTest, DumpLoadedCompsink_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, Recover_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, DoRecover_001, TestSize.Level1)
{
    ComponentManager::GetInstance().DoRecover(DHType::UNKNOWN);
    ComponentManager::GetInstance().DoRecover(DHType::AUDIO);
    ComponentManager::GetInstance().DoRecover(DHType::CAMERA);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: DoRecover_002
 * @tc.desc: Verify the DoRecover function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, DoRecover_002, TestSize.Level1)
{
    CompVersion compVersion;
    compVersion.sinkVersion = "1.0";
    std::shared_ptr<MetaCapabilityInfo> metaCapInfo = std::make_shared<MetaCapabilityInfo>(
        "", "", "devName_test", TEST_DEV_TYPE_PAD, DHType::CAMERA, "attrs_test", "subtype", "", compVersion);
    DHType dhType = DHType::CAMERA;
    std::string udidHash = Sha256(UDID_TEST);
    std::string key = udidHash + "###" + DH_ID_TEST;
    MetaInfoManager::GetInstance()->globalMetaInfoMap_[key] = metaCapInfo;
    ComponentManager::DHSinkStatus sinkStatus;
    ComponentManager::GetInstance().dhSinkStatus_[dhType] = sinkStatus;
    ComponentManager::DHSourceStatus sourceStatus;
    ComponentManager::GetInstance().dhSourceStatus_[dhType] = sourceStatus;
    ComponentManager::GetInstance().DoRecover(dhType);
    EXPECT_EQ(true, ComponentManager::GetInstance().compSource_.empty());
}

/**
 * @tc.name: RetryGetEnableParam_001
 * @tc.desc: Verify the RetryGetEnableParam function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(ComponentManagerTest, RetryGetEnableParam_001, TestSize.Level1)
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
HWTEST_F(ComponentManagerTest, IsIdenticalAccount_001, TestSize.Level1)
{
    auto ret = ComponentManager::GetInstance().IsIdenticalAccount("");
    EXPECT_EQ(ret, false);

    ret = ComponentManager::GetInstance().IsIdenticalAccount(NETWORK_TEST);
    EXPECT_EQ(ret, false);
}

HWTEST_F(ComponentManagerTest, OnUnregisterResult_001, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, OnRegisterResult_001, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, UpdateBusinessState_001, TestSize.Level1)
{
    BusinessState state = BusinessState::UNKNOWN;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState("", "", state));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(NETWORK_TEST, "", state));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState("", DH_ID_TEST, state));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(NETWORK_TEST, DH_ID_TEST, state));
}

HWTEST_F(ComponentManagerTest, UpdateBusinessState_002, TestSize.Level1)
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

HWTEST_F(ComponentManagerTest, UpdateBusinessState_003, TestSize.Level1)
{
    std::string peeruuid = "123456789";
    std::string dhid = "audio_132";
    std::string deviceId = Sha256(peeruuid);
    BusinessState state = BusinessState::IDLE;
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "mic");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo1;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(deviceId, dhid, state));

    dhid = "camera_132";
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "camera");
    key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo2;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateBusinessState(deviceId, dhid, state));
}

HWTEST_F(ComponentManagerTest, GetDHSubtypeByDHId_001, TestSize.Level1)
{
    std::string dhid = "audio_1";
    std::string networkId = "networkId_123";
    DHSubtype dhSubtype;
    auto ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, networkId, dhid);
    EXPECT_EQ(ret, ERR_DH_FWK_BAD_OPERATION);

    std::string deviceId = Sha256(UUID_TEST);
    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, NETWORK_TEST);
    ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, NETWORK_TEST, dhid);
    EXPECT_EQ(ret, ERR_DH_FWK_BAD_OPERATION);

    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo;
    ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, NETWORK_TEST, dhid);
    EXPECT_EQ(ret, ERR_DH_FWK_BAD_OPERATION);
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    DHContext::GetInstance().devIdEntrySet_.clear();
}

HWTEST_F(ComponentManagerTest, GetDHSubtypeByDHId_002, TestSize.Level1)
{
    std::string dhid = "audio_1";
    DHSubtype dhSubtype;
    std::string deviceId = Sha256(UUID_TEST);
    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, NETWORK_TEST);
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "mic");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo1;
    auto ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, NETWORK_TEST, dhid);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    dhid = "audio_2";
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "camera");
    key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo2;
    ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, NETWORK_TEST, dhid);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    dhid = "audio_3";
    std::shared_ptr<CapabilityInfo> capInfo3 = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::UNKNOWN, "attrs", "unknown");
    key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo3;
    ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, NETWORK_TEST, dhid);
    EXPECT_EQ(ret, ERR_DH_FWK_BAD_OPERATION);
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_.clear();
    DHContext::GetInstance().devIdEntrySet_.clear();
}

HWTEST_F(ComponentManagerTest, GetDHSubtypeByDHId_003, TestSize.Level1)
{
    std::string dhid = "audio_1";
    DHSubtype dhSubtype;
    auto ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, g_mocklocalNetworkId, dhid);
    EXPECT_EQ(ret, ERR_DH_FWK_BAD_OPERATION);
}

HWTEST_F(ComponentManagerTest, GetDHSubtypeByDHId_004, TestSize.Level1)
{
    std::string dhid = "audio_1";
    DHSubtype dhSubtype;
    std::string networkId = "networkId_123";
    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, networkId);
    auto ret = ComponentManager::GetInstance().GetDHSubtypeByDHId(dhSubtype, networkId, dhid);
    EXPECT_EQ(ret, ERR_DH_FWK_BAD_OPERATION);
    DHContext::GetInstance().devIdEntrySet_.clear();
}

HWTEST_F(ComponentManagerTest, InitAVSyncSharedMemory_001, TestSize.Level1)
{
    auto ret = ComponentManager::GetInstance().InitAVSyncSharedMemory();
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, DeinitAVSyncSharedMemory_001, TestSize.Level1)
{
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().DeinitAVSyncSharedMemory());
}

HWTEST_F(ComponentManagerTest, GetDHIdByDHSubtype_001, TestSize.Level1)
{
    std::string networkId = "networkId_123456";
    std::string dhid = "audio_132";
    std::string deviceId = Sha256(UUID_TEST);
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "mic");
    std::string key = deviceId + "###" + dhid;
    BusinessState state = BusinessState::IDLE;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo1;
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(networkId, dhid), state);
    DHContext::GetInstance().AddOnlineDevice(UDID_TEST, UUID_TEST, networkId);
    auto ret = ComponentManager::GetInstance().GetDHIdByDHSubtype(DHSubtype::AUDIO_MIC, networkId, dhid);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    deviceId = "deviceId_test";
    ret = ComponentManager::GetInstance().GetDHIdByDHSubtype(DHSubtype::CAMERA, deviceId, dhid);
    EXPECT_EQ(ret, ERR_DH_FWK_BAD_OPERATION);
}

HWTEST_F(ComponentManagerTest, HandleIdleStateChange_001, TestSize.Level1)
{
    std::string networkId = "";
    std::string dhId = "";
    DHType dhType = DHType::UNKNOWN;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleIdleStateChange(networkId, dhId, dhType));

    networkId = "networkId_1";
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleIdleStateChange(networkId, dhId, dhType));

    networkId = "";
    dhId = "dhId_1";
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleIdleStateChange(networkId, dhId, dhType));

    networkId = "networkId_1";
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::CAMERA, sourcePtr));
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleIdleStateChange(networkId, dhId, dhType));

    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleIdleStateChange(networkId, dhId, DHType::CAMERA));
    ComponentManager::GetInstance().compSource_.clear();
}

HWTEST_F(ComponentManagerTest, HandleBusinessStateChange_001, TestSize.Level1)
{
    std::string networkId = "networkId_1";
    std::string dhid = "audio_132";
    DHSubtype dhSubType = DHSubtype::AUDIO_MIC;
    BusinessState state = BusinessState::IDLE;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    state = BusinessState::RUNNING;
    std::string peeruuid = "123456789";
    std::string deviceId = Sha256(peeruuid);
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
        dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "mic");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo1;
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid), BusinessState::IDLE);
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    std::string dhid2 = "camera_132";
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
        dhid2, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "camera");
    key = deviceId + "###" + dhid2;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo2;
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid2), BusinessState::UNKNOWN);
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid2), BusinessState::RUNNING);
    IDistributedHardwareSource *sourcePtr1 = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::CAMERA, sourcePtr1));
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    IDistributedHardwareSource *sourcePtr2 = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::AUDIO, sourcePtr2));
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    ComponentManager::GetInstance().dhBizStates_.clear();
    ComponentManager::GetInstance().compSource_.clear();
}

HWTEST_F(ComponentManagerTest, HandleBusinessStateChange_002, TestSize.Level1)
{
    std::string networkId = "networkId_1";
    std::string dhid = "audio_132";
    DHSubtype dhSubType = DHSubtype::AUDIO_MIC;
    BusinessState state = BusinessState::RUNNING;
    ComponentManager::GetInstance().dhBizStates_.clear();
    ComponentManager::GetInstance().compSource_.clear();
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
}

HWTEST_F(ComponentManagerTest, HandleBusinessStateChange_003, TestSize.Level1)
{
    std::string networkId = "networkId_1";
    std::string dhid = "audio_132";
    DHSubtype dhSubType = DHSubtype::AUDIO_MIC;
    BusinessState state = BusinessState::RUNNING;

    std::string peeruuid = "123456789";
    std::string deviceId = Sha256(peeruuid);
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
    dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "mic");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo1;
    std::string dhid2 = "camera_132";
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
    dhid2, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "camera");
    key = deviceId + "###" + dhid2;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo2;

    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid2), BusinessState::IDLE);
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    ComponentManager::GetInstance().dhBizStates_.clear();
    ComponentManager::GetInstance().compSource_.clear();
}

HWTEST_F(ComponentManagerTest, HandleBusinessStateChange_004, TestSize.Level1)
{
    std::string networkId = "networkId_1";
    std::string dhid = "audio_132";
    DHSubtype dhSubType = DHSubtype::AUDIO_MIC;
    BusinessState state = BusinessState::RUNNING;
    std::string peeruuid = "123456789";
    std::string deviceId = Sha256(peeruuid);
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
    dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "mic");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo1;
    std::string dhid2 = "camera_132";
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
    dhid2, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "camera");
    key = deviceId + "###" + dhid2;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo2;
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid2), BusinessState::RUNNING);
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid), BusinessState::RUNNING);

    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    ComponentManager::GetInstance().dhBizStates_.clear();
    ComponentManager::GetInstance().compSource_.clear();
}

HWTEST_F(ComponentManagerTest, HandleBusinessStateChange_005, TestSize.Level1)
{
    std::string networkId = "networkId_1";
    std::string dhid = "camera_132";
    DHSubtype dhSubType = DHSubtype::CAMERA;
    BusinessState state = BusinessState::IDLE;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    state = BusinessState::RUNNING;
    std::string peeruuid = "123456789";
    std::string deviceId = Sha256(peeruuid);
    std::shared_ptr<CapabilityInfo> capInfo1 = std::make_shared<CapabilityInfo>(
    dhid, deviceId, "devName_test", DEV_TYPE_TEST, DHType::CAMERA, "attrs", "camera");
    std::string key = deviceId + "###" + dhid;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo1;
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid), BusinessState::IDLE);
    std::string dhid2 = "audio_132";
    std::shared_ptr<CapabilityInfo> capInfo2 = std::make_shared<CapabilityInfo>(
    dhid2, deviceId, "devName_test", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "mic");
    key = deviceId + "###" + dhid2;
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo2;
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid2), BusinessState::UNKNOWN);
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(deviceId, dhid2), BusinessState::RUNNING);
    IDistributedHardwareSource *sourcePtr1 = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::AUDIO, sourcePtr1));
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    IDistributedHardwareSource *sourcePtr2 = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::CAMERA, sourcePtr2));
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().HandleBusinessStateChange(networkId,
        dhid, dhSubType, state));
    ComponentManager::GetInstance().dhBizStates_.clear();
    ComponentManager::GetInstance().compSource_.clear();
}

HWTEST_F(ComponentManagerTest, QueryBusinessState_001, TestSize.Level1)
{
    BusinessState ret = ComponentManager::GetInstance().QueryBusinessState("", "");
    EXPECT_EQ(BusinessState::UNKNOWN, ret);

    ret = ComponentManager::GetInstance().QueryBusinessState(UUID_TEST, "");
    EXPECT_EQ(BusinessState::UNKNOWN, ret);

    ret = ComponentManager::GetInstance().QueryBusinessState("", DH_ID_TEST);
    EXPECT_EQ(BusinessState::UNKNOWN, ret);
}

HWTEST_F(ComponentManagerTest, QueryBusinessState_002, TestSize.Level1)
{
    std::string uuid = "uuid_123";
    std::string dhId = "camera_1";
    ComponentManager::GetInstance().dhBizStates_.emplace(std::make_pair(uuid, dhId), BusinessState::IDLE);
    BusinessState ret = ComponentManager::GetInstance().QueryBusinessState(uuid, dhId);
    EXPECT_EQ(BusinessState::IDLE, ret);

    ComponentManager::GetInstance().dhBizStates_.clear();
    ret = ComponentManager::GetInstance().QueryBusinessState(uuid, dhId);
    EXPECT_EQ(BusinessState::UNKNOWN, ret);
}

HWTEST_F(ComponentManagerTest, TriggerFullCapsSync_001, TestSize.Level1)
{
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().TriggerFullCapsSync(""));

    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().TriggerFullCapsSync(NETWORK_TEST));

    ComponentManager::GetInstance().dhCommToolPtr_ = std::make_shared<DHCommTool>();
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().TriggerFullCapsSync(NETWORK_TEST));
}

HWTEST_F(ComponentManagerTest, GetDHSourceInstance_001, TestSize.Level1)
{
    const IDistributedHardwareSource *sourcePtr = ComponentManager::GetInstance().GetDHSourceInstance(DHType::UNKNOWN);
    EXPECT_EQ(nullptr, sourcePtr);
}

HWTEST_F(ComponentManagerTest, GetDHSourceInstance_002, TestSize.Level1)
{
    DHType dhType = DHType::CAMERA;
    IDistributedHardwareSource *sourcePtr = nullptr;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(dhType, sourcePtr));
    IDistributedHardwareSource *dhSourcePtr = ComponentManager::GetInstance().GetDHSourceInstance(dhType);
    EXPECT_EQ(nullptr, dhSourcePtr);
    ComponentManager::GetInstance().compSource_.clear();
}

HWTEST_F(ComponentManagerTest, CheckDemandStart_001, TestSize.Level1)
{
    bool enableSource = false;
    auto ret = ComponentManager::GetInstance().CheckDemandStart(UUID_TEST, DHType::AUDIO, enableSource);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_COMPVERSION_NOT_FOUND);
}

HWTEST_F(ComponentManagerTest, RegisterDHStatusListener_001, TestSize.Level1)
{
    sptr<IHDSinkStatusListener> listener = nullptr;
    auto ret = ComponentManager::GetInstance().RegisterDHStatusListener(listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, RegisterDHStatusListener_002, TestSize.Level1)
{
    sptr<IHDSourceStatusListener> listener = nullptr;
    auto ret = ComponentManager::GetInstance().RegisterDHStatusListener(NETWORK_TEST, listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, UnregisterDHStatusListener_001, TestSize.Level1)
{
    sptr<IHDSinkStatusListener> listener = nullptr;
    auto ret = ComponentManager::GetInstance().UnregisterDHStatusListener(listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, UnregisterDHStatusListener_002, TestSize.Level1)
{
    sptr<IHDSourceStatusListener> listener = nullptr;
    auto ret = ComponentManager::GetInstance().UnregisterDHStatusListener(NETWORK_TEST, listener, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, EnableSink_001, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = AUDIO_ID_TEST,
        .dhType = DHType::AUDIO
    };
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().EnableSink(dhDescriptor, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = ComponentManager::GetInstance().EnableSink(dhDescriptor, 0, 0);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, DisableSink_001, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = AUDIO_ID_TEST,
        .dhType = DHType::AUDIO
    };
    auto ret = ComponentManager::GetInstance().DisableSink(dhDescriptor, 0, 0);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
    ret = ComponentManager::GetInstance().DisableSink(dhDescriptor, 0, 0);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTest, EnableSource_001, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = AUDIO_ID_TEST,
        .dhType = DHType::AUDIO
    };
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().EnableSource(NETWORK_TEST, dhDescriptor, 0, 0);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_MONITOR_NULL);
}

HWTEST_F(ComponentManagerTest, DisableSource_001, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = AUDIO_ID_TEST,
        .dhType = DHType::AUDIO
    };
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().DisableSource(NETWORK_TEST, dhDescriptor, 0, 0);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, ForceDisableSink_001, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = AUDIO_ID_TEST,
        .dhType = DHType::AUDIO
    };
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().ForceDisableSink(dhDescriptor);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTest, ForceDisableSource_001, TestSize.Level1)
{
    DHDescriptor dhDescriptor {
        .id = AUDIO_ID_TEST,
        .dhType = DHType::AUDIO
    };
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().ForceDisableSource(NETWORK_TEST, dhDescriptor);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(ComponentManagerTest, UninitCompSource_001, TestSize.Level1)
{
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().UninitCompSource(DHType::AUDIO);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_MONITOR_NULL);
}

HWTEST_F(ComponentManagerTest, UninitCompSink_001, TestSize.Level1)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    auto ret = ComponentManager::GetInstance().UninitCompSink(DHType::AUDIO);
    EXPECT_EQ(ret, ERR_DH_FWK_LOADER_HANDLER_IS_NULL);
}

HWTEST_F(ComponentManagerTest, UninitCompSink_002, TestSize.Level1)
{
    ComponentLoader::GetInstance().compHandlerMap_.clear();
    SetUpComponentLoaderConfig();
    auto ret = ComponentManager::GetInstance().UninitCompSink(DHType::AUDIO);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(ComponentManagerTest, OnDataSyncTrigger_001, testing::ext::TestSize.Level1)
{
    DHDataSyncTriggerListener dhDataSyncTrigger;
    std::string networkId = "";
    ASSERT_NO_FATAL_FAILURE(dhDataSyncTrigger.OnDataSyncTrigger(networkId));

    networkId = "networkId_test";
    ASSERT_NO_FATAL_FAILURE(dhDataSyncTrigger.OnDataSyncTrigger(networkId));
}

HWTEST_F(ComponentManagerTest, OnStateChanged_001, testing::ext::TestSize.Level1)
{
    DHStateListener dhStateListenenr;
    std::string dhId = "dhId_test";
    BusinessState state = BusinessState::UNKNOWN;
    std::string networkId = "";
    ASSERT_NO_FATAL_FAILURE(dhStateListenenr.OnStateChanged(networkId, dhId, state));

    networkId = "networkId_test";
    ASSERT_NO_FATAL_FAILURE(dhStateListenenr.OnStateChanged(networkId, dhId, state));
}

HWTEST_F(ComponentManagerTest, CheckSinkConfigStart_001, TestSize.Level1)
{
    bool enableSink = false;
    auto ret = ComponentManager::GetInstance().CheckSinkConfigStart(DHType::GPS, enableSink);
    EXPECT_EQ(ret, ERR_DH_FWK_TYPE_NOT_EXIST);
}

HWTEST_F(ComponentManagerTest, InitAndUnInit_DHCommTool_001, TestSize.Level0)
{
    ComponentManager::GetInstance().dhCommToolPtr_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().InitDHCommTool());
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UnInitDHCommTool());

    ComponentManager::GetInstance().dhCommToolPtr_ = std::make_shared<DHCommTool>();
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().InitDHCommTool());
    EXPECT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UnInitDHCommTool());
}

HWTEST_F(ComponentManagerTest, SyncRemoteDeviceInfoBySoftbus_001, TestSize.Level0)
{
    std::string realNetworkId = "realNetworkId_1";
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    sptr<IGetDhDescriptorsCallback> callback = nullptr;
    EXPECT_NO_FATAL_FAILURE(
        ComponentManager::GetInstance().SyncRemoteDeviceInfoBySoftbus(realNetworkId, enableStep, callback));
}

HWTEST_F(ComponentManagerTest, OnGetDescriptors_001, TestSize.Level0)
{
    std::string realNetworkId = "realNetworkId_1";
    std::vector<DHDescriptor> descriptors;
    ComponentManager::GetInstance().OnGetDescriptors(realNetworkId, descriptors);
    EXPECT_TRUE(ComponentManager::GetInstance().syncDeviceInfoMap_.empty());
}

HWTEST_F(ComponentManagerTest, OnGetDescriptors_002, TestSize.Level0)
{
    std::string realNetworkId = "realNetworkId_1";
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    std::vector<DHDescriptor> descriptors;
    DHDescriptor descriptor;
    descriptor.id = "camera_1";
    descriptor.dhType = DHType::CAMERA;
    descriptors.push_back(descriptor);
    ComponentManager::GetInstance().syncDeviceInfoMap_[realNetworkId] = {enableStep, nullptr};
    ComponentManager::GetInstance().OnGetDescriptors("realNetworkId_test", descriptors);
    EXPECT_FALSE(ComponentManager::GetInstance().syncDeviceInfoMap_.empty());
}

HWTEST_F(ComponentManagerTest, OnGetDescriptorsError_001, TestSize.Level0)
{
    std::string realNetworkId = "realNetworkId_1";
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    ComponentManager::GetInstance().syncDeviceInfoMap_[realNetworkId] = {enableStep, nullptr};
    ComponentManager::GetInstance().OnGetDescriptorsError();
    EXPECT_FALSE(ComponentManager::GetInstance().syncDeviceInfoMap_.empty());
}

HWTEST_F(ComponentManagerTest, OnStateChanged_Sink_001, testing::ext::TestSize.Level1)
{
    DHSinkStateListener sinkStateListenenr;
    std::string dhId = "";
    std::string networkId = "";
    BusinessSinkState state = BusinessSinkState::UNKNOWN;
    ASSERT_NO_FATAL_FAILURE(sinkStateListenenr.OnStateChanged(networkId, dhId, state));

    networkId = "networkId_1";
    ASSERT_NO_FATAL_FAILURE(sinkStateListenenr.OnStateChanged(networkId, dhId, state));

    networkId = "";
    dhId = "dhId_1";
    ASSERT_NO_FATAL_FAILURE(sinkStateListenenr.OnStateChanged(networkId, dhId, state));

    networkId = "networkId_1";
    dhId = "dhId_1";
    ASSERT_NO_FATAL_FAILURE(sinkStateListenenr.OnStateChanged(networkId, dhId, state));
}

HWTEST_F(ComponentManagerTest, SetAVSyncScene_001, testing::ext::TestSize.Level1)
{
    DHTopic topic = DHTopic::TOPIC_AV_LOW_LATENCY;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().SetAVSyncScene(topic));

    topic = DHTopic::TOPIC_AV_FLUENCY;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().SetAVSyncScene(topic));

    topic = DHTopic::TOPIC_MAX;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().SetAVSyncScene(topic));
}

HWTEST_F(ComponentManagerTest, UpdateSinkBusinessState_001, testing::ext::TestSize.Level1)
{
    std::string networkId = "";
    std::string dhId = "";
    BusinessSinkState state = BusinessSinkState::UNKNOWN;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateSinkBusinessState(networkId, dhId, state));

    networkId = "networkId_1";
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateSinkBusinessState(networkId, dhId, state));

    networkId = "";
    dhId = "dhId_1";
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateSinkBusinessState(networkId, dhId, state));

    networkId = "networkId_1";
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateSinkBusinessState(networkId, dhId, state));

    state = BusinessSinkState::RUNNING;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateSinkBusinessState(networkId, dhId, state));

    state = BusinessSinkState::IDLE;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().UpdateSinkBusinessState(networkId, dhId, state));
}

HWTEST_F(ComponentManagerTest, NotifyBusinessStateChange_001, testing::ext::TestSize.Level1)
{
    BusinessState state = BusinessState::UNKNOWN;
    DHSubtype dhSubType = DHSubtype::CAMERA;
    ASSERT_NO_FATAL_FAILURE(ComponentManager::GetInstance().NotifyBusinessStateChange(dhSubType, state));
}
} // namespace DistributedHardware
} // namespace OHOS
