# Review Result: merge-7be94f4daad0

- Status: completed-with-gaps
- Classification: Introduces defect
- Frozen HEAD: `5061f60a70ddf0a5e60feae364c792b228b4bd94`
- Attempt: 1

## Summary

Adds sink SA monitoring to ComponentManager, enabling recovery when sink SAs crash. Introduces GetSinkSaId and GetDHTypeBySaId (matching both source and sink saIds), compSinkSaId_ tracking map, and keepSAMonitor parameter to preserve SA subscription during recovery. Introduces a code defect: InitCompSink returns ERR_DH_FWK_COMPONENT_MONITOR_NULL after partially initializing the sink (loaded, listener registered, inserted into compSink_ and compSinkSaId_), without rollback. Similarly, UninitCompSink returns the same error after unregistering the sink listener but before releasing the handler or erasing from compSink_, leaving a partially uninitialized component. These paths are currently unreachable (compMonitorPtr_ is always initialized in the constructor) but represent latent error-path leaks that would cause inconsistent state if the invariant is violated.

## Review Protocol

- Review depth: full
- Protocol version: 2
- Primary dimensions: Functional correctness

## Context Inspected

- Level: direct-flow
- Self-contained: no
- Rationale: Sink SA monitoring introduces a new recovery trigger path (OnRemoveSystemAbility→Recover→DoRecover) and new error paths in InitCompSink/UninitCompSink; traced the full flow from SA monitor callback through recovery to re-initialization
- Changed symbols: ComponentLoader::GetSinkSaId; ComponentLoader::GetDHTypeBySaId; ComponentManager::InitCompSink; ComponentManager::UninitCompSink; ComponentManager::UninitCompSource; ComponentManager::ResetSinkEnableStatus; ComponentManager::ResetSourceEnableStatus; ComponentManager::DoRecover; ComponentMonitor::CompSystemAbilityListener::OnRemoveSystemAbility; ComponentManager::compSinkSaId_

| Evidence | Disposition | Path | Symbol | Relation | Source ref |
| --- | --- | --- | --- | --- | --- |
| E1 | changed | services/distributedhardwarefwkservice/include/componentloader/component_loader.h | ComponentLoader::GetSinkSaId | added-declaration | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E2 | changed | services/distributedhardwarefwkservice/include/componentloader/component_loader.h | ComponentLoader::GetDHTypeBySaId | renamed-from-GetDHTypeBySrcSaId | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E3 | changed | services/distributedhardwarefwkservice/src/componentloader/component_loader.cpp | ComponentLoader::GetSinkSaId | added-implementation | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E4 | changed | services/distributedhardwarefwkservice/src/componentloader/component_loader.cpp | ComponentLoader::GetDHTypeBySaId | modified-matches-both-saIds | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E5 | changed | services/distributedhardwarefwkservice/src/componentmanager/component_manager.cpp | ComponentManager::InitCompSink | modified-adds-sink-SA-monitor | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E6 | changed | services/distributedhardwarefwkservice/src/componentmanager/component_manager.cpp | ComponentManager::UninitCompSink | modified-adds-SA-monitor-cleanup | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E7 | changed | services/distributedhardwarefwkservice/src/componentmanager/component_manager.cpp | ComponentManager::UninitCompSource | modified-adds-keepSAMonitor | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E8 | changed | services/distributedhardwarefwkservice/src/componentmanager/component_manager.cpp | ComponentManager::DoRecover | modified-passes-keepSAMonitor-true | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E9 | changed | services/distributedhardwarefwkservice/src/componentmanager/component_monitor.cpp | OnRemoveSystemAbility | uses-GetDHTypeBySaId | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E10 | unchanged | services/distributedhardwarefwkservice/include/componentmanager/component_monitor.h | saIdProcessNameMap_ | contains-both-source-and-sink-SA-IDs | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |
| E11 | changed | services/distributedhardwarefwkservice/include/componentmanager/component_manager.h | compSinkSaId_ | added-member | 4ac767a3a8c8d9ebe7430aaeaa0979ca58bc14c6 |

### Verified flow edges

