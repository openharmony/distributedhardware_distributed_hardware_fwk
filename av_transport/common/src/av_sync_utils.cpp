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

#include "av_trans_errno.h"
#include "av_trans_log.h"

namespace OHOS {
namespace DistributedHardware {
AVTransSharedMemory CreateAVTransSharedMemory(const std::string &name, size_t size)
{
    int32_t fd = AshmemCreate(name.c_str(), size);
    if (fd <= 0) {
        DHLOGE("create av trans shared memory failed, name=%s, fd=%" PRId32, name.c_str(), fd);
        return AVTransSharedMemory{0, 0, name};
    }

    unsigned int prot = PROT_READ | PROT_WRITE;
    int result = AshmemSetProt(fd, static_cast<int>(prot));
    if (result < 0) {
        DHLOGE("AshmemSetProt failed, name=%s, fd=%" PRId32, name.c_str(), fd);
        (void)::close(fd);
        return AVTransSharedMemory{0, 0, name};
    }

    void *addr = ::mmap(nullptr, size, static_cast<int>(prot), MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        DHLOGE("shared memory mmap failed, name=%s, fd=%" PRId32, name.c_str(), fd);
        (void)::close(fd);
        return AVTransSharedMemory{0, 0, name};
    }

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    (void)memset_s(base, size, INVALID_VALUE_FALG, size);

    DHLOGI("create av trans shared memory success, name=%s, size=%" PRId32 ", fd=%" PRId32, name.c_str(), size, fd);
    return AVTransSharedMemory{fd, size, name};
}

void CloseAVTransSharedMemory(const AVTransSharedMemory &memory) noexcept
{
    DHLOGI("close shared memory, name=%s, size=%" PRId32 ", fd=%" PRId32, memory.name.c_str(), memory.size, memory.fd);
    if (IsInValidSharedMemory(memory)) {
        DHLOGE("invalid input shared memory");
        return;
    }
    if (memory.fd > 0) {
        (void)::close(memory.fd);
    }
}

int32_t WriteClockUnitToMemory(const AVTransSharedMemory &memory, AVSyncClockUnit &clockUnit)
{
    DHLOGI("write clock unit to shared memory, name=%s, size=%" PRId32 ", fd=%" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    DHLOGI("clock unit index=%" PRId32 ", frameNum=%" PRId32 ", timestamp=%lld", clockUnit.index,
        clockUnit.frameNum, (long long)clockUnit.timestamp);
    TRUE_RETURN_V_MSG_E(IsInValidClockUnit(clockUnit), ERR_DH_AVT_INVALID_PARAM, "invalid input clock unit");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %" PRId32, size);

    unsigned int prot = PROT_READ | PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    TRUE_RETURN_V_MSG_E(addr == MAP_FAILED, ERR_DH_AVT_SHARED_MEMORY_FAILED, "shared memory mmap failed");

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    size_t fOffset = (sizeof(uint32_t) + sizeof(int64_t)) * clockUnit.index;
    size_t tOffset = fOffset + sizeof(uint32_t);
    U64ToU8(base + tOffset, clockUnit.timestamp);
    U32ToU8(base + fOffset, clockUnit.frameNum);

    clockUnit.index ++;
    if (clockUnit.index == MAX_CLOCK_UNIT_COUNT) {
        clockUnit.index = 0;
    }
    return DH_AVT_SUCCESS;
}

int32_t ReadClockUnitFromMemory(const AVTransSharedMemory &memory, AVSyncClockUnit &clockUnit)
{
    DHLOGI("read clock unit from shared memory, name=%s, size=%" PRId32 ", fd=%" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    DHLOGI("clock unit index=%" PRId32 ", frameNum=%" PRId32, clockUnit.index, clockUnit.frameNum);
    TRUE_RETURN_V_MSG_E((clockUnit.frameNum <= 0), ERR_DH_AVT_INVALID_PARAM, "invalid input frame number");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %" PRId32, size);

    unsigned int prot = PROT_READ;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    TRUE_RETURN_V_MSG_E(addr == MAP_FAILED, ERR_DH_AVT_SHARED_MEMORY_FAILED, "shared memory mmap failed");

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    uint32_t firstUnit = U8ToU32(base);
    TRUE_RETURN_V_MSG_E(firstUnit == 0, ERR_DH_AVT_MASTER_NOT_READY, "master queue not ready, clock is null.");

    uint32_t index = 0;
    size_t unitSize = sizeof(uint32_t) + sizeof(int64_t);
    while (index < MAX_CLOCK_UNIT_COUNT) {
        uint32_t frameNum = U8ToU32(base + (index * unitSize));
        if (frameNum == clockUnit.frameNum) {
            clockUnit.timestamp = U8ToU64(base + (index * unitSize) + sizeof(uint32_t));
            DHLOGI("read clock unit from shared memory success, frameNum=%" PRId32 ", timestamp=%lld",
                clockUnit.frameNum, (long long)clockUnit.timestamp);
            return DH_AVT_SUCCESS;
        }
        index++;
    }

    DHLOGE("read clock unit from shared memory failed for frameNum=%" PRId32, clockUnit.frameNum);
    return ERR_DH_AVT_SHARED_MEMORY_FAILED;
}

int32_t WriteFrameInfoToMemory(const AVTransSharedMemory &memory, uint32_t frameNum, int64_t timestamp)
{
    DHLOGI("write frame number to shared memory, name=%s, size=%" PRId32 ", fd=%" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    TRUE_RETURN_V_MSG_E((frameNum <= 0), ERR_DH_AVT_INVALID_PARAM, "invalid input frame number");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %" PRId32, size);

    unsigned int prot = PROT_READ | PROT_WRITE;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    TRUE_RETURN_V_MSG_E(addr == MAP_FAILED, ERR_DH_AVT_SHARED_MEMORY_FAILED, "shared memory mmap failed");

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    U32ToU8(base, frameNum);
    U64ToU8(base + sizeof(uint32_t), timestamp);

    DHLOGI("write frameNum=%" PRId32 ", timestamp=%lld to shared memory success.", frameNum, (long long)timestamp);
    return DH_AVT_SUCCESS;
}

int32_t ReadFrameInfoFromMemory(const AVTransSharedMemory &memory, uint32_t &frameNum, int64_t &timestamp)
{
    DHLOGI("read frame number and pts from shared memory, name=%s, size=%" PRId32 ", fd=%" PRId32,
        memory.name.c_str(), memory.size, memory.fd);
    TRUE_RETURN_V_MSG_E(IsInValidSharedMemory(memory), ERR_DH_AVT_INVALID_PARAM, "invalid input shared memory");

    int size = AshmemGetSize(memory.fd);
    TRUE_RETURN_V_MSG_E(size != memory.size, ERR_DH_AVT_SHARED_MEMORY_FAILED, "invalid memory size = %" PRId32, size);

    unsigned int prot = PROT_READ;
    int result = AshmemSetProt(memory.fd, static_cast<int>(prot));
    TRUE_RETURN_V_MSG_E(result < 0, ERR_DH_AVT_SHARED_MEMORY_FAILED, "AshmemSetProt failed");

    void *addr = ::mmap(nullptr, static_cast<size_t>(memory.size), static_cast<int>(prot), MAP_SHARED, memory.fd, 0);
    TRUE_RETURN_V_MSG_E(addr == MAP_FAILED, ERR_DH_AVT_SHARED_MEMORY_FAILED, "shared memory mmap failed");

    uint8_t *base = reinterpret_cast<uint8_t*>(addr);
    frameNum = U8ToU32(base);
    timestamp = U8ToU64(base + sizeof(uint32_t));
    TRUE_RETURN_V_MSG_E(frameNum <= 0, ERR_DH_AVT_MASTER_NOT_READY, "master queue not ready, frameNum is null.");

    DHLOGI("read frameNum=%" PRId32 ", timestamp=%lld from shared memory success.", frameNum, (long long)timestamp);
    return DH_AVT_SUCCESS;
}

bool IsInValidSharedMemory(const AVTransSharedMemory &memory)
{
    return (memory.fd <= 0) || (memory.size <= 0) || memory.name.empty();
}

bool IsInValidClockUnit(const AVSyncClockUnit &clockUnit)
{
    return (clockUnit.index < 0) || (clockUnit.index >= MAX_CLOCK_UNIT_COUNT) || (clockUnit.frameNum <= 0)
        || (clockUnit.timestamp <= 0);
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