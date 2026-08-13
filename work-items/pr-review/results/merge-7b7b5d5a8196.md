# Review Result: merge-7b7b5d5a8196

- Status: completed
- Classification: Fixes defect
- Frozen HEAD: `5061f60a70ddf0a5e60feae364c792b228b4bd94`
- Attempt: 1

## Summary

ReadDeviceInfo parameter changed from pass-by-value (AuthDeviceInfo info) to pass-by-reference (AuthDeviceInfo &info), fixing a defect where the IPC stub deserialized device info into a stack copy that was immediately discarded. The caller in OnRemoteRequest reads into a local 'info' variable and passes it to ReadDeviceInfo, so with pass-by-value the writes to 'info' inside ReadDeviceInfo were lost. This was a functional correctness bug.

## Review Protocol

- Review depth: full
- Protocol version: 2
- Primary dimensions: Functional correctness

## Context Inspected

- Level: direct-flow
- Self-contained: no
- Rationale: Traced the data flow from OnRemoteRequest through ReadDeviceInfo to OnRequestHardwareAccess to verify the fix resolves a real data-loss bug. Also checked the IAccessListener interface and proxy to confirm no ABI impact.
- Changed symbols: AccessListenerStub::ReadDeviceInfo

| Evidence | Disposition | Path | Symbol | Relation | Source ref |
| --- | --- | --- | --- | --- | --- |
| E1 | changed | services/distributedhardwarefwkservice/include/ipc/access_listener_stub.h | AccessListenerStub::ReadDeviceInfo | declaration-site | d8e620f774f0a9c5731f6bbb528a39ee4273c996 |
| E2 | changed | services/distributedhardwarefwkservice/src/ipc/access_listener_stub.cpp | AccessListenerStub::ReadDeviceInfo | definition-site | d8e620f774f0a9c5731f6bbb528a39ee4273c996 |
| E3 | changed | services/distributedhardwarefwkservice/src/ipc/access_listener_stub.cpp | AccessListenerStub::OnRemoteRequest | caller-of-ReadDeviceInfo | d8e620f774f0a9c5731f6bbb528a39ee4273c996 |
| E4 | unchanged | common/utils/include/iaccess_listener.h | IAccessListener::OnRequestHardwareAccess | interface-contract | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |
| E5 | unchanged | interfaces/inner_kits/src/ipc/access_listener_proxy.cpp | AccessListenerProxy::OnRequestHardwareAccess | proxy-side-serialization | 5061f60a70ddf0a5e60feae364c792b228b4bd94 |

### Verified flow edges

- AccessListenerStub::OnRemoteRequest → AccessListenerStub::ReadDeviceInfo (calls; evidence: E3, E2)
- AccessListenerStub::ReadDeviceInfo → AccessListenerStub::OnRemoteRequest::info (writes-via-reference; evidence: E2, E3)
- AccessListenerStub::OnRemoteRequest::info → OnRequestHardwareAccess (passes-by-value-per-interface; evidence: E3, E4)

### Unresolved context

- None

## Quality Dimensions

| Dimension | Applicable | Level | Rationale | Evidence |
| --- | --- | --- | --- | --- |
| Functional correctness | yes | deep | Pass-by-value to pass-by-reference fix in IPC stub is a correctness-critical change; verified the call site in OnRemoteRequest and the IAccessListener interface to confirm the data flow. | access_listener_stub.h:31 ReadDeviceInfo(MessageParcel &data, AuthDeviceInfo &info)<br>access_listener_stub.cpp:55 ReadDeviceInfo(MessageParcel &data, AuthDeviceInfo &info)<br>access_listener_stub.cpp:43 AuthDeviceInfo info {"", "", 0}; ReadDeviceInfo(data, info); |
| API and compatibility | yes | screen | Screened for defect risk; no routed signal | - |
| State, lifecycle, concurrency | yes | screen | Screened for defect risk; no routed signal | - |
| Resource and memory | yes | screen | Screened for defect risk; no routed signal | - |
| Reliability and recovery | yes | screen | Screened for defect risk; no routed signal | - |
| Performance and scalability | no | not-applicable | Pass-by-reference avoids a copy of AuthDeviceInfo (small struct), marginal improvement only. | - |
| Build and portability | no | not-applicable | Header and source signature match; no build issue. | - |
| Test adequacy and observability | no | not-applicable | Test adequacy is outside current finding scope | - |
| Maintainability | yes | screen | Screened for defect risk; no routed signal | - |
| Security and privacy | no | not-applicable | No security-relevant change; the data is the same, just correctly populated now. | - |

