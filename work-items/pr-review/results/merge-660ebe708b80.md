# Review Result: merge-660ebe708b80

- Status: completed-with-gaps
- Classification: Residual quality risk
- Frozen HEAD: `5061f60a70ddf0a5e60feae364c792b228b4bd94`
- Attempt: 1

## Summary

Adds customParams field to DHDescriptor, threads it through IPC (proxy/stub Read/WriteDescriptors), EnableTask, ComponentManager::Enable, ComponentEnable::Enable, and adds ConfigDistributedHardware call before RegisterDistributedHardware when customParams is non-empty. Also removes udid parameter from CheckDemandStart/GetRemoteVerInfo and adds cloud sync fallback (ActiveSyncMetaData/CloudSyncData) in online_task. Key risks: (1) IPC wire-format change adds customParams string to DHDescriptor serialization — breaks ABI compatibility with older clients; (2) ERR_DH_FWK_COMPONENT_CONFIG_FAILED (-10307) is within ComponentManager range but overlaps the common errno range pattern where -10007 is already used for UNREGISTER_FAILED; (3) ActiveSyncMetaData in online_task introduces a 13-second delayed task on the event handler that calls ActiveCloudSyncData if no meta data found, which may fire after the online task has already completed.

## Review Protocol

- Review depth: full
- Protocol version: 2
- Primary dimensions: Functional correctness

## Context Inspected

- Level: cross-boundary
- Self-contained: no
- Rationale: customParams flows from IPC entry (EnableSource) through TaskParam -> EnableTask -> ComponentManager::Enable -> ComponentEnable::Enable -> ConfigDistributedHardware. Also traced CheckDemandStart udid removal impact on GetRemoteVerInfo matching logic. Read online_task.cpp for ActiveSyncMetaData flow and db_adapter.h for CloudSyncData interface.
- Changed symbols: DHDescriptor::customParams; ComponentEnable::Enable (signature change); ComponentManager::Enable (signature change); ComponentManager::RealEnableSource (signature change); ComponentManager::CheckDemandStart (signature change); ComponentManager::GetRemoteVerInfo (signature change); EnableTask::SetCustomParams; EnableTask::GetCustomParams; TaskParam::customParams; DistributedHardwareProxy::WriteDescriptors; DistributedHardwareProxy::ReadDescriptors; DistributedHardwareStub::WriteDescriptors; DistributedHardwareStub::ReadDescriptors; GetDhDescriptorsCallbackStub::ReadDescriptors; OnLineTask::ActiveSyncMetaData; OnLineTask::IsNeedActiveSyncData; DBAdapter::CloudSyncData; DBAdapter::CloudSyncCallback; MetaInfoManager::ActiveCloudSyncData

| Evidence | Disposition | Path | Symbol | Relation | Source ref |
| --- | --- | --- | --- | --- | --- |
| ctx-1 | changed | common/utils/include/dhardware_descriptor.h | DHDescriptor | struct-definition | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |
| ctx-2 | changed | services/distributedhardwarefwkservice/include/componentmanager/component_enable.h | ComponentEnable::Enable | method-signature | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |
| ctx-3 | changed | services/distributedhardwarefwkservice/src/componentmanager/component_manager.cpp | ComponentManager::GetRemoteVerInfo | udid-matching-removed | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |
| ctx-4 | unchanged | services/distributedhardwarefwkservice/src/task/online_task.cpp | OnLineTask::ActiveSyncMetaData | new-fallback-sync | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |
| ctx-5 | changed | common/utils/include/distributed_hardware_errno.h | ERR_DH_FWK_COMPONENT_CONFIG_FAILED | new-error-code | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |
| ctx-6 | unchanged | services/distributedhardwarefwkservice/include/resourcemanager/db_adapter.h | DBAdapter::CloudSyncData | new-method | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |

### Verified flow edges

