/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "av_sync_utils.h"

#include <sys/mman.h>
#include <securec.h>
#include <unistd.h>
#include "ashmem.h"
#include "nlohmann/json.hpp"

#include "av_trans_constants.h"
#include "av_trans_errno.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
AVTransSharedMemory CreateAVTransSharedMemory(const std::string &name, size_t size)
{
    int32_t fd = AshmemCreate(name.c_str(), size);
    if (fd <= 0) {
        AVTRANS_LOGE("create av trans shared memory failed, name=%{public}s, fd=%{public}" PRId32, name.c_str(), fd);
        return AVTransSharedMemory{0, 0, name};
    }

    unsigned int prot = PROT_READ | PROT_WRITE;
    int result = AshmemSetProt(fd, static_cast<int>(prot));
    if (result < 0) {
        AVTRANS_LOGE("AshmemSetProt failed, name=%{public}s, fd=%{public}" PRId32, name.c_str(), fd);
        (void)::close(fd);
        return AVTransSharedMemory{0, 0, name};
    }

    void *addr = ::mmap(nullptr, size, static_cast<int>(prot), MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        AVTRANS_LOGE("shared memory mmap failed, name=%{public}s, fd=%{public}" PRId32, name.c_str(), fd);
        (void)::close(fd);
        return AVTransSharedMemory{0, 0, name};
    }

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    (void)memset_s(base, size, INVALID_VALUE_FALG, size);
    uint64_t tmpsize = static_cast<uint64_t>(size);
    AVTRANS_LOGI("create av trans shared memory success, name=%{public}s, size=%{public}" PRIu64 ", fd=%{public}"
        PRId32, name.c_str(), tmpsize, fd);
    return AVTransSharedMemory{fd, size, name};
}

void CloseAVTransSharedMemory(const AVTransSharedMemory &memory) noexcept
{
    AVTRANS_LOGI("close shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    if (IsInValidSharedMemory(memory)) {
        AVTRANS_LOGE("invalid input shared memory");
        return;
    }
    if (memory.fd > 0) {
        (void)::close(memory.fd);
    }
}

int32_t WriteClockUnitToMemory(const AVTransSharedMemory &memory, AVSyncClockUnit &clockUnit)
{
    AVTRANS_LOGI("write clock unit to shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    AVTRANS_LOGI("clock unit index=%{public}" PRId32 ", frameNum=%{public}" PRId32 ", pts=%{public}lld",
        clockUnit.index, clockUnit.frameNum, (long long)clockUnit.pts);
    TRUE_RETURN_V_MSG_E(IsInValidClockUnit(clockUnit), ERR_DH_AVT_INVALID_PARAM, "invalid input clock unit");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    if (addr == MAP_FAILED) {
        free(addr);
        addr = nullptr;
        AVTRANS_LOGE("shared memory mmap failed, mmap address is invalid.");
        return ERR_DH_AVT_SHARED_MEMORY_FAILED;
    }

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    size_t fOffset = (sizeof(uint32_t) + sizeof(int64_t)) * clockUnit.index;
    size_t tOffset = fOffset + sizeof(uint32_t);
    U64ToU8(base + tOffset, clockUnit.pts);
    U32ToU8(base + fOffset, clockUnit.frameNum);

    clockUnit.index ++;
    if (clockUnit.index == MAX_CLOCK_UNIT_COUNT) {
        clockUnit.index = 0;
    }

    AVTRANS_LOGI("write clock unit frameNum=%{public}" PRId32 ", pts=%{public}lld to shared memory success",
        clockUnit.frameNum, (long long)(clockUnit.pts));
    return DH_AVT_SUCCESS;
}