## Changed-Path Coverage

| Path | Disposition | Rationale | Evidence |
| --- | --- | --- | --- |
| services/distributedhardwarefwkservice/include/ipc/access_listener_stub.h | direct | ReadDeviceInfo signature changed from value to reference parameter. | access_listener_stub.h:31 int32_t ReadDeviceInfo(MessageParcel &data, AuthDeviceInfo &info); |
| services/distributedhardwarefwkservice/src/ipc/access_listener_stub.cpp | direct | ReadDeviceInfo definition changed from value to reference parameter. | access_listener_stub.cpp:55 int32_t ReadDeviceInfo(MessageParcel &data, AuthDeviceInfo &info) |

## Flow Coverage

- F1 [Functional correctness]: AccessListenerStub::OnRemoteRequest → IPC deserialization: OnRemoteRequest creates local AuthDeviceInfo, calls ReadDeviceInfo(data, info) to populate it, then passes info to OnRequestHardwareAccess. With the old pass-by-value signature, ReadDeviceInfo wrote into a copy and the local info remained uninitialized, causing OnRequestHardwareAccess to receive empty/zero fields. (access_listener_stub.cpp:41-47; access_listener_stub.cpp:55-60)

## Failure Hypotheses

### H1: Functional correctness

- Statement: Before the fix, ReadDeviceInfo's pass-by-value parameter meant the deserialized AuthDeviceInfo fields (networkId, deviceName, deviceType) were written into a temporary copy that was discarded, so OnRequestHardwareAccess always received empty/default AuthDeviceInfo.
- Trigger: Any IPC call to ON_REQUEST_HARDWARE_ACCESS via AccessListenerStub
- Expected failure: OnRequestHardwareAccess receives AuthDeviceInfo with empty networkId, empty deviceName, and deviceType=0, causing access listener to operate on invalid device identity
- Outcome: supported
- Evidence: access_listener_stub.cpp:42-43: info is default-constructed as {"", "", 0}, then ReadDeviceInfo(data, info) is called; With pass-by-value, ReadDeviceInfo modifies its own copy; the caller's 'info' stays {"", "", 0}; access_listener_stub.cpp:46: OnRequestHardwareAccess(requestId, info, dhType, pkgName) passes unmodified info
- Verification: Static analysis: C++ pass-by-value semantics confirm the writes in ReadDeviceInfo do not propagate to the caller's variable

### H2: Functional correctness

- Statement: The fix may break binary compatibility if ReadDeviceInfo was called across translation units with the old mangled symbol name.
- Trigger: Linking against old object file referencing the pass-by-value mangled name
- Expected failure: Link error or undefined reference at build time
- Outcome: ruled-out
- Evidence: ReadDeviceInfo is a private method of AccessListenerStub, only called from OnRemoteRequest in the same .cpp file; No external callers exist
- Verification: ReadDeviceInfo is private; grep confirms single call site in access_listener_stub.cpp


## Findings

### F1: Finding 1

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
- Evidence: access_listener_stub.cpp:42-47; access_listener_stub.cpp:55-60
- Verification: Static code path review
- Temporal status: Current at scan HEAD; not yet fixed

## Checked Safe

- ReadDeviceInfo is private with single call site in same TU; no ABI risk
- IAccessListener interface unchanged; OnRequestHardwareAccess still takes AuthDeviceInfo by value per interface contract

## Exclusions

- None

## Coverage Gaps

- None

## Verification Disposition

- Status: static-analysis
- Rationale: C++ language semantics definitively confirm pass-by-value vs pass-by-reference behavior. No runtime verification needed.
- Evidence: access_listener_stub.cpp:42-47 call-site analysis

## Executed Checks

- `Verified ReadDeviceInfo is private and only called from OnRemoteRequest in same .cpp` — passed: reviewed
- `Verified IAccessListener::OnRequestHardwareAccess signature unchanged` — passed: reviewed
- `Verified AccessListenerProxy serialization is unaffected` — passed: reviewed
- `Verified AuthDeviceInfo struct has no special copy semantics that would mask the bug` — passed: reviewed

## Temporal Status

current
