/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "pipeline_test.h"

#include <queue>
#include <stack>

#include "osal/task/autolock.h"
#include "osal/task/jobutils.h"

#include "av_trans_log.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace DistributedHardware {
namespace Pipeline {
void PipelineTest::SetUp()
{
}

void PipelineTest::TearDown()
{
}

void PipelineTest::SetUpTestCase()
{
}

void PipelineTest::TearDownTestCase()
{
}

HWTEST_F(PipelineTest, Prepare_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Prepare();
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Prepare();
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, Start_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Start();
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Start();
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, Pause_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Pause();
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Pause();
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, Resume_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Resume();
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Resume();
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, Stop_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Stop();
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Stop();
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, Flush_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Flush();
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Flush();
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, Release_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Release();
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Release();
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, Preroll_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.Preroll(true);
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.Preroll(false);
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, SetPlayRange_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    pipeline.filters_.emplace_back(nullptr);
    Status ret1 = pipeline.SetPlayRange(0, 10);
    EXPECT_EQ(ret1, Status::OK);

    pipeline.filters_.clear();
    auto mockFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(mockFilter);
    Status ret2 = pipeline.SetPlayRange(10, 20);
    EXPECT_EQ(ret2, Status::OK);
}

HWTEST_F(PipelineTest, AddHeadFilters_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    std::vector<std::shared_ptr<Filter>> filtersIn;
    filtersIn.emplace_back(nullptr);

    auto validFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    filtersIn.emplace_back(validFilter);
    Status ret = pipeline.AddHeadFilters(filtersIn);
    EXPECT_EQ(ret, Status::OK);
    EXPECT_EQ(pipeline.filters_.size(), 1);
    EXPECT_NE(pipeline.filters_[0], nullptr);
}

HWTEST_F(PipelineTest, RemoveHeadFilter_001, testing::ext::TestSize.Level1)
{
    Pipeline pipeline;
    std::shared_ptr<EventReceiver> receiver = nullptr;
    std::shared_ptr<FilterCallback> callback = nullptr;
    std::string groupId = "test_group";
    pipeline.Init(receiver, callback, groupId);

    auto validFilter = std::make_shared<Filter>("testFilter", FilterType::FILTERTYPE_VENC, true);
    pipeline.filters_.emplace_back(validFilter);

    Status ret1 = pipeline.RemoveHeadFilter(validFilter);
    EXPECT_EQ(ret1, Status::OK);
    EXPECT_EQ(pipeline.filters_.size(), 0);

    std::shared_ptr<Filter> nullFilter = nullptr;
    Status ret2 = pipeline.RemoveHeadFilter(nullFilter);
    EXPECT_EQ(ret2, Status::OK);
}
} // namespace Pipeline
} // namespace DistributedHardware
} // namespace OHOS