- DistributedHardwareService::EnableSource → TaskParam::customParams (populates-from-descriptor; evidence: ctx-1)
- TaskFactory::CreateEnableTask → EnableTask::SetCustomParams (sets-from-TaskParam; evidence: ctx-2)
- EnableTask::DoAutoEnable/DoActiveEnable → DHDescriptor::customParams (populates-from-customParams_; evidence: ctx-2)
- ComponentManager::EnableSourceInternal → ComponentManager::Enable (passes-customParams; evidence: ctx-1)
- ComponentEnable::Enable → ConfigDistributedHardware (calls-when-non-empty; evidence: ctx-2)
- OnLineTask::DoSyncInfo → ActiveSyncMetaData (calls-on-SyncMetaInfoFromDB-failure; evidence: ctx-4)

### Unresolved context

- Whether old-version IPC clients exist that would be broken by the DHDescriptor wire-format change

## Quality Dimensions

| Dimension | Applicable | Level | Rationale | Evidence |
| --- | --- | --- | --- | --- |
| Functional correctness | yes | deep | Multiple behavioral changes: new customParams IPC field, ConfigDistributedHardware before Register, removed udid from CheckDemandStart, new ActiveSyncMetaData flow. Must verify data flow integrity, ABI impact, and functional correctness of each change. | DHDescriptor gains customParams string field, serialized/deserialized in 3 locations: DistributedHardwareProxy, DistributedHardwareStub, GetDhDescriptorsCallbackStub — all consistently add WriteString/ReadString for customParams<br>ComponentEnable::Enable now calls handler->ConfigDistributedHardware(networkId, dhId, 'enable_init_params', customParams) when customParams is non-empty, returning ERR_DH_FWK_COMPONENT_CONFIG_FAILED on failure before attempting RegisterDistributedHardware<br>CheckDemandStart and GetRemoteVerInfo remove udid parameter and udidHash-based matching; now matches only by UUID via DHContext::GetInstance().GetUUIDByDeviceId()<br>OnLineTask::DoSyncInfo calls ActiveSyncMetaData() when SyncMetaInfoFromDB fails; ActiveSyncMetaData checks for MODEM source availability before triggering cloud sync<br>ActiveCloudSyncData calls dbAdapterPtr_->CloudSyncData() which calls kvStoragePtr_->CloudSync(callback) |
| API and compatibility | yes | screen | Screened for defect risk; no routed signal | - |
| State, lifecycle, concurrency | yes | screen | Screened for defect risk; no routed signal | - |
| Resource and memory | yes | screen | Screened for defect risk; no routed signal | - |
| Reliability and recovery | yes | screen | Screened for defect risk; no routed signal | - |
| Performance and scalability | yes | screen | Screened for defect risk; no routed signal | - |
| Build and portability | yes | screen | module_out_path changes in multiple av_transport test BUILD.gn files; distributedhardwarefwk.gni path variables changed. | distributedhardwarefwk.gni: fuzz_test_output_path and unittest_output_path shortened (removed dh_fwk_fuzz/dh_fwk_unittest suffix)<br>Multiple av_transport test BUILD.gn files override module_out_path locally instead of using the gni variable<br>av_transport/distributed_av_transport.gni: unittest_output_path removed, fuzz_test_output_path shortened |
| Test adequacy and observability | no | not-applicable | Test adequacy is outside current finding scope | - |
| Maintainability | yes | screen | Screened for defect risk; no routed signal | - |
| Security and privacy | yes | screen | customParams flows from IPC caller through to ConfigDistributedHardware — must verify no injection risk. | customParams is a string from the IPC caller that is passed directly to ConfigDistributedHardware with key 'enable_init_params'<br>No validation or sanitization of customParams content before passing to the hardware source handler<br>However, the caller is already privileged (passes IPC permission checks), and the source handler is a loaded plugin expected to validate its own inputs |

## Changed-Path Coverage

