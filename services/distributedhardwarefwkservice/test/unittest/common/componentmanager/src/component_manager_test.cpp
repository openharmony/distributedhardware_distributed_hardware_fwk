/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <thread>
#include <vector>

#include <gmock/gmock.h>

#include "component_disable.h"
#include "component_enable.h"
#define private public
#include "component_manager.h"
#undef private
#include "constants.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "mock_idistributed_hardware_sink.h"
#include "mock_idistributed_hardware_source.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
#undef DH_LOG_TAG
#define DH_LOG_TAG "ComponentManagerTest"

constexpr int32_t EXECUTE_TIME_TEST = 1000;
const std::string DEV_ID_TEST = "123456";
const std::string DH_ID_TEST = "Camera_0";

void ComponentManagerTest::SetUpTestCase(void) {}

void ComponentManagerTest::TearDownTestCase(void) {}

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

    MockIDistributedHardwareSource speakerSource;
    ComponentManager::GetInstance().compSource_.insert(std::make_pair(DHType::SPEAKER, &speakerSource));
    EXPECT_CALL(speakerSource, InitSource(testing::_)).Times(1).WillOnce(testing::Invoke(handler));

    MockIDistributedHardwareSink micSink;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(DHType::MIC, &micSink));
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

    MockIDistributedHardwareSink speakerSink;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(DHType::SPEAKER, &speakerSink));
    EXPECT_CALL(speakerSink, ReleaseSink()).Times(1).WillOnce(testing::Invoke(handler));

    MockIDistributedHardwareSink micSink;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(DHType::MIC, &micSink));
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
} // namespace DistributedHardware
} // namespace OHOS
