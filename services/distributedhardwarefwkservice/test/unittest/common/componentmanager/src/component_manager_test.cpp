/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>

#include "component_disable.h"
#include "component_enable.h"
#include "component_loader.h"
#include "capability_info.h"
#include "capability_info_manager.h"
#define private public
#include "component_manager.h"
#undef private
#include "constants.h"
#include "dh_context.h"
#include "dh_utils_tool.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "mock_idistributed_hardware_sink.h"
#include "mock_idistributed_hardware_source.h"
#include "version_info_manager.h"
#include "version_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentManagerTest"

namespace {
constexpr int32_t EXECUTE_TIME_TEST = 1000;
constexpr uint16_t TEST_DEV_TYPE_PAD = 0x11;
const std::string DATABASE_DIR = "/data/service/el1/public/database/dtbhardware_manager_service/";
const std::string NAME_CAMERA = "distributed_camera";
const std::string VERSION_1 = "1.0";
const std::string DEV_ID_TEST = "123456";
const std::string DH_ID_TEST = "Camera_0";
const std::string NETWORK_TEST = "nt36a637105409e904d4da83790a4a8";
const std::string UUID_TEST = "bb536a637105409e904d4da78290ab1";
const std::string DH_ATTR_1 = "attr1";
const std::string DEV_NAME = "Dev1";
const std::string DH_ID_1 = "Camera_1";
const std::shared_ptr<CapabilityInfo> CAP_INFO_1 =
    std::make_shared<CapabilityInfo>(DH_ID_1, GetDeviceIdByUUID(UUID_TEST), DEV_NAME,
    TEST_DEV_TYPE_PAD, DHType::CAMERA, DH_ATTR_1);
}

void ComponentManagerTest::SetUpTestCase(void)
{
    auto ret = mkdir(DATABASE_DIR.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (ret != 0) {
        DHLOGE("mkdir failed, path: %s, errno : %d", DATABASE_DIR.c_str(), errno);
    }
}

void ComponentManagerTest::TearDownTestCase(void)
{
    auto ret = remove(DATABASE_DIR.c_str());
    if (ret != 0) {
        DHLOGE("remove dir failed, path: %s, errno : %d", DATABASE_DIR.c_str(), errno);
    }
}

void ComponentManagerTest::SetUp()
{
    ComponentManager::GetInstance().compSource_.clear();
    ComponentManager::GetInstance().compSink_.clear();
}

void ComponentManagerTest::TearDown() {}

int32_t ComponentManagerTest::Enable(int32_t timeout, int32_t status)
{
    MockIDistributedHardwareSource stub;
    auto compEnable = std::make_shared<ComponentEnable>();
    EnableParam parameters;
    std::future<int32_t> future;
    auto handler = [&future, timeout, status, compEnable](std::string uuid, std::string dhId,
        const EnableParam &parameters, std::shared_ptr<RegisterCallback> callback) {
        future = std::async(std::launch::async, [timeout, compEnable, uuid, dhId, status]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            return compEnable->OnRegisterResult(uuid, dhId, status, "");
        });
        return DH_FWK_SUCCESS;
    };

    EXPECT_CALL(stub, RegisterDistributedHardware(DEV_ID_TEST, DH_ID_TEST, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(handler));

    auto start = std::chrono::system_clock::now();
    auto ret = compEnable->Enable(DEV_ID_TEST, DH_ID_TEST, parameters, &stub);
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    DHLOGI("enable callback use time: %d (ms)", diff);
    return ret;
}

int32_t ComponentManagerTest::Disable(int32_t timeout, int32_t status)
{
    MockIDistributedHardwareSource stub;
    auto compDisable = std::make_shared<ComponentDisable>();

    std::future<int32_t> future;
    auto handler = [&future, timeout, status, compDisable](std::string uuid, std::string dhId,
        std::shared_ptr<UnregisterCallback> callback) {
        future = std::async(std::launch::async, [timeout, compDisable, uuid, dhId, status]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
            return compDisable->OnUnregisterResult(uuid, dhId, status, "");
        });
        return DH_FWK_SUCCESS;
    };

    EXPECT_CALL(stub, UnregisterDistributedHardware(DEV_ID_TEST, DH_ID_TEST, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(handler));

    auto start = std::chrono::system_clock::now();

    auto ret = compDisable->Disable(DEV_ID_TEST, DH_ID_TEST, &stub);
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    DHLOGI("disable callback use time: %d (ms)", diff);
    return ret;
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
 * @tc.name: init_test_003
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, init_test_003, TestSize.Level0)
{
    auto handler = [](std::string param) {
        std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTE_TIME_TEST));
        return DH_FWK_SUCCESS;
    };

    MockIDistributedHardwareSource cameraSource;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::CAMERA, &cameraSource));
    EXPECT_CALL(cameraSource, InitSource(testing::_)).Times(1).WillOnce(testing::Invoke(handler));

    MockIDistributedHardwareSource displaySource;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::DISPLAY, &displaySource));
    EXPECT_CALL(displaySource, InitSource(testing::_)).Times(1).WillOnce(testing::Invoke(handler));
    
    MockIDistributedHardwareSink micSink;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(DHType::AUDIO, &micSink));
    EXPECT_CALL(micSink, InitSink(testing::_)).Times(1).WillOnce(testing::Invoke(handler));

    auto start = std::chrono::system_clock::now();

    auto ret = ComponentManager::GetInstance().Init();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    DHLOGI("Init use time: %d (ms)", diff);
    ASSERT_TRUE(diff <= EXECUTE_TIME_TEST * 1.1);
}