| Path | Disposition | Rationale | Evidence |
| --- | --- | --- | --- |
| common/utils/include/dhardware_descriptor.h | direct | Adds customParams string field to DHDescriptor struct | customParams added after dhType in struct definition |
| common/utils/include/distributed_hardware_errno.h | direct | Adds ERR_DH_FWK_COMPONENT_CONFIG_FAILED = -10307 | New error code in ComponentManager range [-10302, -10399] |
| interfaces/inner_kits/src/ipc/distributed_hardware_proxy.cpp | direct | Adds ReadString/WriteString for customParams in ReadDescriptors/WriteDescriptors | ReadDescriptors adds descriptor.customParams = data.ReadString()<br>WriteDescriptors adds data.WriteString(descriptor.customParams) with error check |
| interfaces/inner_kits/src/ipc/get_dh_descriptors_callback_stub.cpp | direct | Adds ReadString for customParams in ReadDescriptors | descriptor.customParams = data.ReadString() added |
| interfaces/inner_kits/test/unittest/common/ipc/distributed_hardware_proxy/src/distributed_hardware_proxy_test.cpp | direct | Adds WriteDescriptors_002 test verifying customParams round-trip | Test writes customParams JSON and reads back via ReadDescriptors |
| services/distributedhardwarefwkservice/include/componentmanager/component_enable.h | direct | Enable signature adds customParams parameter with default empty string | int32_t Enable(... IDistributedHardwareSource *handler, const std::string &customParams = "") |
| services/distributedhardwarefwkservice/include/componentmanager/component_manager.h | direct | Enable and RealEnableSource add customParams; CheckDemandStart removes udid | Enable adds customParams param<br>RealEnableSource adds customParams param<br>CheckDemandStart removes udid param |
| services/distributedhardwarefwkservice/include/task/enable_task.h | direct | Adds SetCustomParams/GetCustomParams and customParams_ member | Two new methods and one new member |
| services/distributedhardwarefwkservice/include/utils/impl_utils.h | direct | Adds customParams field to TaskParam struct | customParams string added to TaskParam |
| services/distributedhardwarefwkservice/src/componentmanager/component_enable.cpp | direct | Enable calls ConfigDistributedHardware when customParams non-empty, returns ERR_DH_FWK_COMPONENT_CONFIG_FAILED on failure | 14 lines added: check customParams.empty(), call ConfigDistributedHardware, return on failure |
| services/distributedhardwarefwkservice/src/componentmanager/component_manager.cpp | direct | Enable/RealEnableSource pass customParams; CheckDemandStart/GetRemoteVerInfo remove udid; GetRemoteVerInfo removes udidHash matching | Enable passes customParams to ComponentEnable::Enable<br>CheckDemandStart drops udid param<br>GetRemoteVerInfo drops udid param and udidHash matching, now matches only by UUID |
| services/distributedhardwarefwkservice/src/distributed_hardware_service.cpp | direct | EnableSource populates TaskParam.customParams from descriptor.customParams | .customParams = descriptor.customParams added to TaskParam initialization |
| services/distributedhardwarefwkservice/src/distributed_hardware_stub.cpp | direct | Read/WriteDescriptors add customParams serialization | ReadDescriptors adds descriptor.customParams = data.ReadString()<br>WriteDescriptors adds data.WriteString(descriptor.customParams) with error check |
| services/distributedhardwarefwkservice/src/task/enable_task.cpp | direct | Adds SetCustomParams/GetCustomParams implementations; DoAutoEnable/DoActiveEnable populate DHDescriptor.customParams from customParams_ | 8 lines for getter/setter<br>DHDescriptor initialization adds .customParams = customParams_ |
| services/distributedhardwarefwkservice/src/task/task_factory.cpp | direct | CreateEnableTask calls enableTask->SetCustomParams(taskParam.customParams) | 1 line added |
| services/distributedhardwarefwkservice/test/unittest/common/componentmanager/component_enable/component_enable_test.cpp | direct | Adds FakeDistributedHardwareSource and 3 new Enable tests covering ConfigDistributedHardware behavior | FakeDistributedHardwareSource class with configurable configRet_<br>Enable_003: config fails with customParams returns ERR_DH_FWK_COMPONENT_CONFIG_FAILED<br>Enable_004: config succeeds returns not CONFIG_FAILED<br>Enable_005: empty customParams skips config |
| services/distributedhardwarefwkservice/test/unittest/common/componentmanager/component_manager/src/mock_component_enable.cpp | direct | Mock Enable signature updated to include customParams | Signature change: adds const std::string &customParams |
| services/distributedhardwarefwkservice/test/unittest/common/distributedhardwarestub/src/distributed_hardware_stub_test.cpp | direct | Adds WriteDescriptors_001 test with customParams | Test creates descriptor with customParams JSON and writes it |

