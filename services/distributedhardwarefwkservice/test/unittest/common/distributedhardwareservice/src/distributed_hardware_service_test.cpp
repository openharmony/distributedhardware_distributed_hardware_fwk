/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include "cJSON.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_service.h"
#include "distributed_hardware_manager.h"
#include "task_board.h"
#include "mock_publisher_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
    const int32_t ASID = 4801;
    const DHTopic TOPIC = DHTopic::TOPIC_START_DSCREEN;
    sptr<IPublisherListener> g_listener(new MockPublisherListener());

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
 * @tc.name: QueryLocalSysSpec_001
 * @tc.desc: Verify the QueryLocalSysSpec function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, QueryLocalSysSpec_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string ret = service.QueryLocalSysSpec(QueryLocalSysSpecType::HISTREAMER_AUDIO_ENCODER);
    ret = service.QueryLocalSysSpec(QueryLocalSysSpecType::HISTREAMER_AUDIO_DECODER);
    ret = service.QueryLocalSysSpec(QueryLocalSysSpecType::HISTREAMER_VIDEO_ENCODER);
    ret = service.QueryLocalSysSpec(QueryLocalSysSpecType::HISTREAMER_VIDEO_DECODER);
    ret = service.QueryLocalSysSpec(QueryLocalSysSpecType::MAX);
    EXPECT_EQ(ret.empty(), true);
}

/**
 * @tc.name: PauseDistributedHardware_001
 * @tc.desc: Verify the PauseDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, PauseDistributedHardware_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.PauseDistributedHardware(DHType::UNKNOWN, networkId);
    ret = service.PauseDistributedHardware(DHType::CAMERA, networkId);
    ret = service.PauseDistributedHardware(DHType::MAX_DH, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

/**
 * @tc.name: ResumeDistributedHardware_001
 * @tc.desc: Verify the ResumeDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, ResumeDistributedHardware_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.PauseDistributedHardware(DHType::UNKNOWN, networkId);
    ret = service.PauseDistributedHardware(DHType::AUDIO, networkId);
    ret = service.PauseDistributedHardware(DHType::MAX_DH, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

/**
 * @tc.name: StopDistributedHardware_001
 * @tc.desc: Verify the StopDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, StopDistributedHardware_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.PauseDistributedHardware(DHType::UNKNOWN, networkId);
    ret = service.PauseDistributedHardware(DHType::INPUT, networkId);
    ret = service.PauseDistributedHardware(DHType::MAX_DH, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(DistributedHardwareServiceTest, ResumeDistributedHardware_002, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.ResumeDistributedHardware(DHType::UNKNOWN, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(DistributedHardwareServiceTest, StopDistributedHardware_002, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.StopDistributedHardware(DHType::UNKNOWN, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(DistributedHardwareServiceTest, DoBusinessInit_001, TestSize.Level0)
{
    DistributedHardwareService service(ASID, true);
    auto ret = service.DoBusinessInit();
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedHardwareServiceTest, QueryDhSysSpec_001, TestSize.Level0)
{
    std::string targetKey = "";
    std::string attrs = "";
    DistributedHardwareService service(ASID, true);
    auto ret = service.QueryDhSysSpec(targetKey, attrs);
    EXPECT_EQ(0, ret.length());
}

HWTEST_F(DistributedHardwareServiceTest, QueryDhSysSpec_002, TestSize.Level0)
{
    std::string targetKey = "histmAudEnc";
    int32_t target = 100;
    cJSON *attrJson = cJSON_CreateObject();
    if (attrJson == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(attrJson, targetKey.c_str(), target);
    char* cjson = cJSON_PrintUnformatted(attrJson);
    if (cjson == nullptr) {
        cJSON_Delete(attrJson);
        return;
    }
    std::string attrs(cjson);
    DistributedHardwareService service(ASID, true);
    auto ret = service.QueryDhSysSpec(targetKey, attrs);
    EXPECT_EQ(0, ret.length());
    cJSON_free(cjson);
    cJSON_Delete(attrJson);

    cJSON *attrJson1 = cJSON_CreateObject();
    if (attrJson1 == nullptr) {
        return;
    }
    std::string targetKeyValue = "targetKeyValue";
    cJSON_AddStringToObject(attrJson1, targetKey.c_str(), targetKeyValue.c_str());
    char* cjson1 = cJSON_PrintUnformatted(attrJson1);
    if (cjson1 == nullptr) {
        cJSON_Delete(attrJson1);
        return;
    }
    std::string attrs1(cjson1);
    ret = service.QueryDhSysSpec(targetKey, attrs1);
    EXPECT_NE(0, ret.length());
    cJSON_free(cjson1);
    cJSON_Delete(attrJson1);
}
} // namespace DistributedHardware
} // namespace OHOS
