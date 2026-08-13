# Review Result: merge-c2fe6a5cd366

- Status: completed
- Classification: Quality improvement
- Frozen HEAD: `5061f60a70ddf0a5e60feae364c792b228b4bd94`
- Attempt: 1

## Summary

Refactors const std::string constants in av_trans_types.h and constants.h to constexpr const char* to avoid static initialization order fiasco (SIOF) and reduce runtime overhead. Adds inline to DHTypeStrMap and DHTypePrefixMap in device_type.h to fix ODR violations across translation units. All concatenation sites in softbus_channel_adapter.cpp and dh_transport.cpp are correctly adapted with std::string() wrappers. RESOURCE_SEPARATOR concatenation in dh_context_test.cpp correctly wrapped. No code defects found.

## Review Protocol

- Review depth: full
- Protocol version: 2
- Primary dimensions: Functional correctness

## Context Inspected

- Level: local-symbol
- Self-contained: yes
- Rationale: Changes are type substitutions for compile-time constants; all use sites within the same repository are adapted in the same commit
- Changed symbols: OWNER_NAME_D_CAMERA; OWNER_NAME_D_SCREEN; OWNER_NAME_D_MIC; OWNER_NAME_D_SPEAKER; OWNER_NAME_D_VIRMODEM_MIC; OWNER_NAME_D_VIRMODEM_SPEAKER; SCENE_TYPE_D_MIC; SCENE_TYPE_D_SCREEN; SCENE_TYPE_D_SPEAKER; SCENE_TYPE_D_CAMERA_STR; SCENE_TYPE_D_CAMERA_PIC; PKG_NAME_DH_FWK; PKG_NAME_D_AUDIO; PKG_NAME_D_CALL; PKG_NAME_D_CAMERA; PKG_NAME_D_SCREEN; MIME_VIDEO_RAW; MIME_VIDEO_H264; MIME_VIDEO_H265; VIDEO_FORMAT_NV12; VIDEO_FORMAT_NV21; VIDEO_FORMAT_JEPG; VIDEO_FORMAT_YUVI420; VIDEO_FORMAT_RGBA8888; RESOURCE_SEPARATOR; DH_FWK_PKG_NAME; DHTypeStrMap; DHTypePrefixMap

| Evidence | Disposition | Path | Symbol | Relation | Source ref |
| --- | --- | --- | --- | --- | --- |
| E1 | changed | av_transport/common/include/av_trans_types.h | OWNER_NAME_D_* | type-changed | 74fc6846b46b7f35d773e9b94497632f23607386 |
| E2 | unchanged | av_transport/common/include/av_trans_constants.h | SENDER_CONTROL_SESSION_NAME_SUFFIX | unchanged-type-still-const-std-string | 74fc6846b46b7f35d773e9b94497632f23607386 |
| E3 | changed | av_transport/common/src/softbus_channel_adapter.cpp | LOCAL_TO_PEER_SESSION_NAME_MAP | adapted-concatenation | 74fc6846b46b7f35d773e9b94497632f23607386 |
| E4 | changed | common/utils/include/constants.h | DH_FWK_PKG_NAME | type-changed | 74fc6846b46b7f35d773e9b94497632f23607386 |
| E5 | changed | common/utils/include/device_type.h | DHTypeStrMap | inline-added | 74fc6846b46b7f35d773e9b94497632f23607386 |
| E6 | changed | common/utils/include/device_type.h | DHTypePrefixMap | inline-added | 74fc6846b46b7f35d773e9b94497632f23607386 |
| E7 | changed | services/distributedhardwarefwkservice/src/transport/dh_transport.cpp | DHTransport::CreateServerSocket | adapted-c_str-removed | 74fc6846b46b7f35d773e9b94497632f23607386 |

### Verified flow edges

- OWNER_NAME_D_MIC → softbus_channel_adapter.cpp:LOCAL_TO_PEER_SESSION_NAME_MAP (string-concatenation; evidence: E1, E3)
- DH_FWK_PKG_NAME → DHTransport::CreateServerSocket (used-as-pkgName; evidence: E4, E7)

### Unresolved context

- None

## Quality Dimensions

| Dimension | Applicable | Level | Rationale | Evidence |
| --- | --- | --- | --- | --- |
| Functional correctness | yes | deep | const std::string to constexpr const char* refactoring changes string literal type and requires adaptation at all concatenation sites; reviewed all use sites for correctness | av_trans_types.h: 25 constants changed to constexpr const char*<br>constants.h: RESOURCE_SEPARATOR and DH_FWK_PKG_NAME changed to constexpr const char*<br>All concatenation sites in softbus_channel_adapter.cpp wrapped with std::string()<br>dh_transport.cpp: DH_FWK_PKG_NAME used directly without .c_str() - correct for const char* |
| API and compatibility | yes | screen | Screened for defect risk; no routed signal | - |
| State, lifecycle, concurrency | no | not-applicable | No concurrency changes | - |
| Resource and memory | yes | screen | Screened for defect risk; no routed signal | - |
| Reliability and recovery | yes | screen | Screened for defect risk; no routed signal | - |
| Performance and scalability | yes | screen | constexpr const char* eliminates static initialization overhead for string constants | Compile-time constants replace runtime-initialized std::string objects |
| Build and portability | yes | screen | Screened for defect risk; no routed signal | - |
| Test adequacy and observability | no | not-applicable | Test adequacy is outside current finding scope | - |
| Maintainability | yes | screen | Screened for defect risk; no routed signal | - |
| Security and privacy | no | not-applicable | No security-relevant changes | - |

