# Review Result: merge-01b5c0c0fefa

- Status: completed-with-gaps
- Classification: Introduces defect
- Frozen HEAD: `5061f60a70ddf0a5e60feae364c792b228b4bd94`
- Attempt: 1

## Summary

Introduces firstCallingTokenId field to DHDescriptor and propagates it through IPC (proxy/stub ReadDescriptors/WriteDescriptors), TaskParam, and EnableTask. Two behavioral concerns: (1) The IPC serialization format is changed without backward compatibility - adding ReadUint32/WriteUint32 for firstCallingTokenId breaks old proxy-to-new-stub and new-proxy-to-old-stub communication due to parcel data misalignment. (2) The tokenId append condition changed from AUDIO-only to all DH types when firstCallingTokenId!=0, which expands tokenId injection to non-AUDIO components. The IPC format change is a concrete behavioral-compatibility defect.

## Review Protocol

- Review depth: full
- Protocol version: 2
- Primary dimensions: Functional correctness

## Context Inspected

- Level: direct-flow
- Self-contained: yes
- Rationale: Change is self-contained within modified files; no cross-boundary flow observed
- Changed symbols: DHDescriptor::firstCallingTokenId; DistributedHardwareProxy::ReadDescriptors; DistributedHardwareProxy::WriteDescriptors; DistributedHardwareStub::ReadDescriptors; DistributedHardwareStub::WriteDescriptors; TaskParam::firstCallingTokenId; EnableTask::SetFirstCallingTokenId; EnableTask::GetFirstCallingTokenId; EnableTask::AppendTokenIdToParams; EnableTask::DoActiveEnable; TaskFactory::CreateEnableTask; DistributedHardwareService::EnableSource

| Evidence | Disposition | Path | Symbol | Relation | Source ref |
| --- | --- | --- | --- | --- | --- |
| E1 | changed | common/utils/include/dhardware_descriptor.h | DHDescriptor::firstCallingTokenId | added-field | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E2 | changed | interfaces/inner_kits/src/ipc/distributed_hardware_proxy.cpp | DistributedHardwareProxy::ReadDescriptors | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E3 | changed | interfaces/inner_kits/src/ipc/distributed_hardware_proxy.cpp | DistributedHardwareProxy::WriteDescriptors | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E4 | changed | services/distributedhardwarefwkservice/src/distributed_hardware_stub.cpp | DistributedHardwareStub::ReadDescriptors | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E5 | changed | services/distributedhardwarefwkservice/src/distributed_hardware_stub.cpp | DistributedHardwareStub::WriteDescriptors | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E6 | changed | services/distributedhardwarefwkservice/src/distributed_hardware_service.cpp | DistributedHardwareService::EnableSource | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E7 | changed | services/distributedhardwarefwkservice/src/task/enable_task.cpp | EnableTask::DoActiveEnable | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E8 | changed | services/distributedhardwarefwkservice/src/task/enable_task.cpp | EnableTask::AppendTokenIdToParams | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E9 | changed | services/distributedhardwarefwkservice/src/task/task_factory.cpp | TaskFactory::CreateEnableTask | modified | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E10 | changed | services/distributedhardwarefwkservice/include/task/enable_task.h | EnableTask::firstCallingTokenId_ | added-field | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |
| E11 | changed | services/distributedhardwarefwkservice/include/utils/impl_utils.h | TaskParam::firstCallingTokenId | added-field | 772d1d30c5edb27b4521dc8b91655e06ef668ca5 |

### Verified flow edges

- DHDescriptor::firstCallingTokenId → DistributedHardwareProxy::ReadDescriptors (deserialized-from-parcel; evidence: E1, E2)
- DistributedHardwareProxy::ReadDescriptors → DistributedHardwareService::EnableSource (ipc-receive; evidence: E2, E6)
- DistributedHardwareService::EnableSource → TaskParam::firstCallingTokenId (copies-from-descriptor; evidence: E6, E11)
- TaskParam::firstCallingTokenId → EnableTask::firstCallingTokenId_ (set-via-TaskFactory; evidence: E9, E10)
- EnableTask::firstCallingTokenId_ → EnableTask::AppendTokenIdToParams (used-as-tokenId; evidence: E7, E8)
- DistributedHardwareProxy::WriteDescriptors → DistributedHardwareStub::ReadDescriptors (ipc-wire-format; evidence: E3, E4)

### Unresolved context

- None

## Quality Dimensions

