// test/unittest/common/lowlatency/mock/mock_res_sched_client.h
#ifndef OHOS_DISTRIBUTED_HARDWARE_MOCK_RES_SCHED_CLIENT_H
#define OHOS_DISTRIBUTED_HARDWARE_MOCK_RES_SCHED_CLIENT_H

#include "res_sched_client.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace ResourceSchedule {
class MockResSchedClient {
public:
    static MockResSchedClient& GetInstance() {
        static MockResSchedClient instance;
        return instance;
    }
    MOCK_METHOD3(ReportData, void(uint32_t, int32_t, const std::unordered_map<std::string, std::string>&));
};
}
}
#endif