int32_t ReadClockUnitFromMemory(const AVTransSharedMemory &memory, AVSyncClockUnit &clockUnit)
{
    AVTRANS_LOGI("read clock unit from shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    AVTRANS_LOGI("clock unit index=%{public}" PRId32 ", frameNum=%{public}" PRId32,
        clockUnit.index, clockUnit.frameNum);
    TRUE_RETURN_V_MSG_E((clockUnit.frameNum <= 0), ERR_DH_AVT_INVALID_PARAM, "invalid input frame number");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    if (addr == MAP_FAILED) {
        free(addr);
        addr = nullptr;
        AVTRANS_LOGE("shared memory mmap failed, mmap address is invalid.");
        return ERR_DH_AVT_SHARED_MEMORY_FAILED;
    }

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    uint32_t firstUnit = U8ToU32(base);
    TRUE_RETURN_V_MSG_E(firstUnit == 0, ERR_DH_AVT_MASTER_NOT_READY, "master queue not ready, clock is null.");

    uint32_t index = 0;
    int64_t latestPts = 0;
    size_t unitSize = sizeof(uint32_t) + sizeof(int64_t);
    while (index < MAX_CLOCK_UNIT_COUNT) {
        uint32_t frameNum = U8ToU32(base + (index * unitSize));
        int64_t pts = static_cast<int64_t>(U8ToU64(base + (index * unitSize) + sizeof(uint32_t)));
        if (pts > latestPts) {
            latestPts = pts;
            clockUnit.pts = pts;
            clockUnit.frameNum = frameNum;
        }
        index++;
    }
    AVTRANS_LOGI("read clock unit from shared memory success, frameNum=%{public}" PRId32 ", pts=%{public}lld",
        clockUnit.frameNum, (long long)clockUnit.pts);
    return DH_AVT_SUCCESS;
}

int32_t WriteFrameInfoToMemory(const AVTransSharedMemory &memory, uint32_t frameNum, int64_t timestamp)
{
    AVTRANS_LOGI("write frame info to shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    TRUE_RETURN_V_MSG_E((frameNum <= 0), ERR_DH_AVT_INVALID_PARAM, "invalid input frame number");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    if (addr == MAP_FAILED) {
        free(addr);
        addr = nullptr;
        AVTRANS_LOGE("shared memory mmap failed, mmap address is invalid.");
        return ERR_DH_AVT_SHARED_MEMORY_FAILED;
    }

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    U32ToU8(base, frameNum);
    U64ToU8(base + sizeof(uint32_t), timestamp);

    AVTRANS_LOGI("write frameNum=%{public}" PRId32 ", timestamp=%{public}lld to shared memory success",
        frameNum, (long long)timestamp);
    return DH_AVT_SUCCESS;
}

int32_t ReadFrameInfoFromMemory(const AVTransSharedMemory &memory, uint32_t &frameNum, int64_t &timestamp)
{
    AVTRANS_LOGI("read frame info from shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    if (addr == MAP_FAILED) {
        free(addr);
        addr = nullptr;
        AVTRANS_LOGE("shared memory mmap failed, mmap address is invalid.");
        return ERR_DH_AVT_SHARED_MEMORY_FAILED;
    }

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    frameNum = U8ToU32(base);
    timestamp = static_cast<int64_t>(U8ToU64(base + sizeof(uint32_t)));
    TRUE_RETURN_V_MSG_E(frameNum <= 0, ERR_DH_AVT_MASTER_NOT_READY, "master queue not ready, frameNum is null.");

    AVTRANS_LOGI("read frameNum=%{public}" PRId32 ", timestamp=%{public}lld from shared memory success.", frameNum,
        (long long)timestamp);
    return DH_AVT_SUCCESS;
}

int32_t ResetSharedMemory(const AVTransSharedMemory &memory)
{
    AVTRANS_LOGI("reset shared memory, name=%{public}s, size=%{public}" PRId32 ", fd=%{public}" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %{public}" PRId32,
        size);

    unsigned int prot = PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    if (addr == MAP_FAILED) {
        free(addr);
        addr = nullptr;
        AVTRANS_LOGE("shared memory mmap failed, mmap address is invalid.");
        return ERR_DH_AVT_SHARED_MEMORY_FAILED;
    }
    (void)memset_s(reinterpret_cast<uint8_t*>(addr), size, INVALID_VALUE_FALG, size);

    AVTRANS_LOGI("reset shared memory success.");
    return DH_AVT_SUCCESS;
}