| Dimension | Applicable | Level | Rationale | Evidence |
| --- | --- | --- | --- | --- |
| Functional correctness | yes | deep | Core change modifies enable flow to use firstCallingTokenId from DHDescriptor instead of callingTokenId from IPCSkeleton, and expands tokenId injection to all DH types | AppendTokenIdToParams now uses firstCallingTokenId parameter instead of GetCallingTokenId()<br>DoActiveEnable condition changed from DHType::AUDIO check to firstCallingTokenId!=0<br>firstCallingTokenId flows from DHDescriptor through TaskParam to EnableTask |
| API and compatibility | yes | screen | Screened for defect risk; no routed signal | - |
| State, lifecycle, concurrency | no | not-applicable | No concurrency changes | - |
| Resource and memory | yes | screen | Screened for defect risk; no routed signal | - |
| Reliability and recovery | yes | screen | Screened for defect risk; no routed signal | - |
| Performance and scalability | no | not-applicable | Adding one uint32 to IPC has negligible performance impact | - |
| Build and portability | yes | screen | Screened for defect risk; no routed signal | - |
| Test adequacy and observability | no | not-applicable | Test adequacy is outside current finding scope | - |
| Maintainability | yes | screen | Screened for defect risk; no routed signal | - |
| Security and privacy | yes | screen | Token ID propagation affects access control; using firstCallingTokenId (original caller) instead of callingTokenId (immediate IPC caller) changes which identity is forwarded to components | AppendTokenIdToParams now injects firstCallingTokenId into customParams.tokenId |

## Changed-Path Coverage

| Path | Disposition | Rationale | Evidence |
| --- | --- | --- | --- |
| common/utils/include/dhardware_descriptor.h | direct | DHDescriptor struct gains firstCallingTokenId field; struct layout and default value reviewed | uint32_t firstCallingTokenId{0} member added |
| interfaces/inner_kits/src/ipc/distributed_hardware_proxy.cpp | direct | ReadDescriptors/WriteDescriptors add uint32 field; serialization format change reviewed for compatibility | ReadUint32/WriteUint32 added for firstCallingTokenId |
| interfaces/inner_kits/test/unittest/common/ipc/distributed_hardware_proxy/src/distributed_hardware_proxy_test.cpp | direct | Test updated to write uint32(0) for firstCallingTokenId in manual parcel construction | data.WriteUint32(0u) added |
| services/distributedhardwarefwkservice/include/task/enable_task.h | direct | EnableTask gains firstCallingTokenId accessors and member; AppendTokenIdToParams signature changed | SetFirstCallingTokenId/GetFirstCallingTokenId added<br>AppendTokenIdToParams now takes uint32_t firstCallingTokenId parameter |
| services/distributedhardwarefwkservice/include/utils/impl_utils.h | direct | TaskParam gains firstCallingTokenId field | uint32_t firstCallingTokenId{0} member added |
| services/distributedhardwarefwkservice/src/distributed_hardware_service.cpp | direct | EnableSource populates firstCallingTokenId from DHDescriptor into TaskParam | .firstCallingTokenId = descriptor.firstCallingTokenId |
| services/distributedhardwarefwkservice/src/distributed_hardware_stub.cpp | direct | Stub-side ReadDescriptors/WriteDescriptors add uint32 field; same serialization format change as proxy | ReadUint32/WriteUint32 added for firstCallingTokenId |
| services/distributedhardwarefwkservice/src/task/enable_task.cpp | direct | AppendTokenIdToParams now uses firstCallingTokenId parameter; DoActiveEnable condition changed from AUDIO-only to firstCallingTokenId!=0 | Condition: if (GetFirstCallingTokenId() != 0)<br>cJSON_AddNumberToObject(json, "tokenId", static_cast<double>(firstCallingTokenId)) |
| services/distributedhardwarefwkservice/src/task/task_factory.cpp | direct | CreateEnableTask propagates firstCallingTokenId from TaskParam to EnableTask | enableTask->SetFirstCallingTokenId(taskParam.firstCallingTokenId) |
| services/distributedhardwarefwkservice/test/unittest/common/distributedhardwarestub/src/distributed_hardware_stub_test.cpp | direct | Test updated to include firstCallingTokenId in DHDescriptor construction | .firstCallingTokenId = 12345u |
| services/distributedhardwarefwkservice/test/unittest/common/task/src/enable_task_test.cpp | direct | Tests updated from SetCallingTokenId to SetFirstCallingTokenId; AppendTokenIdToParams calls updated with explicit parameter | SetFirstCallingTokenId(12345u)<br>AppendTokenIdToParams(params, 12345u) |

## Flow Coverage