/**
 * @tc.name: unInit_test_001
 * @tc.desc: Verify the UnInit function
 * @tc.type: FUNC
 * @tc.require: AR000GHSK5
 */
HWTEST_F(ComponentManagerTest, unInit_test_001, TestSize.Level0)
{
    auto handler = []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(EXECUTE_TIME_TEST));
        return DH_FWK_SUCCESS;
    };

    MockIDistributedHardwareSource cameraSource;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::CAMERA, &cameraSource));
    EXPECT_CALL(cameraSource, ReleaseSource()).Times(1).WillOnce(testing::Invoke(handler));

    MockIDistributedHardwareSink displaycSink;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(DHType::DISPLAY, &displaycSink));
    EXPECT_CALL(displaycSink, ReleaseSink()).Times(1).WillOnce(testing::Invoke(handler));
    
    MockIDistributedHardwareSink micSink;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(DHType::AUDIO, &micSink));
    EXPECT_CALL(micSink, ReleaseSink()).Times(1).WillOnce(testing::Invoke(handler));

    auto start = std::chrono::system_clock::now();

    auto ret = ComponentManager::GetInstance().UnInit();
    EXPECT_EQ(DH_FWK_SUCCESS, ret);

    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
    DHLOGI("UnInit use time : %d (ms)", diff);
    ASSERT_TRUE(diff <= EXECUTE_TIME_TEST * 1.1);
}

/**
 * @tc.name: enable_test_001
 * @tc.desc: Verify the Enable success
 * @tc.type: FUNC
 * @tc.require: AR000GHSK7
 */
HWTEST_F(ComponentManagerTest, enable_test_001, TestSize.Level0)
{
    auto result = Enable(ENABLE_TIMEOUT_MS / 2, DH_FWK_SUCCESS);
    EXPECT_EQ(DH_FWK_SUCCESS, result);
}

/**
 * @tc.name: enable_test_002
 * @tc.desc: Verify the Enable failed for register hardware failed
 * @tc.type: FUNC
 * @tc.require: AR000GHSK7
 */

HWTEST_F(ComponentManagerTest, enable_test_002, TestSize.Level0)
{
    auto result = Enable(ENABLE_TIMEOUT_MS / 2, ERR_DH_FWK_COMPONENT_ENABLE_FAILED);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_ENABLE_FAILED, result);
}

/**
 * @tc.name: enable_test_003
 * @tc.desc: Verify the Enable timeout
 * @tc.type: FUNC
 * @tc.require: AR000GHSK7
 */
HWTEST_F(ComponentManagerTest, enable_test_003, TestSize.Level0)
{
    auto result = Enable(ENABLE_TIMEOUT_MS * 2, ERR_DH_FWK_COMPONENT_ENABLE_FAILED);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT, result);
}

/**
 * @tc.name: enable_test_004
 * @tc.desc: Verify the Enable for Multi-thread
 * @tc.type: FUNC
 * @tc.require: AR000GHSK7
 */
