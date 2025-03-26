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

#include "filter_test.h"

#include <algorithm>
#include "osal/utils/util.h"
#include "av_trans_log.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;
using namespace std;

namespace OHOS {
namespace DistributedHardware {

void FilterTest::SetUp()
{
}

void FilterTest::TearDown()
{
}

void FilterTest::SetUpTestCase()
{
}

void FilterTest::TearDownTestCase()
{
}

HWTEST_F(FilterTest, LinkPipeLine_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    std::string groupId = "testGroup";
    filterAsync.LinkPipeLine(groupId);
    EXPECT_EQ(filterAsync.GetFilterType(), FilterType::FILTERTYPE_VENC);
    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.LinkPipeLine(groupId);
    EXPECT_TRUE(!filterSync.isAsyncMode_);
}

HWTEST_F(FilterTest, Prepare_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    std::unique_ptr<Media::Task> mockTask = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    filterAsync.filterTask_ = std::move(mockTask);
    Status result = filterAsync.Prepare();
    EXPECT_EQ(result, Status::OK);

    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_VENC, false);
    filterSync.filterTask_ = nullptr;
    Status result_two = filterSync.Prepare();
    EXPECT_EQ(result_two, Status::OK);
}

HWTEST_F(FilterTest, PrepareDone_001, testing::ext::TestSize.Level1)
{
    Filter filter("testFilter", FilterType::FILTERTYPE_VENC, true);
    Status result1 = filter.PrepareDone();
    EXPECT_EQ(result1, Status::OK);
    
    Filter filterYnc("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterYnc.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result2 = filterYnc.PrepareDone();
    EXPECT_EQ(result2, Status::OK);
}

HWTEST_F(FilterTest, Pause_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterAsync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result1 = filterAsync.Pause();
    EXPECT_EQ(result1, Status::OK);

    Filter filterYnc("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterYnc.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result2 = filterYnc.Pause();
    EXPECT_EQ(result2, Status::OK);
}

HWTEST_F(FilterTest, PauseDragging_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterAsync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result = filterAsync.PauseDragging();
    EXPECT_EQ(result, Status::OK);

    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_VENC, false);
    filterSync.filterTask_ = nullptr;
    auto mockNextFilter2 = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterSync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter2);
    Status result2 = filterSync.PauseDragging();
    EXPECT_EQ(result2, Status::OK);

    Filter filterYnc("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterYnc.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result3 = filterYnc.PauseDragging();
    EXPECT_EQ(result3, Status::OK);
}

HWTEST_F(FilterTest, Resume_001, testing::ext::TestSize.Level1)
{
    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterSync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result_sync = filterSync.Resume();
    EXPECT_EQ(result_sync, Status::OK);

    Filter filterYnc("testFilterYnc", FilterType::FILTERTYPE_ASINK, false);
    filterYnc.filterTask_ = nullptr;
    filterYnc.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result = filterYnc.Resume();
    EXPECT_EQ(result, Status::OK);
}

HWTEST_F(FilterTest, ResumeDragging_001, testing::ext::TestSize.Level1)
{
    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterSync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result_sync = filterSync.ResumeDragging();
    EXPECT_EQ(result_sync, Status::OK);

    Filter filterYnc("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterYnc.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result_ync = filterYnc.ResumeDragging();
    EXPECT_EQ(result_ync, Status::OK);
}

HWTEST_F(FilterTest, Stop_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    Status result_async = filterAsync.Stop();
    EXPECT_EQ(result_async, Status::OK);

    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterSync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result_sync = filterSync.Stop();
    EXPECT_EQ(result_sync, Status::OK);

    Filter filterYnc("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterYnc.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result_ync = filterYnc.Stop();
    EXPECT_EQ(result_ync, Status::OK);
}

HWTEST_F(FilterTest, Flush_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result1 = filterAsync.Flush();
    EXPECT_EQ(result1, Status::OK);

    Filter filterSync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterSync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result2 = filterSync.Flush();
    EXPECT_EQ(result2, Status::OK);
}

HWTEST_F(FilterTest, Release_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    Status result_async = filterAsync.Release();
    EXPECT_EQ(result_async, Status::OK);
    
    Filter filterSync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterSync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result_sync = filterSync.Release();
    EXPECT_EQ(result_sync, Status::OK);

    Filter filterYnc("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterYnc.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result_ync = filterYnc.Release();
    EXPECT_EQ(result_ync, Status::OK);
}

HWTEST_F(FilterTest, SetPlayRange_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterAsync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result1 = filterAsync.SetPlayRange(10, 100);

    EXPECT_EQ(result1, Status::OK);
    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_VENC, false);
    filterSync.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result2 = filterSync.SetPlayRange(20, 200);
    EXPECT_EQ(result2, Status::OK);
}

HWTEST_F(FilterTest, Preroll_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync1("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync1.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result1 = filterAsync1.Preroll();
    EXPECT_EQ(result1, Status::OK);

    Filter filterAsync2("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterAsync2.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result2 = filterAsync2.Preroll();
    EXPECT_EQ(result2, Status::OK);

    Filter filterAsync3("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    auto mockNextFilterFail = std::make_shared<Filter>("mockNextFilterFail", FilterType::FILTERTYPE_ASINK, false);
    filterAsync3.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilterFail);
    Status result3 = filterAsync3.Preroll();
    EXPECT_EQ(result3, Status::OK);
}

HWTEST_F(FilterTest, WaitPrerollDone_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync1("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync1.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result1 = filterAsync1.WaitPrerollDone(false);
    EXPECT_EQ(result1, Status::OK);

    Filter filterAsync2("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filterAsync2.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    Status result2 = filterAsync2.WaitPrerollDone(false);
    EXPECT_EQ(result2, Status::OK);

    Filter filterAsync3("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    auto mockNextFilterFail = std::make_shared<Filter>("mockNextFilterFail", FilterType::FILTERTYPE_ASINK, false);
    filterAsync3.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilterFail);
    Status result3 = filterAsync3.WaitPrerollDone(false);
    EXPECT_EQ(result3, Status::OK);
}

HWTEST_F(FilterTest, StartFilterTask_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    filterAsync.StartFilterTask();
    EXPECT_TRUE(filterAsync.filterTask_ != nullptr);

    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    filterSync.StartFilterTask();
    EXPECT_TRUE(filterSync.filterTask_ == nullptr);
}

HWTEST_F(FilterTest, PauseFilterTask_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    filterAsync.PauseFilterTask();
    EXPECT_TRUE(filterAsync.filterTask_ != nullptr);

    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    filterSync.PauseFilterTask();
    EXPECT_TRUE(filterSync.filterTask_ == nullptr);
}

HWTEST_F(FilterTest, ProcessInputBuffer_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    Status result_async = filterAsync.ProcessInputBuffer(1, 1000);
    EXPECT_EQ(result_async, Status::OK);

    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    Status result_sync = filterSync.ProcessInputBuffer(2, 2000);
    EXPECT_EQ(result_sync, Status::OK);
}

HWTEST_F(FilterTest, ProcessOutputBuffer_001, testing::ext::TestSize.Level1)
{
    Filter filterAsync("testFilterAsync", FilterType::FILTERTYPE_VENC, true);
    filterAsync.filterTask_ = std::make_unique<Media::Task>(
        "mockTask", "groupId", Media::TaskType::SINGLETON, Media::TaskPriority::HIGH, false);
    Status result_async = filterAsync.ProcessOutputBuffer(1, 1000, false, 0, -1);
    EXPECT_EQ(result_async, Status::OK);

    Filter filterSync("testFilterSync", FilterType::FILTERTYPE_ASINK, false);
    filterSync.filterTask_ = nullptr;
    Status result_sync = filterSync.ProcessOutputBuffer(2, 2000, false, 0, -1);
    EXPECT_EQ(result_sync, Status::OK);
}

HWTEST_F(FilterTest, WaitAllState_001, testing::ext::TestSize.Level1)
{
    Filter filter("testFilter", FilterType::FILTERTYPE_VENC, true);
    filter.ChangeState(FilterState::CREATED);
    Status result1 = filter.WaitAllState(FilterState::RUNNING);
    EXPECT_EQ(result1, Status::OK);

    filter.ChangeState(FilterState::RUNNING);
    Status result2 = filter.WaitAllState(FilterState::RUNNING);
    EXPECT_EQ(result2, Status::OK);

    filter.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(nullptr);
    Status result3 = filter.WaitAllState(FilterState::RUNNING);
    EXPECT_EQ(result3, Status::OK);

    auto mockNextFilter = std::make_shared<Filter>("mockNextFilter", FilterType::FILTERTYPE_ASINK, false);
    filter.nextFiltersMap_[StreamType::STREAMTYPE_ENCODED_AUDIO].push_back(mockNextFilter);
    mockNextFilter->ChangeState(FilterState::RUNNING);
    Status result4 = filter.WaitAllState(FilterState::RUNNING);
    EXPECT_EQ(result4, Status::OK);
}
} // namespace DistributedHardware
} // namespace OHOS