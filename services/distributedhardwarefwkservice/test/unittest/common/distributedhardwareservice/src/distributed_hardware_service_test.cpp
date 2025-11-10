/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "component_loader.h"
#include "component_manager.h"
#include "dh_context.h"
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"
#include "distributed_hardware_service.h"
#include "distributed_hardware_manager.h"
#include "local_capability_info_manager.h"
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

    constexpr int32_t TEST_COMP_SINK_SA_ID = 4804;
    constexpr int32_t TEST_SINK_SA_ID = 12345;
    constexpr int32_t TEST_SOURCE_SA_ID = 12345;
    constexpr uint16_t DEV_TYPE_TEST = 14;
}
void DistributedHardwareServiceTest::SetUpTestCase(void) {}

void DistributedHardwareServiceTest::TearDownTestCase(void) {}

void DistributedHardwareServiceTest::SetUp() {}

void DistributedHardwareServiceTest::TearDown() {}

void SetUpComponentLoaderConfig()
{
    if (ComponentLoader::GetInstance().compHandlerMap_.find(DHType::AUDIO)
        == ComponentLoader::GetInstance().compHandlerMap_.end()) {
        CompHandler handler;
        handler.compConfig.name = "distributed_audio";
        handler.compConfig.type = DHType::AUDIO;
        handler.compConfig.compHandlerLoc = "libdistributed_audio_handler.z.so";
        handler.compConfig.compHandlerVersion = "1.0";
        handler.compConfig.compSourceLoc = "libdistributed_audio_source_sdk.z.so";
        handler.compConfig.compSourceVersion = "1.0";
        handler.compConfig.compSinkLoc = "libdistributed_audio_sink_sdk.z.so";
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

void DistributedHardwareServiceTest::TestGetDistributedHardwareCallback::OnSuccess(
    const std::string &networkId, const std::vector<DHDescriptor> &descriptors, EnableStep enableStep)
{
    (void)networkId;
    (void)descriptors;
    (void)enableStep;
}

void DistributedHardwareServiceTest::TestGetDistributedHardwareCallback::OnError(const std::string &networkId,
    int32_t error)
{
    (void)networkId;
    (void)error;
}

/**
 * @tc.name: register_publisher_listener_001
 * @tc.desc: Verify the RegisterPublisherListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, register_publisher_listener_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, register_publisher_listener_002, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, unregister_publisher_listener_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, publish_message_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, publish_message_002, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string msg = "msg";
    service.RegisterPublisherListener(TOPIC, g_listener);
    auto ret = service.PublishMessage(TOPIC, msg);

    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

HWTEST_F(DistributedHardwareServiceTest, publish_message_003, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string msg = "msg";
    DHTopic topic = DHTopic::TOPIC_CREATE_SESSION_READY;
    auto ret = service.PublishMessage(topic, msg);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: onStop_test_002
 * @tc.desc: Verify the OnStop function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, onStop_test_002, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, dump_test_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, OnStart_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, QueryLocalSysSpec_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, PauseDistributedHardware_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, ResumeDistributedHardware_001, TestSize.Level1)
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
HWTEST_F(DistributedHardwareServiceTest, StopDistributedHardware_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.PauseDistributedHardware(DHType::UNKNOWN, networkId);
    ret = service.PauseDistributedHardware(DHType::INPUT, networkId);
    ret = service.PauseDistributedHardware(DHType::MAX_DH, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

/**
 * @tc.name: GetDistributedHardware_001
 * @tc.desc: Verify the GetDistributedHardware function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, GetDistributedHardware_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "";
    EnableStep enableSourceStep = EnableStep::ENABLE_SOURCE;
    EnableStep enableSinkStep = EnableStep::ENABLE_SINK;
    DistributedHardwareManager::GetInstance().isAllInit_.store(true);
    auto ret = service.GetDistributedHardware(networkId, enableSourceStep, nullptr);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);

    networkId = "111";
    ret = service.GetDistributedHardware(networkId, enableSourceStep, nullptr);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);

    DistributedHardwareManager::GetInstance().isAllInit_.store(false);
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    ret = service.GetDistributedHardware(networkId, enableSourceStep, callback);
    EXPECT_EQ(ret, ERR_DH_FWK_HARDWARE_MANAGER_BUSY);

    ret = service.GetDistributedHardware(networkId, enableSinkStep, callback);
    EXPECT_EQ(ret, ERR_DH_FWK_HARDWARE_MANAGER_BUSY);

    DistributedHardwareManager::GetInstance().isAllInit_.store(true);
    ret = service.GetDistributedHardware(networkId, enableSourceStep, callback);
    EXPECT_EQ(ret, ERR_DH_FWK_HARDWARE_MANAGER_BUSY);

    DistributedHardwareManager::GetInstance().isAllInit_.store(false);
    DHContext::GetInstance().AddRealTimeOnlineDeviceNetworkId(networkId);
    ret = service.GetDistributedHardware(networkId, enableSourceStep, callback);
    EXPECT_EQ(ret, ERR_DH_FWK_HARDWARE_MANAGER_BUSY);

    ret = service.GetDistributedHardware(networkId, enableSinkStep, callback);
    EXPECT_EQ(ret, ERR_DH_FWK_HARDWARE_MANAGER_BUSY);
}

HWTEST_F(DistributedHardwareServiceTest, GetDistributedHardware_002, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "networkId_1";
    EnableStep enableSourceStep = EnableStep::ENABLE_SOURCE;
    EnableStep enableSinkStep = EnableStep::ENABLE_SINK;
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    DistributedHardwareManager::GetInstance().isAllInit_.store(true);
    auto ret = service.GetDistributedHardware(networkId, enableSinkStep, callback);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    DHContext::GetInstance().AddRealTimeOnlineDeviceNetworkId(networkId);
    ret = service.GetDistributedHardware(networkId, enableSourceStep, callback);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    networkId = "local";
    DHContext::GetInstance().AddRealTimeOnlineDeviceNetworkId(networkId);
    ret = service.GetDistributedHardware(networkId, enableSourceStep, callback);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: RegisterDHStatusListener_001
 * @tc.desc: Verify the RegisterDHStatusListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, RegisterDHStatusListener_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    sptr<IHDSinkStatusListener> listener = nullptr;

    SetUpComponentLoaderConfig();
    auto ret = service.RegisterDHStatusListener(listener);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.RegisterDHStatusListener(listener);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_REPEAT_CALL);
}

/**
 * @tc.name: RegisterDHStatusListener_002
 * @tc.desc: Verify the RegisterDHStatusListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, RegisterDHStatusListener_002, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    sptr<IHDSourceStatusListener> listener = nullptr;

    SetUpComponentLoaderConfig();
    std::string networkId = "111";
    auto ret = service.RegisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.RegisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_REPEAT_CALL);
    SetDownComponentLoaderConfig();

    networkId = "";
    ret = service.RegisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

/**
 * @tc.name: UnregisterDHStatusListener_001
 * @tc.desc: Verify the UnregisterDHStatusListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, UnregisterDHStatusListener_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    sptr<IHDSinkStatusListener> listener = nullptr;

    SetUpComponentLoaderConfig();
    auto ret = service.UnregisterDHStatusListener(listener);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.UnregisterDHStatusListener(listener);
    SetDownComponentLoaderConfig();
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_REPEAT_CALL);
}

/**
 * @tc.name: UnregisterDHStatusListener_002
 * @tc.desc: Verify the UnregisterDHStatusListener function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, UnregisterDHStatusListener_002, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    sptr<IHDSourceStatusListener> listener = nullptr;

    SetUpComponentLoaderConfig();
    std::string networkId = "111";
    auto ret = service.UnregisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
    ret = service.UnregisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ret, ERR_DH_FWK_COMPONENT_REPEAT_CALL);
    SetDownComponentLoaderConfig();

    networkId = "";
    ret = service.UnregisterDHStatusListener(networkId, listener);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

/**
 * @tc.name: EnableSink_001
 * @tc.desc: Verify the EnableSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, EnableSink_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::vector<DHDescriptor> descriptors{{"id_test", DHType::AUDIO}};

    auto ret = service.EnableSink(descriptors);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: DisableSink_001
 * @tc.desc: Verify the DisableSink function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, DisableSink_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::vector<DHDescriptor> descriptors{{"id_test", DHType::AUDIO}};

    auto ret = service.DisableSink(descriptors);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);
}

/**
 * @tc.name: EnableSource_001
 * @tc.desc: Verify the EnableSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, EnableSource_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    std::vector<DHDescriptor> descriptors{{"id_test", DHType::AUDIO}};

    auto ret = service.EnableSource(networkId, descriptors);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    networkId = "";
    ret = service.EnableSource(networkId, descriptors);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

/**
 * @tc.name: DisableSource_001
 * @tc.desc: Verify the DisableSource function
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DistributedHardwareServiceTest, DisableSource_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    std::vector<DHDescriptor> descriptors{{"id_test", DHType::AUDIO}};

    auto ret = service.DisableSource(networkId, descriptors);
    EXPECT_EQ(ret, DH_FWK_SUCCESS);

    networkId = "";
    ret = service.DisableSource(networkId, descriptors);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(DistributedHardwareServiceTest, PauseDistributedHardware_003, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(dhType, sinkPtr));
    auto ret = service.PauseDistributedHardware(dhType, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
    ComponentManager::GetInstance().compSink_.clear();
}

HWTEST_F(DistributedHardwareServiceTest, PauseDistributedHardware_004, TestSize.Level1)
{
    SetUpComponentLoaderConfig();
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    DHType dhType = DHType::AUDIO;
    ComponentManager::GetInstance().InitCompSink(dhType);
    auto ret = service.PauseDistributedHardware(dhType, networkId);
    EXPECT_NE(ret, DH_FWK_SUCCESS);
    SetDownComponentLoaderConfig();
    ComponentManager::GetInstance().compSink_.clear();
}

HWTEST_F(DistributedHardwareServiceTest, ResumeDistributedHardware_002, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.ResumeDistributedHardware(DHType::UNKNOWN, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(DistributedHardwareServiceTest, ResumeDistributedHardware_003, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(dhType, sinkPtr));
    auto ret = service.ResumeDistributedHardware(dhType, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
    ComponentManager::GetInstance().compSink_.clear();
}

HWTEST_F(DistributedHardwareServiceTest, ResumeDistributedHardware_004, TestSize.Level1)
{
    SetUpComponentLoaderConfig();
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    DHType dhType = DHType::AUDIO;
    ComponentManager::GetInstance().InitCompSink(dhType);
    auto ret = service.ResumeDistributedHardware(dhType, networkId);
    EXPECT_NE(ret, DH_FWK_SUCCESS);
    SetDownComponentLoaderConfig();
    ComponentManager::GetInstance().compSink_.clear();
}

HWTEST_F(DistributedHardwareServiceTest, StopDistributedHardware_002, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    auto ret = service.StopDistributedHardware(DHType::UNKNOWN, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
}

HWTEST_F(DistributedHardwareServiceTest, StopDistributedHardware_003, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    DHType dhType = DHType::AUDIO;
    IDistributedHardwareSink *sinkPtr = nullptr;
    ComponentManager::GetInstance().compSink_.insert(std::make_pair(dhType, sinkPtr));
    auto ret = service.StopDistributedHardware(dhType, networkId);
    EXPECT_EQ(ret, ERR_DH_FWK_PARA_INVALID);
    ComponentManager::GetInstance().compSink_.clear();
}

HWTEST_F(DistributedHardwareServiceTest, StopDistributedHardware_004, TestSize.Level1)
{
    SetUpComponentLoaderConfig();
    DistributedHardwareService service(ASID, true);
    std::string networkId = "111";
    DHType dhType = DHType::AUDIO;
    ComponentManager::GetInstance().InitCompSink(dhType);
    auto ret = service.StopDistributedHardware(dhType, networkId);
    EXPECT_NE(ret, DH_FWK_SUCCESS);
    SetDownComponentLoaderConfig();
    ComponentManager::GetInstance().compSink_.clear();
}

HWTEST_F(DistributedHardwareServiceTest, DoBusinessInit_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    auto ret = service.DoBusinessInit();
    EXPECT_EQ(true, ret);
}

HWTEST_F(DistributedHardwareServiceTest, QueryDhSysSpec_001, TestSize.Level1)
{
    std::string targetKey = "";
    std::string attrs = "";
    DistributedHardwareService service(ASID, true);
    auto ret = service.QueryDhSysSpec(targetKey, attrs);
    EXPECT_EQ(0, ret.length());
}

HWTEST_F(DistributedHardwareServiceTest, QueryDhSysSpec_002, TestSize.Level1)
{
    std::string targetKey = "histmAudEnc";
    int32_t target = 100;
    cJSON *attrJson = cJSON_CreateObject();
    ASSERT_TRUE(attrJson != nullptr);
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
    ASSERT_TRUE(attrJson1 != nullptr);
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

HWTEST_F(DistributedHardwareServiceTest, LoadDistributedHDF_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    service.LoadDistributedHDF(DHType::AUDIO);
    service.LoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_FWK_NO_HDF_SUPPORT, service.LoadDistributedHDF(DHType::UNKNOWN));
    service.UnLoadDistributedHDF(DHType::AUDIO);
    service.UnLoadDistributedHDF(DHType::CAMERA);
    EXPECT_EQ(ERR_DH_FWK_NO_HDF_SUPPORT, service.UnLoadDistributedHDF(DHType::UNKNOWN));
}

HWTEST_F(DistributedHardwareServiceTest, GetDeviceDhInfo_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string realNetworkId = "";
    std::string udidHash = "";
    std::string deviceId = "";
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    sptr<IGetDhDescriptorsCallback> callback = nullptr;
    auto ret = service.GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    realNetworkId = "networkId_1";
    ret = service.GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    udidHash = "udidHash_1";
    ret = service.GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    deviceId = "deviceId_1";
    realNetworkId = "";
    ret = service.GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    realNetworkId = "networkId_1";
    udidHash = "";
    ret = service.GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_PARA_INVALID, ret);

    udidHash = "udidHash_1";
    ret = service.GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    EXPECT_EQ(ERR_DH_FWK_HARDWARE_MANAGER_GET_DHINFO_FAIL, ret);
}

HWTEST_F(DistributedHardwareServiceTest, GetDeviceDhInfo_002, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string realNetworkId = "networkId_1";
    std::string udidHash = "udidHash_1";
    std::string deviceId = "deviceId_1";
    EnableStep enableStep = EnableStep::ENABLE_SOURCE;
    sptr<IGetDhDescriptorsCallback> callback(new TestGetDistributedHardwareCallback());
    std::shared_ptr<CapabilityInfo> capInfo = std::make_shared<CapabilityInfo>(
        "dhId_1", deviceId, "devName_1", DEV_TYPE_TEST, DHType::AUDIO, "attrs", "subtype");
    std::string key = deviceId + "###" + "dhId_1";
    LocalCapabilityInfoManager::GetInstance()->globalCapInfoMap_[key] = capInfo;
    auto ret = service.GetDeviceDhInfo(realNetworkId, udidHash, deviceId, enableStep, callback);
    EXPECT_EQ(DH_FWK_SUCCESS, ret);
}

HWTEST_F(DistributedHardwareServiceTest, LoadSinkDMSDPService_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string udid = "";
    auto ret = service.LoadSinkDMSDPService(udid);
    EXPECT_EQ(ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL, ret);
}

HWTEST_F(DistributedHardwareServiceTest, NotifySinkRemoteSourceStarted_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string udid = "";
    auto ret = service.NotifySinkRemoteSourceStarted(udid);
    EXPECT_EQ(ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL, ret);
}

HWTEST_F(DistributedHardwareServiceTest, NotifySourceRemoteSinkStarted_001, TestSize.Level1)
{
    DistributedHardwareService service(ASID, true);
    std::string udid = "";
    auto ret = service.NotifySourceRemoteSinkStarted(udid);
    EXPECT_EQ(ERR_DH_FWK_ACCESS_PERMISSION_CHECK_FAIL, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