HWTEST_F(ComponentManagerTest, enable_test_004, TestSize.Level0)
{
    auto handler = [this](int32_t time, int32_t status, int32_t expect) {
        auto ret = this->Enable(time, status);
        EXPECT_EQ(expect, ret);
    };

    std::thread thread1(handler, ENABLE_TIMEOUT_MS / 2, DH_FWK_SUCCESS, DH_FWK_SUCCESS);
    std::thread thread2(handler, ENABLE_TIMEOUT_MS / 2, ERR_DH_FWK_COMPONENT_ENABLE_FAILED,
        ERR_DH_FWK_COMPONENT_ENABLE_FAILED);
    std::thread thread3(handler, ENABLE_TIMEOUT_MS * 3, DH_FWK_SUCCESS, ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT);

    std::thread thread6(handler, ENABLE_TIMEOUT_MS / 10, DH_FWK_SUCCESS, DH_FWK_SUCCESS);
    std::thread thread4(handler, ENABLE_TIMEOUT_MS / 10, ERR_DH_FWK_COMPONENT_ENABLE_FAILED,
        ERR_DH_FWK_COMPONENT_ENABLE_FAILED);
    std::thread thread5(handler, ENABLE_TIMEOUT_MS * 2, DH_FWK_SUCCESS, ERR_DH_FWK_COMPONENT_ENABLE_TIMEOUT);

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
    thread6.join();
}

/**
 * @tc.name: disable_test_001
 * @tc.desc: Verify the Disable success
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, disable_test_001, TestSize.Level0)
{
    auto result = Disable(DISABLE_TIMEOUT_MS / 2, DH_FWK_SUCCESS);
    EXPECT_EQ(DH_FWK_SUCCESS, result);
}

/**
 * @tc.name: disable_test_002
 * @tc.desc: Verify the Disable failed
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, disable_test_002, TestSize.Level0)
{
    auto result = Disable(DISABLE_TIMEOUT_MS / 2, ERR_DH_FWK_COMPONENT_DISABLE_FAILED);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_DISABLE_FAILED, result);
}

/**
 * @tc.name: disable_test_003
 * @tc.desc: Verify the Disable timeout
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, disable_test_003, TestSize.Level0)
{
    auto result = Disable(DISABLE_TIMEOUT_MS * 2, ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT);
    EXPECT_EQ(ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT, result);
}

/**
 * @tc.name: disable_test_004
 * @tc.desc: Verify the Disable for Multi-thread
 * @tc.type: FUNC
 * @tc.require: AR000GHSK9
 */
HWTEST_F(ComponentManagerTest, disable_test_004, TestSize.Level0)
{
    auto handler = [this](int32_t timeout, int32_t status, int32_t expect) {
        auto result = this->Disable(timeout, status);
        EXPECT_EQ(expect, result);
    };

    std::thread thread1(handler, DISABLE_TIMEOUT_MS / 2, DH_FWK_SUCCESS, DH_FWK_SUCCESS);
    std::thread thread2(handler, DISABLE_TIMEOUT_MS / 2, ERR_DH_FWK_COMPONENT_DISABLE_FAILED,
        ERR_DH_FWK_COMPONENT_DISABLE_FAILED);
    std::thread thread3(handler, DISABLE_TIMEOUT_MS * 3, DH_FWK_SUCCESS, ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT);

    std::thread thread4(handler, DISABLE_TIMEOUT_MS / 10, ERR_DH_FWK_COMPONENT_DISABLE_FAILED,
        ERR_DH_FWK_COMPONENT_DISABLE_FAILED);
    std::thread thread6(handler, DISABLE_TIMEOUT_MS / 10, DH_FWK_SUCCESS, DH_FWK_SUCCESS);
    std::thread thread5(handler, DISABLE_TIMEOUT_MS * 2, DH_FWK_SUCCESS, ERR_DH_FWK_COMPONENT_DISABLE_TIMEOUT);

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();
    thread5.join();
    thread6.join();
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
 * @tc.desc: Verify the InitCompSource
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
    DHContext::GetInstance().AddOnlineDevice(NETWORK_TEST, UUID_TEST);

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
    verInfo1.deviceId = GetDeviceIdByUUID(UUID_TEST);
    verInfo1.dhVersion = VERSION_1;
    verInfo1.compVersions.insert(std::pair<DHType, CompVersion>(compVersions1.dhType, compVersions1));

    VersionInfoManager::GetInstance()->Init();
    VersionInfoManager::GetInstance()->AddVersion(verInfo1);

    EnableParam param;
    auto ret = ComponentManager::GetInstance().GetEnableParam(NETWORK_TEST, UUID_TEST,
        DH_ID_1, DHType::CAMERA, param);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: get_sinkversionfromvermgr_test_001
 * @tc.desc: Verify the GetSinkVersionFromVerMgr
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
    auto ret = ComponentManager::GetInstance().GetSinkVersionFromVerMgr(UUID_TEST, DHType::CAMERA, sinkVersion);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}
} // namespace DistributedHardware
} // namespace OHOS