- OnRemoveSystemAbility → ComponentLoader::GetDHTypeBySaId (resolves-DHType-from-saId; evidence: E9, E4)
- OnRemoveSystemAbility → ComponentManager::Recover (triggers-recovery; evidence: E9)
- ComponentManager::DoRecover → ResetSinkEnableStatus(keepSAMonitor=true) (preserves-SA-monitor-during-recovery; evidence: E8)
- ResetSinkEnableStatus → UninitCompSink(keepSAMonitor=true) (uninitializes-sink-without-removing-monitor; evidence: E6)
- ComponentLoader::GetSinkSaId → InitCompSink (provides-sink-SA-ID-for-monitoring; evidence: E3, E5)

### Unresolved context

- None

## Quality Dimensions

| Dimension | Applicable | Level | Rationale | Evidence |
| --- | --- | --- | --- | --- |
| Functional correctness | yes | deep | Core change adds sink SA monitoring and recovery flow; error paths in InitCompSink/UninitCompSink leave partial state on compMonitorPtr_==null | InitCompSink returns ERR_DH_FWK_COMPONENT_MONITOR_NULL after compSink_.insert and compSinkSaId_.insert<br>UninitCompSink returns ERR_DH_FWK_COMPONENT_MONITOR_NULL after UnregisterDistributedHardwareSinkStateListener but before ReleaseSink and compSink_.erase |
| API and compatibility | yes | screen | Screened for defect risk; no routed signal | - |
| State, lifecycle, concurrency | yes | deep | Dual SA monitoring (source+sink) for same DHType creates concurrent recovery scenarios | Both sourceSaId and sinkSaId OnRemoveSystemAbility can trigger Recover for same DHType |
| Resource and memory | yes | screen | Screened for defect risk; no routed signal | - |
| Reliability and recovery | yes | screen | Screened for defect risk; no routed signal | - |
| Performance and scalability | no | not-applicable | Minimal performance impact from additional map lookups and SA subscriptions | - |
| Build and portability | yes | screen | Screened for defect risk; no routed signal | - |
| Test adequacy and observability | no | not-applicable | Test adequacy is outside current finding scope | - |
| Maintainability | yes | screen | Screened for defect risk; no routed signal | - |
| Security and privacy | no | not-applicable | No security-relevant changes | - |

## Changed-Path Coverage

| Path | Disposition | Rationale | Evidence |
| --- | --- | --- | --- |
| services/distributedhardwarefwkservice/include/componentloader/component_loader.h | direct | GetDHTypeBySrcSaId renamed to GetDHTypeBySaId; GetSinkSaId added | int32_t GetSinkSaId(const DHType dhType);<br>DHType GetDHTypeBySaId(const int32_t saId); |
| services/distributedhardwarefwkservice/include/componentmanager/component_manager.h | direct | UninitCompSource/UninitCompSink gain keepSAMonitor parameter; ResetSinkEnableStatus/ResetSourceEnableStatus gain keepSAMonitor; compSinkSaId_ map added | std::map<DHType, int32_t> compSinkSaId_;<br>int32_t UninitCompSource(DHType dhType, bool keepSAMonitor = false); |
| services/distributedhardwarefwkservice/src/componentloader/component_loader.cpp | direct | GetSinkSaId implemented; GetDHTypeBySaId matches both sourceSaId and sinkSaId | if (handler.second.sourceSaId == saId \|\| handler.second.sinkSaId == saId) |
| services/distributedhardwarefwkservice/src/componentmanager/component_manager.cpp | direct | InitCompSink adds sink SA monitoring; UninitCompSink adds SA monitor cleanup with keepSAMonitor; DoRecover passes keepSAMonitor=true | InitCompSink: compMonitorPtr_->AddSAMonitor(saId)<br>UninitCompSink: if (!keepSAMonitor) compMonitorPtr_->RemoveSAMonitor(it->second)<br>DoRecover: ResetSinkEnableStatus(dhType, true) |
| services/distributedhardwarefwkservice/src/componentmanager/component_monitor.cpp | direct | OnRemoveSystemAbility uses GetDHTypeBySaId instead of GetDHTypeBySrcSaId | DHType dhType = ComponentLoader::GetInstance().GetDHTypeBySaId(saId); |
| services/distributedhardwarefwkservice/test/unittest/common/componentloader/src/component_loader_test.cpp | direct | Tests added for GetSinkSaId and GetDHTypeBySaId (matching both source and sink saIds) | GetSinkSaId_001, GetDHTypeBySaId_001, GetDHTypeBySaId_002 |
| services/distributedhardwarefwkservice/test/unittest/common/componentmanager/component_manager/src/component_manager_test.cpp | direct | Tests added for InitCompSink with compMonitorPtr_ null/valid, UninitCompSink with keepSAMonitor, ResetSinkEnableStatus/ResetSourceEnableStatus with keepSAMonitor | InitCompSink_002, InitCompSink_003, UninitCompSink_003, UninitCompSink_004, UninitCompSource_002, UninitCompSource_003 |
| services/distributedhardwarefwkservice/test/unittest/common/componentmanager/component_monitior/src/component_monitor_test.cpp | direct | OnRemoveSystemAbility tests updated from GetDHTypeBySrcSaId to GetDHTypeBySaId | ComponentLoader::GetInstance().GetDHTypeBySaId(saId) |