bool IsInValidSharedMemory(const AVTransSharedMemory &memory)
{
    return (memory.fd <= 0) || (memory.size <= 0) || memory.name.empty();
}

bool IsInValidClockUnit(const AVSyncClockUnit &clockUnit)
{
    return (clockUnit.index < 0) || (clockUnit.index >= MAX_CLOCK_UNIT_COUNT) || (clockUnit.frameNum <= 0)
        || (clockUnit.pts <= 0);
}

std::string MarshalSharedMemory(const AVTransSharedMemory &memory)
{
    nlohmann::json memoryJson;

    memoryJson[KEY_SHARED_MEM_FD] = memory.fd;
    memoryJson[KEY_SHARED_MEM_SIZE] = memory.size;
    memoryJson[KEY_SHARED_MEM_NAME] = memory.name;

    return memoryJson.dump();
}

AVTransSharedMemory UnmarshalSharedMemory(const std::string &jsonStr)
{
    nlohmann::json paramJson = nlohmann::json::parse(jsonStr, nullptr, false);
    if (paramJson.is_discarded()) {
        return AVTransSharedMemory{0, 0, ""};
    }

    if (!paramJson.contains(KEY_SHARED_MEM_FD) || !paramJson.contains(KEY_SHARED_MEM_SIZE) ||
        !paramJson.contains(KEY_SHARED_MEM_NAME)) {
        return AVTransSharedMemory{0, 0, ""};
    }

    if (!paramJson[KEY_SHARED_MEM_FD].is_number_integer() || !paramJson[KEY_SHARED_MEM_SIZE].is_number_integer() ||
        !paramJson[KEY_SHARED_MEM_NAME].is_string()) {
        return AVTransSharedMemory{0, 0, ""};
    }

    int32_t fd = paramJson[KEY_SHARED_MEM_FD].get<int32_t>();
    int32_t size = paramJson[KEY_SHARED_MEM_SIZE].get<int32_t>();
    std::string name = paramJson[KEY_SHARED_MEM_NAME].get<std::string>();

    return AVTransSharedMemory{ fd, size, name };
}

void U32ToU8(uint8_t *ptr, uint32_t value)
{
    ptr[0] = (uint8_t)((value) & 0xff);
    ptr[1] = (uint8_t)((value >> 8) & 0xff);
    ptr[2] = (uint8_t)((value >> 16) & 0xff);
    ptr[3] = (uint8_t)((value >> 24) & 0xff);
}

void U64ToU8(uint8_t *ptr, uint64_t value)
{
    ptr[0] = (uint8_t)((value) & 0xff);
    ptr[1] = (uint8_t)((value >> 8) & 0xff);
    ptr[2] = (uint8_t)((value >> 16) & 0xff);
    ptr[3] = (uint8_t)((value >> 24) & 0xff);
    ptr[4] = (uint8_t)((value >> 32) & 0xff);
    ptr[5] = (uint8_t)((value >> 40) & 0xff);
    ptr[6] = (uint8_t)((value >> 48) & 0xff);
    ptr[7] = (uint8_t)((value >> 56) & 0xff);
}

uint32_t U8ToU32(const uint8_t *ptr)
{
    return (((uint32_t)(ptr[0] & 0xff)) |
            ((uint32_t)(ptr[1] & 0xff) << 8) |
            ((uint32_t)(ptr[2] & 0xff) << 16) |
            ((uint32_t)(ptr[3] & 0xff) << 24));
}

uint64_t U8ToU64(const uint8_t *ptr)
{
    return (((uint64_t)(ptr[0] & 0xff)) |
            ((uint64_t)(ptr[1] & 0xff) << 8) |
            ((uint64_t)(ptr[2] & 0xff) << 16) |
            ((uint64_t)(ptr[3] & 0xff) << 24) |
            ((uint64_t)(ptr[4] & 0xff) << 32) |
            ((uint64_t)(ptr[5] & 0xff) << 40) |
            ((uint64_t)(ptr[6] & 0xff) << 48) |
            ((uint64_t)(ptr[7] & 0xff) << 56));
}
} // namespace DistributedHardware
} // namespace OHOS