- F1 [Functional correctness]: DistributedHardwareService::EnableSource → Enable source with tokenId: DHDescriptor.firstCallingTokenId flows through TaskParam to EnableTask and is injected into customParams via AppendTokenIdToParams for all DH types (not just AUDIO) (enable_task.cpp:260 if (GetFirstCallingTokenId() != 0) AppendTokenIdToParams(...))
- F2 [Functional correctness]: DistributedHardwareProxy::WriteDescriptors → IPC serialization: DHDescriptor is written to MessageParcel with 4 fields per descriptor (id, dhType, customParams, firstCallingTokenId); old version writes 3 fields, causing parcel misalignment across versions (proxy.cpp: WriteString(id), WriteInt32(dhType), WriteString(customParams), WriteUint32(firstCallingTokenId))

## Failure Hypotheses

### H1: Functional correctness

- Statement: IPC parcel misalignment: old proxy writes 3 fields per descriptor (id, dhType, customParams), new stub reads 4 fields; the stub's ReadUint32 for firstCallingTokenId reads from the next descriptor's data or trailing parcel data
- Trigger: An old-version process uses DistributedHardwareProxy::WriteDescriptors (3 fields) to call a new-version service running DistributedHardwareStub::ReadDescriptors (4 fields)
- Expected failure: Parcel data corruption: firstCallingTokenId reads garbage, subsequent descriptor fields are misaligned, possible crash or wrong enable behavior
- Outcome: supported
- Evidence: proxy.cpp WriteDescriptors: WriteString(id) + WriteInt32(dhType) + WriteString(customParams) + WriteUint32(firstCallingTokenId); stub.cpp ReadDescriptors: ReadString(id) + ReadInt32(dhType) + ReadString(customParams) + ReadUint32(firstCallingTokenId); No version field or length prefix to allow backward-compatible parsing
- Verification: Static analysis of ReadDescriptors/WriteDescriptors serialization format; confirmed no version negotiation mechanism exists

### H2: Functional correctness

- Statement: tokenId injection expanded from AUDIO-only to all DH types: non-AUDIO components may not expect or correctly handle a tokenId field in their customParams JSON
- Trigger: A non-AUDIO component (e.g., CAMERA, SCREEN) is enabled with firstCallingTokenId != 0
- Expected failure: Non-AUDIO component receives unexpected tokenId field in customParams, potentially causing parsing errors or incorrect behavior
- Outcome: inconclusive
- Evidence: Old condition: GetDhType() == DHType::AUDIO && GetCallingTokenId() != 0; New condition: GetFirstCallingTokenId() != 0; Default firstCallingTokenId is 0, so only explicitly-set values trigger the injection
- Verification: Cannot verify downstream component behavior statically; depends on whether callers set firstCallingTokenId for non-AUDIO types


## Findings

### F-01b5c0-001: Finding 1

- Type: code-defect
- Current at scan HEAD: yes
- Dimension: Functional correctness
- Severity / confidence: medium / medium
- Judgment: defect identified
- Violated invariant: Correctness invariant violated by changed code
- Trigger: Execution of the changed code path
- Impact: Cross-version IPC calls between processes with mismatched DHDescriptor serialization will corrupt parcel data, potentially causing enable operations to fail or crash
- Relevant flow: Changed code path to affected behavior
- Changed code: see diff evidence
- Evidence: proxy.cpp:919-951 WriteDescriptors writes id(dhType,customParams,firstCallingTokenId; stub.cpp:657-693 ReadDescriptors reads id,dhType,customParams,firstCallingTokenId; No parcel version/length field for DHDescriptor array
- Verification: Static code path review
- Temporal status: Current at scan HEAD; not yet fixed

## Checked Safe

- firstCallingTokenId default value 0 preserves no-injection behavior when not explicitly set
- cJSON_AddNumberToObject with static_cast<double>(uint32_t) preserves full uint32 range (53-bit mantissa)
- TaskFactory::CreateEnableTask correctly propagates firstCallingTokenId from TaskParam to EnableTask

## Exclusions

- None

## Coverage Gaps

- Cannot verify whether downstream non-AUDIO components handle tokenId in customParams correctly
- Cannot verify if old-version processes can call EnableSource on new-version services in production deployment scenarios

## Verification Disposition

- Status: static-analysis
- Rationale: IPC compatibility defect verified by code inspection; no runtime test executed
- Evidence: ReadDescriptors/WriteDescriptors format confirmed incompatible across versions

## Executed Checks

- `git diff + source read` — passed: Reviewed changed code paths against invariants

## Temporal Status

current