## Flow Coverage

- F1 [Functional correctness]: ComponentMonitor::OnRemoveSystemAbility → Sink SA crash recovery: OnRemoveSystemAbility resolves DHType via GetDHTypeBySaId (matching sink saIds), triggers Recover→DoRecover which calls ResetSinkEnableStatus(keepSAMonitor=true) to uninitialize sink while preserving SA subscription, then RecoverAutoEnableSink re-initializes via InitCompSink (component_monitor.cpp:56 GetDHTypeBySaId resolves both source and sink SA IDs; component_manager.cpp:687 ResetSinkEnableStatus(dhType, true); InitCompSink AddSAMonitor skips if already subscribed (component_monitor.cpp:78-80))
- F2 [Functional correctness]: ComponentManager::InitCompSink → Error path: InitCompSink inserts sink into compSink_ and compSinkSaId_ then returns ERR_DH_FWK_COMPONENT_MONITOR_NULL if compMonitorPtr_ is null, without rolling back the insertions (component_manager.cpp:2149 compSink_.insert before null check; component_manager.cpp:2152 compSinkSaId_.insert before null check; component_manager.cpp:2154 return ERR_DH_FWK_COMPONENT_MONITOR_NULL without cleanup)

## Failure Hypotheses

### H1: Functional correctness

- Statement: InitCompSink returns ERR_DH_FWK_COMPONENT_MONITOR_NULL after partially initializing the sink: compSink_ and compSinkSaId_ are populated, sink handler is loaded and listener registered, but the function signals failure
- Trigger: compMonitorPtr_ is nullptr when InitCompSink is called and GetSinkSaId returns a valid SA ID
- Expected failure: Caller believes initialization failed but the sink is actually in compSink_ and compSinkSaId_; subsequent calls may find the sink already loaded (GetSink returns ERR_DH_FWK_LOADER_SOURCE_LOAD) causing double-initialization or inconsistent state
- Outcome: supported
- Evidence: component_manager.cpp:2149 compSink_.insert(std::make_pair(dhType, sinkPtr)); component_manager.cpp:2152 compSinkSaId_.insert(std::make_pair(dhType, saId)); component_manager.cpp:2154-2155 DHLOGE + return ERR_DH_FWK_COMPONENT_MONITOR_NULL without erasing compSink_ or compSinkSaId_ entries
- Verification: Static code path analysis; confirmed compMonitorPtr_ is always initialized in constructor but error path leaves inconsistent state

### H2: Functional correctness

- Statement: UninitCompSink returns ERR_DH_FWK_COMPONENT_MONITOR_NULL after UnregisterDistributedHardwareSinkStateListener but before ReleaseSink and compSink_.erase, leaving the sink handler loaded and tracked
- Trigger: compMonitorPtr_ is nullptr when UninitCompSink is called and compSinkSaId_ has an entry for the DHType
- Expected failure: Sink listener is unregistered but sink handler is not released; compSink_ still has the entry; subsequent re-initialization would see sink already loaded and fail
- Outcome: supported
- Evidence: component_manager.cpp:2177 sinkPtr->UnregisterDistributedHardwareSinkStateListener(); component_manager.cpp:2180-2182 return ERR_DH_FWK_COMPONENT_MONITOR_NULL without calling ReleaseSink or erasing compSink_
- Verification: Static code path analysis

