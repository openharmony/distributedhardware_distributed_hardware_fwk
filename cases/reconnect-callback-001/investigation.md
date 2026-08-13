# Investigation Report: reconnect-callback-001

## Case Summary

**Case ID**: reconnect-callback-001  
**Result Status**: Probable  
**Date**: 2026-08-13  

**Original Symptom**: 设备上线之后主动使能分布式通信失败了，日志能看到设备上线没有走到使能流程。

After a device comes online, the active enable of distributed communication fails. Logs show the device online event is received but does not reach the enable flow.

---

## Investigation Summary

The device online event is received by `AccessManager::OnDeviceOnline` but fails to reach the enable flow. The most likely root cause is a **race condition in `DistributedHardwareManagerFactory`** where `flagUnInit_` is set to `true` during offline processing and is not properly reset before the next online event arrives. The `flagUnInit_` guard at `SendOnLineEvent:185` blocks the online event, returning `ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED` without creating an `OnLineTask`.

---

## Code Path Traced

```
AccessManager::OnDeviceOnline (access_manager.cpp:121)
  -> DistributedHardwareManagerFactory::SendOnLineEvent (distributed_hardware_manager_factory.cpp:174)
    -> GUARD: flagUnInit_.load() == true? → RETURN ERR (line 185-188)  *** BLOCKED HERE ***
    -> GUARD: IsDeviceOnline(uuid) == true? → RETURN ERR (line 190-192)
    -> DHContext::AddOnlineDevice (line 195)
    -> GUARD: !isInit_ && !Init() → RETURN ERR (line 200-203)
    -> DistributedHardwareManager::SendOnLineEvent (line 218)
      -> TaskFactory::CreateTask(TaskType::ON_LINE) (distributed_hardware_manager.cpp:121)
        -> OnLineTask::DoTask (online_task.cpp:56)
          -> DoSyncInfo (line 65)
          -> CreateEnableTask (line 69)
            -> CapabilityInfoManager / LocalCapabilityInfoManager / MetaInfoManager lookup
            -> TaskFactory::CreateTask(TaskType::ENABLE) for each capability
              -> EnableTask::DoTask -> DoAutoEnable / DoActiveEnable
                -> ComponentManager::EnableSink / EnableSource
```

---

## Findings

### F1 (Probable - Root Cause): flagUnInit_ race condition blocks device re-online enable flow

**Dimension**: State/lifecycle/concurrency | **Severity**: critical | **Confidence**: high

**Causal Chain**:
1. Last device goes offline → `SendOffLineEvent` sets `flagUnInit_=true` (line 246) when `GetRealTimeOnlineDeviceCount()==0`
2. Device comes back online → `AccessManager::OnDeviceOnline` → `SendOnLineEvent`
3. `SendOnLineEvent` checks `flagUnInit_.load()` (line 185) which is still **true**
4. Returns `ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED` without creating `OnLineTask`
5. **Enable flow never executes**

**Key Evidence**:
- `distributed_hardware_manager_factory.cpp:246`: `flagUnInit_.store(true)` in `SendOffLineEvent`
- `distributed_hardware_manager_factory.cpp:185-188`: `if (flagUnInit_.load()) { return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED; }`
- `flagUnInit_` is only reset to `false` in `UnInit()` at line 118, which may have already completed
- **No path resets `flagUnInit_` to `false` when a new online event arrives before `UnInit` completes**

**Code Reference**: `distributed_hardware_manager_factory.cpp:185-188`
```cpp
if (flagUnInit_.load()) {
    DHLOGE("is in uniniting, can not process online event.");
    return ERR_DH_FWK_HARDWARE_MANAGER_INIT_FAILED;
}
```

---

### F2 (Possible - Contributor): DHContext online device set not cleaned during offline

**Dimension**: State/lifecycle/concurrency | **Severity**: high | **Confidence**: medium

**Causal Chain**:
1. Device goes offline → `SendOffLineEvent` calls `DeleteRealTimeOnlineDeviceNetworkId` but **NOT** `RemoveOnlineDeviceIdEntryByNetworkId`
2. Device comes back online → `SendOnLineEvent` checks `IsDeviceOnline(uuid)` which searches `devIdEntrySet_`
3. Finds device still in set → returns `ERR_DH_FWK_HARDWARE_MANAGER_DEVICE_REPEAT_ONLINE`
4. **Online event silently dropped**

**Key Evidence**:
- `SendOffLineEvent` (line 243) only calls `DeleteRealTimeOnlineDeviceNetworkId`, not `RemoveOnlineDeviceIdEntryByNetworkId`
- `IsDeviceOnline` at line 190 checks `devIdEntrySet_` which is not cleaned in offline path

---

### F3 (Possible - Contributor): isInit_ re-initialization races with UnInit release

**Dimension**: State/lifecycle/concurrency | **Severity**: medium | **Confidence**: medium

- `SendOnLineEvent:200-203` checks `isInit_` and calls `Init()` if false
- `UnInit` sets `isInit_=false` at line 117 and holds `releaseProcessMutex_`
- Race between UnInit and new online event Init is not protected

---

### F4 (Possible - Manifestation): CreateEnableTask finds no capability data

**Dimension**: Functional correctness | **Severity**: medium | **Confidence**: low

- For double-frame devices, `SendOnLineEvent` clears meta/local capability data (line 215-217)
- If `DoSyncInfo` hasn't completed, `CreateEnableTask` finds empty capabilities and returns without creating enable tasks

---

### F5 (Possible - Contributor): CheckDemandStart fails due to missing remote version

**Dimension**: Functional correctness | **Severity**: medium | **Confidence**: low

- `EnableTask::DoAutoEnable` calls `CheckDemandStart` which requires `MetaInfoManager` data
- If meta info hasn't synced, `GetRemoteVerInfo` returns error, blocking auto-enable

---

## Excluded Candidates

| Candidate | Excluding Invariant | Evidence |
|-----------|-------------------|----------|
| AccessManager callback missing | Logs show device online IS received | OnDeviceOnline fires correctly |
| TaskExecutor queue full | MAX_TASK_QUEUE_LENGTH=256, unlikely full | Would produce distinct log |
| Component SO load failure | Happens after OnLineTask creation, not before | Symptom is earlier in chain |
| EventHandler null | Initialized in DHContext constructor | Catastrophic failure only |

---

## Recommended Verification Steps

1. **Add logging** at `SendOnLineEvent:185` to log the value of `flagUnInit_`, `IsDeviceOnline()`, and `isInit_` when the online event is received
2. **Check if `RemoveOnlineDeviceIdEntryByNetworkId`** should be called in `SendOffLineEvent` to clean up `devIdEntrySet_`
3. **Reset `flagUnInit_`** to false before the guard check in `SendOnLineEvent`, or at the start of the online processing, since the online event itself indicates the system should be active
4. **Write unit test** simulating offline→online sequence to verify `flagUnInit_` does not block re-online
5. **Verify timing**: check if `UnInit` has completed before the next online event arrives

---

## Ledger Candidates

| ID | Finding | Root Cause Key | Question to Confirm |
|----|---------|---------------|-------------------|
| LC1 | F1 | flagUnInit_not_reset_before_online_event | Is `flagUnInit_` still true when the re-online event arrives? |
| LC2 | F2 | devIdEntrySet_not_cleaned_on_offline | Is the device still in `devIdEntrySet_` after offline? |
| LC3 | F3 | isInit_reinit_race_with_uninit | Does `Init()` fail while `UnInit` is in progress? |