## Flow Coverage

- F1 [Functional correctness]: DistributedHardwareService::EnableSource → IPC caller provides descriptors with customParams -> TaskParam populated -> TaskFactory creates EnableTask with customParams -> DoAutoEnable/DoActiveEnable creates DHDescriptor with customParams -> ComponentManager::EnableSourceInternal passes customParams to Enable -> ComponentEnable::Enable calls ConfigDistributedHardware before Register when non-empty (distributed_hardware_service.cpp:604; task_factory.cpp:85; enable_task.cpp:191-195; component_manager.cpp:1553; component_enable.cpp:54-68)
- F2 [Functional correctness]: DistributedHardwareProxy::WriteDescriptors → DHDescriptor with customParams serialized to MessageParcel -> transmitted via IPC -> stub ReadDescriptors deserializes including customParams (distributed_hardware_proxy.cpp:946-949; distributed_hardware_stub.cpp:656-657)
- F3 [Functional correctness]: OnLineTask::DoSyncInfo → SyncMetaInfoFromDB fails -> ActiveSyncMetaData checks MODEM source -> posts 13s delayed task -> if no meta data found, triggers ActiveCloudSyncData -> CloudSyncData calls kvStoragePtr_->CloudSync (online_task.cpp:106-109; online_task.cpp:218-242; meta_info_manager.cpp:533-543; db_adapter.cpp:603-641)
- F4 [Functional correctness]: ComponentManager::CheckDemandStart → CheckDemandStart no longer receives udid; GetRemoteVerInfo matches meta info only by UUID (removed udidHash fallback) (component_manager.cpp:1088-1091; component_manager.cpp:1348-1363)

## Failure Hypotheses

### H1: Functional correctness

- Statement: IPC wire-format change for DHDescriptor (adding customParams string) breaks compatibility with older clients that expect only 2 fields (dhType + id) per descriptor
- Trigger: Old-version client reads descriptors from new-version stub, or new-version proxy writes to old-version stub
- Expected failure: Deserialization misalignment: old client reads customParams as next descriptor's dhType, causing garbage data or crash
- Outcome: supported
- Evidence: WriteDescriptors now writes 3 strings per descriptor where it previously wrote 2; ReadDescriptors reads 3 per descriptor; No version negotiation or capability check in the IPC protocol; This is a cross-version ABI break per AGENTS.md constraint: '改动跨设备协议字段、SendOnLineEvent/Enable 顺序、序列化格式'
- Verification: Test IPC communication between old and new version binaries

### H2: Functional correctness

- Statement: GetRemoteVerInfo removal of udidHash matching may fail to find meta info for devices where UUID lookup via DHContext fails but udidHash would have matched
- Trigger: Device meta info has deviceId that maps to udidHash but UUID is not yet registered in DHContext
- Expected failure: ERR_DH_FWK_COMPONENT_COMPVERSION_NOT_FOUND returned when meta info exists but UUID path fails
- Outcome: inconclusive
- Evidence: Previously matched by either UUID or udidHash; now only UUID; DHContext::GetInstance().GetUUIDByDeviceId() depends on device being online and registered in context; If device goes offline between meta info storage and version lookup, UUID may not be in DHContext
- Verification: Test CheckDemandStart with device that has meta info by udidHash but not by UUID in DHContext

### H3: Functional correctness