### H3: Functional correctness

- Statement: Concurrent recovery: both source and sink SA going down for the same DHType triggers two concurrent DoRecover operations via ffrt::submit, which could race on ResetSinkEnableStatus/ResetSourceEnableStatus and recovery re-initialization
- Trigger: Source SA (e.g., 4805) and sink SA (e.g., 4806) for AUDIO both crash simultaneously or in quick succession
- Expected failure: Two DoRecover(AUDIO) tasks run concurrently; first uninitializes and starts recovery, second finds component already uninitialized and may cause double-recovery or inconsistent state
- Outcome: inconclusive
- Evidence: component_monitor.cpp:56 GetDHTypeBySaId matches both sourceSaId and sinkSaId for same DHType; component_monitor.cpp:71 ComponentManager::GetInstance().Recover(dhType) called from both callbacks; DoRecover uses locks for individual Reset* calls but not for the entire recovery sequence
- Verification: Cannot fully verify concurrency behavior statically; existing locking provides partial serialization but recovery sequence as a whole is not atomic


## Findings

### F-7be94f-001: Finding 1

- Type: code-defect
- Current at scan HEAD: yes
- Dimension: Functional correctness
- Severity / confidence: low / medium
- Judgment: defect identified
- Violated invariant: Correctness invariant violated by changed code
- Trigger: Execution of the changed code path
- Impact: If triggered, leaves ComponentManager in inconsistent state: sink is loaded and tracked but caller believes init failed; subsequent operations may malfunction
- Relevant flow: Changed code path to affected behavior
- Changed code: see diff evidence
- Evidence: component_manager.cpp:2149 compSink_.insert(std::make_pair(dhType, sinkPtr)); component_manager.cpp:2152 compSinkSaId_.insert(std::make_pair(dhType, saId)); component_manager.cpp:2154-2155 return ERR_DH_FWK_COMPONENT_MONITOR_NULL without rollback
- Verification: Static code path review
- Temporal status: Current at scan HEAD; not yet fixed

### F-7be94f-002: Finding 2

- Type: code-defect
- Current at scan HEAD: yes
- Dimension: Functional correctness
- Severity / confidence: low / medium
- Judgment: defect identified
- Violated invariant: Correctness invariant violated by changed code
- Trigger: Execution of the changed code path
- Impact: If triggered, sink handler is leaked (not released via ReleaseSink) and compSink_ entry persists; component is in partially uninitialized state with listener unregistered but handler still loaded
- Relevant flow: Changed code path to affected behavior
- Changed code: see diff evidence
- Evidence: component_manager.cpp:2177 sinkPtr->UnregisterDistributedHardwareSinkStateListener(); component_manager.cpp:2180-2182 return ERR_DH_FWK_COMPONENT_MONITOR_NULL before ReleaseSink/compSink_.erase
- Verification: Static code path review
- Temporal status: Current at scan HEAD; not yet fixed

## Checked Safe

- GetDHTypeBySaId correctly matches both sourceSaId and sinkSaId
- GetSinkSaId uses mutex and find() pattern consistent with GetSourceSaId
- keepSAMonitor=true in DoRecover correctly preserves SA subscription during recovery cycle
- AddSAMonitor correctly skips if SA ID is already monitored (idempotent)
- compSinkSaId_ initialized in constructor and cleared in tests
- keepSAMonitor=false default preserves existing UninitComp* behavior for non-recovery callers

## Exclusions

- None

## Coverage Gaps

- Inconclusive hypothesis requires runtime verification

## Verification Disposition

- Status: static-analysis
- Rationale: Error-path defects verified by code inspection; concurrent recovery concern identified but cannot be fully verified statically
- Evidence: Read InitCompSink, UninitCompSink, DoRecover, and OnRemoveSystemAbility at head SHA

## Executed Checks

- `git diff + source read` — passed: Reviewed changed code paths against invariants

## Temporal Status

current
