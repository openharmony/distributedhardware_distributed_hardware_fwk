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

#include "av_trans_log.h"

#include "securec.h"

#ifdef HI_LOG_ENABLE
#include "hilog/log.h"
#else
#include <cstdio>
#endif

namespace OHOS {
namespace DistributedHardware {
const std::string DAVTRANS_LOG_TITLE_TAG = "DAVTRANS";
constexpr int32_t AV_LOG_MAX_LEN = 4096;

static void AVTransLogOut(DHLogLevel logLevel, const char *logBuf)
{
#ifdef HI_LOG_ENABLE
    LogLevel hiLogLevel = LOG_INFO;
    switch (logLevel) {
        case DH_LOG_DEBUG:
            hiLogLevel = LOG_DEBUG;
            break;
        case DH_LOG_INFO:
            hiLogLevel = LOG_INFO;
            break;
        case DH_LOG_WARN:
            hiLogLevel = LOG_WARN;
            break;
        case DH_LOG_ERROR:
            hiLogLevel = LOG_ERROR;
            break;
        default:
            break;
    }
    (void)HiLogPrint(LOG_CORE, hiLogLevel, LOG_DOMAIN, DAVTRANS_LOG_TITLE_TAG.c_str(), "%{public}s", logBuf);
#else
    switch (logLevel) {
        case DH_LOG_DEBUG:
            printf("[D]%s\n", logBuf);
            break;
        case DH_LOG_INFO:
            printf("[I]%s\n", logBuf);
            break;
        case DH_LOG_WARN:
            printf("[W]%s\n", logBuf);
            break;
        case DH_LOG_ERROR:
            printf("[E]%s\n", logBuf);
            break;
        default:
            break;
    }
#endif
}

void AVTransLog(DHLogLevel logLevel, const char *fmt, ...)
{
    char logBuf[AV_LOG_MAX_LEN] = {0};
    va_list arg;

    (void)memset_s(&arg, sizeof(va_list), 0, sizeof(va_list));
    va_start(arg, fmt);
    int32_t ret = vsprintf_s(logBuf, sizeof(logBuf), fmt, arg);
    va_end(arg);
    if (ret < 0) {
        AVTransLogOut(logLevel, "AVTrans log length error.");
        return;
    }
    AVTransLogOut(logLevel, logBuf);
}

std::string GetAnonyString(const std::string &value)
{
    constexpr size_t INT32_SHORT_ID_LENGTH = 20;
    constexpr size_t INT32_PLAINTEXT_LENGTH = 4;
    constexpr size_t INT32_MIN_ID_LENGTH = 3;
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

std::string GetAnonyInt32(const int32_t value)
{
    constexpr int32_t INT32_STRING_LENGTH = 40;
    char tempBuffer[INT32_STRING_LENGTH] = "";
    int32_t secRet = sprintf_s(tempBuffer, INT32_STRING_LENGTH, "%d", value);
    if (secRet <= 0) {
        std::string nullString("");
        return nullString;
    }
    size_t length = strlen(tempBuffer);
    for (size_t i = 1; i <= length - 1; i++) {
        tempBuffer[i] = '*';
    }
    if (length == 0x01) {
        tempBuffer[0] = '*';
    }

    std::string tempString(tempBuffer);
    return tempString;
}
} // namespace DistributedHardware
} // namespace OHOS