- Statement: ActiveSyncMetaData posts a 13-second delayed task that checks for meta data and triggers cloud sync if empty; this may fire after the online task has completed and the device context has changed
- Trigger: Device online -> DoSyncInfo -> SyncMetaInfoFromDB fails -> ActiveSyncMetaData posts 13s task -> online task completes -> 13s later task fires and calls ActiveCloudSyncData on stale context
- Expected failure: Cloud sync triggered unnecessarily or on stale udidHash; minor resource waste, no data corruption
- Outcome: ruled-out
- Evidence: The lambda captures udidHash by value, so it uses the correct hash regardless of timing; ActiveCloudSyncData is a self-contained operation that checks for empty meta data before proceeding; 13s delay is designed to wait for passive sync to complete; if passive sync succeeds, the check finds data and skips cloud sync
- Verification: Trace ActiveSyncMetaData timing relative to online task completion


## Findings

### F-660e-1: Finding 1

- Type: code-defect
- Current at scan HEAD: yes
- Dimension: Functional correctness
- Severity / confidence: high / medium
- Judgment: defect identified
- Violated invariant: Correctness invariant violated by changed code
- Trigger: Execution of the changed code path
- Impact: Functional degradation or incorrect behavior
- Relevant flow: Changed code path to affected behavior
- Changed code: see diff evidence
- Evidence: WriteDescriptors writes 3 fields per descriptor (dhType, id, customParams) where previously only 2 were written; No version negotiation or backward-compatible padding
- Verification: Static code path review
- Temporal status: Current at scan HEAD; not yet fixed

### F-660e-2: Finding 2

- Type: code-defect
- Current at scan HEAD: yes
- Dimension: Functional correctness
- Severity / confidence: medium / medium
- Judgment: defect identified
- Violated invariant: Correctness invariant violated by changed code
- Trigger: Execution of the changed code path
- Impact: Functional degradation or incorrect behavior
- Relevant flow: Changed code path to affected behavior
- Changed code: see diff evidence
- Evidence: Original code: if (GetUUIDByDeviceId == uuid || udidHash == udidHash) — dual path; New code: if (GetUUIDByDeviceId == uuid) — single path
- Verification: Static code path review
- Temporal status: Current at scan HEAD; not yet fixed

## Checked Safe

- customParams data flow is consistent from IPC entry through to ConfigDistributedHardware call
- ConfigDistributedHardware is correctly called only when customParams is non-empty
- ERR_DH_FWK_COMPONENT_CONFIG_FAILED (-10307) is within the ComponentManager errno range [-10302, -10399]
- EnableTask::DoAutoEnable and DoActiveEnable both correctly populate DHDescriptor.customParams from customParams_
- TaskFactory correctly propagates customParams from TaskParam to EnableTask
- Test coverage for ConfigDistributedHardware success/failure/empty-customParams paths is adequate
- DBAdapter::CloudSyncData correctly checks kvStoragePtr_ null under lock and handles CLOUD_DISABLED status
- ActiveSyncMetaData lambda captures udidHash by value, avoiding use-after-free

## Exclusions

- None

## Coverage Gaps

- Inconclusive hypothesis requires runtime verification

## Verification Disposition

- Status: static-analysis
- Rationale: No build environment or device available; analysis based on code flow tracing, IPC wire-format analysis, and error code range verification.
- Evidence: IPC serialization format compared across 3 Read/WriteDescriptors implementations; Error code range verified against distributed_hardware_errno.h; GetRemoteVerInfo matching logic traced through DHContext and MetaInfoManager

## Executed Checks

- `IPC wire-format compatibility analysis across 3 Read/WriteDescriptors implementations` — passed: reviewed
- `customParams data flow integrity from IPC entry to ConfigDistributedHardware` — passed: reviewed
- `Error code range compliance for ERR_DH_FWK_COMPONENT_CONFIG_FAILED` — passed: reviewed
- `GetRemoteVerInfo matching logic regression analysis (udidHash removal)` — passed: reviewed
- `ActiveSyncMetaData timing and capture semantics analysis` — passed: reviewed
- `Test coverage adequacy for new ConfigDistributedHardware path` — passed: reviewed

## Temporal Status

current
