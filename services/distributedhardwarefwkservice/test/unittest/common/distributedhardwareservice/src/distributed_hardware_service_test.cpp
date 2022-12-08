/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "distributed_hardware_service_test.h"

#include <chrono>
#include <thread>
#include <vector>

#include "constants.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#define protected public
#define private public
#include "distributed_hardware_service.h"
#include "distributed_hardware_manager.h"
#include "task_board.h"
#undef private
#undef protected
#include "mock_publisher_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t ASID = 4801;
    const DHTopic TOPIC = DHTopic::TOPIC_START_DSCREEN;
    sptr<IPublisherListener> g_listener = sptr<IPublisherListener>(new MockPublisherListener());

    const std::u16string ARGS_H = u"-h";
    const std::u16string ARGS_L = u"-l";
    const std::u16string ARGS_E = u"-e";
    const std::u16string ARGS_T = u"-t";
    const std::u16string ARGS_C = u"-c";
}
void DistributedHardwareServiceTest::SetUpTestCase(void) {}

void DistributedHardwareServiceTest::TearDownTestCase(void) {}

void DistributedHardwareServiceTest::SetUp() {}

void DistributedHardwareServiceTest::TearDown() {}

/**
 * @tc.name: register_publisher_listener_001
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, register_publisher_listener_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    auto ret = service.RegisterPublisherListener(TOPIC, g_listener);

    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: register_publisher_listener_002
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, register_publisher_listener_002, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    sptr<IPublisherListener> listener = nullptr;
    auto ret = service.RegisterPublisherListener(TOPIC, listener);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: unregister_publisher_listener_001
 * @tc.desc: Verify the UnregisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, unregister_publisher_listener_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    service.RegisterPublisherListener(TOPIC, g_listener);
    auto ret = service.UnregisterPublisherListener(TOPIC, g_listener);

    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: publish_message_001
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, publish_message_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string msg;
    service.RegisterPublisherListener(TOPIC, g_listener);
    auto ret = service.PublishMessage(TOPIC, msg);

    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: publish_message_002
 * @tc.desc: Verify the PublishMessage function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, publish_message_002, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string msg = "msg";
    service.RegisterPublisherListener(TOPIC, g_listener);
    auto ret = service.PublishMessage(TOPIC, msg);

    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: onStop_test_002
 * @tc.desc: Verify the OnStop function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, onStop_test_002, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    service.OnStop();

    EXPECT_EQ(service.state_, ServiceRunningState::STATE_NOT_START);
}

/**
 * @tc.name: dump_test_001
 * @tc.desc: Verify the Dump function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, dump_test_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    int32_t fd = 1;

    auto ret = service.Dump(fd, std::vector<std::u16string> { ARGS_H });
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.Dump(fd, std::vector<std::u16string> { ARGS_L });
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.Dump(fd, std::vector<std::u16string> { ARGS_E });
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.Dump(fd, std::vector<std::u16string> { ARGS_T });
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.Dump(fd, std::vector<std::u16string> { ARGS_C });
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: OnStart_001
 * @tc.desc: Verify the OnStart function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, OnStart_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    service.state_ = ServiceRunningState::STATE_RUNNING;
    service.OnStart();
    service.OnStop();
    EXPECT_EQ(ServiceRunningState::STATE_NOT_START, service.state_);
}

/**
 * @tc.name: Init_001
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, Init_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    service.registerToService_ = false;

    EXPECT_EQ(false, service.Init());
}
} // namespace DistributedHardware
} // namespace OHOS