## Changed-Path Coverage

| Path | Disposition | Rationale | Evidence |
| --- | --- | --- | --- |
| av_transport/av_trans_engine/plugin/test/av_trans_input/daudio_input_test/daudio_input_test.cpp | direct | Test code wraps OWNER_NAME_D_SCREEN in std::string() for concatenation; correct adaptation | std::string(OWNER_NAME_D_SCREEN) + "_" + suffix |
| av_transport/common/include/av_trans_types.h | direct | 25 constants changed from const std::string to constexpr const char*; all values preserved | constexpr const char* OWNER_NAME_D_CAMERA = "ohos.dhardware.dcamera" |
| av_transport/common/src/softbus_channel_adapter.cpp | direct | LOCAL_TO_PEER_SESSION_NAME_MAP entries wrapped with std::string() for concatenation with SENDER/RECEIVER suffixes (still const std::string in av_trans_constants.h) | std::string(OWNER_NAME_D_MIC) + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX |
| common/utils/include/constants.h | direct | RESOURCE_SEPARATOR and DH_FWK_PKG_NAME changed from const std::string to constexpr const char* | constexpr const char *RESOURCE_SEPARATOR = "###" |
| common/utils/include/device_type.h | direct | DHTypeStrMap and DHTypePrefixMap changed from const to inline const; C++17 inline variables for proper ODR | inline const std::unordered_map<DHType, std::string> DHTypeStrMap = { ... } |
| services/distributedhardwarefwkservice/src/transport/dh_transport.cpp | direct | DH_FWK_PKG_NAME.c_str() changed to DH_FWK_PKG_NAME (now const char*); correct for const_cast<char*> and strdup | const_cast<char*>(DH_FWK_PKG_NAME) instead of const_cast<char*>(DH_FWK_PKG_NAME.c_str()) |
| services/distributedhardwarefwkservice/test/fuzztest/dhtransportone_fuzzer/dhtransportone_fuzzer.cpp | direct | strdup(DH_FWK_PKG_NAME.c_str()) changed to strdup(DH_FWK_PKG_NAME); correct since DH_FWK_PKG_NAME is now const char* | strdup(DH_FWK_PKG_NAME) |
| services/distributedhardwarefwkservice/test/fuzztest/dhtransporttwo_fuzzer/dhtransporttwo_fuzzer.cpp | direct | Same adaptation as dhtransportone_fuzzer | strdup(DH_FWK_PKG_NAME) |
| services/distributedhardwarefwkservice/test/unittest/common/componentmanager/component_manager/src/component_manager_test.cpp | direct | DH_FWK_PKG_NAME.c_str() changed to DH_FWK_PKG_NAME for MockGetLocalNodeDeviceInfo call | MockGetLocalNodeDeviceInfo(DH_FWK_PKG_NAME, info.get()) |
| services/distributedhardwarefwkservice/test/unittest/common/utils/dh_context_test.cpp | direct | "prefix" + RESOURCE_SEPARATOR changed to std::string("prefix") + RESOURCE_SEPARATOR; prevents pointer arithmetic on const char* | std::string("prefix") + RESOURCE_SEPARATOR |

## Flow Coverage

- F1 [Functional correctness]: softbus_channel_adapter.cpp:LOCAL_TO_PEER_SESSION_NAME_MAP → Session name map construction: constexpr const char* OWNER_NAME_* constants concatenated with const std::string SENDER/RECEIVER suffixes via std::string() wrapper (std::string(OWNER_NAME_D_MIC) + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX yields correct std::string result)

## Failure Hypotheses

### H1: Functional correctness

- Statement: String concatenation with constexpr const char* and const char* literal without std::string wrapper would perform pointer arithmetic instead of string concatenation
- Trigger: Code does OWNER_NAME_D_MIC + "_" + SENDER_CONTROL_SESSION_NAME_SUFFIX where OWNER_NAME_D_MIC is constexpr const char* and the literal is const char*
- Expected failure: Pointer arithmetic produces wrong memory address, crash or garbage string
- Outcome: ruled-out
- Evidence: All concatenation sites correctly wrap at least the first operand in std::string(); softbus_channel_adapter.cpp uses std::string(OWNER_NAME_D_MIC) + "_" + suffix pattern; dh_context_test.cpp uses std::string("prefix") + RESOURCE_SEPARATOR
- Verification: Reviewed all concatenation sites in the diff; all correctly use std::string() wrapper


## Findings

No structured findings.
## Checked Safe

- std::string() wrapping for constexpr const char* concatenation in softbus_channel_adapter.cpp
- DH_FWK_PKG_NAME direct use (no .c_str()) in dh_transport.cpp
- inline keyword for DHTypeStrMap and DHTypePrefixMap provides correct ODR semantics
- std::string("prefix") + RESOURCE_SEPARATOR prevents pointer arithmetic in dh_context_test.cpp
- SENDER_CONTROL_SESSION_NAME_SUFFIX remains const std::string in av_trans_constants.h - compatible with std::string + std::string concatenation

## Exclusions

- None

## Coverage Gaps

- None

## Verification Disposition

- Status: static-analysis
- Rationale: Type refactoring verified by code inspection; all use sites adapted correctly
- Evidence: Reviewed all changed files and concatenation patterns

## Executed Checks

- `git diff + source read` — passed: Reviewed changed code paths against invariants

## Temporal Status

current
