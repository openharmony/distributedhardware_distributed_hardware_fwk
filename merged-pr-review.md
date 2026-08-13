# AI Quality Harness — distributed_hardware_fwk Merged PR Review

## 审查概要

| 项目 | 值 |
|------|-----|
| 源码仓 | C:\code\0813\ai-code-harness-7.0\03-distributed-ai\distributed_hardware_fwk |
| 目标分支 | master (HEAD: 5061f60a70ddf0a5e60feae364c792b228b4bd94) |
| 审查范围 | 2026-03-01 至 2026-08-12 (Asia/Shanghai, UTC+8) |
| 路径过滤 | services/ |
| 提交总数（全仓） | 71 |
| 范围内单元 | 24 |
| 范围外跳过 | 47 |
| 发现问题 | 10 |
| Ledger 条目 | 3 |
| 验证方式 | 静态代码审查（local-only） |

## 变更摘要

| 日期区间 | 单元数 | 主要变更 |
|-----------|--------|----------|
| 2026-03 ~ 04 | 5 | DHDescriptor 序列化扩展、ComponentLoader 监控、IPC stub 修复 |
| 2026-05 ~ 06 | 7 | 权限检查移除与恢复、EnableTask cJSON tokenId、Build/fuzz 配置 |
| 2026-07 | 9 | SA 状态 HiSysEvent、DB 适配器/元数据管理、dhfwkInit 计数、组件去使能 |
| 2026-08 | 3 | 组件使能/去使能修正、Resource Manager/OnlineTask 重构 |

## 当前可操作缺陷

### 1. [High] IPC 序列化格式不兼容 — DHDescriptor 代理/桩序列化漂移

- **维度**: API and compatibility
- **影响**: 新旧版本 proxy/stub 通信时 parcel 数据错位，可能导致使能操作失败或崩溃
- **触发**: 旧格式 proxy 与新格式 stub 跨进程通信
- **涉及单元**: merge-01b5c0c0fefa, merge-660ebe708b80
- **证据**: `distributed_hardware_proxy.cpp` 中 WriteDescriptors 写入 dhType/id/customParams/firstCallingTokenId 四字段，旧版仅三字段；stub 侧 ReadDescriptors 读取四字段
- **Ledger ID**: AQH-F95A6C4D0ECF327A
- **建议验证**: 测试旧 proxy 与新 stub 之间的 IPC 通信

### 2. [High] 安全检查移除窗口 — CheckDHAccessPermission 被删除后未立即替换

- **维度**: Security and privacy
- **影响**: LoadSinkDMSDPService 和 NotifySinkRemoteSourceStarted 入口在权限检查被删除期间可被未授权调用
- **触发**: SA 入口在 merge-63c841fd613b 合入后、后续修复合入前被调用
- **涉及单元**: merge-63c841fd613b
- **证据**: `distributed_hardware_service.cpp` 中 `if (CheckDHAccessPermission(udid) != DH_FWK_SUCCESS)` 被替换为 `(void)udid;`
- **Ledger ID**: AQH-22B441E144A63981
- **建议验证**: 检查设备日志中是否存在未授权 SA 调用

### 3. [High] 持锁 IPC 回调 — dhfwkInit 路径在锁内触发回调

- **维度**: State, lifecycle, concurrency
- **影响**: StartGetDeviceDhInfo 在 localInitMgrMutex_ 持有期间调用 callback->OnSuccess，可能导致死锁
- **触发**: DHFWK 初始化路径在持锁状态下调用远程回调
- **涉及单元**: merge-95c2b253a6ec
- **证据**: `distributed_hardware_service.cpp:562` 持锁调用 StartGetDeviceDhInfo

### 4. [Medium] cJSON tokenId 精度 — AddNumberToObject 用 double 存 uint32_t

- **维度**: Functional correctness
- **影响**: cJSON.valueint 字段为 int32_t，tokenId > 2^31-1 时截断；依赖 valueint 的下游解析器会读到错误值
- **触发**: tokenId > 2147483647 的场景
- **涉及单元**: merge-c05632ffcde9
- **证据**: `enable_task.cpp:233` 和 `component_manager.cpp:354` 使用 `cJSON_AddNumberToObject(json, "tokenId", static_cast<double>(tokenId))`

### 5. [Medium] dhfwkInitTimes_ 计数永不重置

- **维度**: Reliability and recovery
- **影响**: DHFWK 重复初始化/去初始化后 dhfwkInitTimes_ 只增不减，SA 退出后重新加载状态不一致
- **触发**: DHFWK 服务退出后重新启动
- **涉及单元**: merge-95c2b253a6ec

### 6. [Medium] ReadDeviceInfo 传值 bug — AccessListenerStub 参数拷贝导致信息丢失

- **维度**: Functional correctness
- **影响**: ReadDeviceInfo 以值传递接收 DHItem，导致原始设备信息丢失
- **触发**: 设备信息回调
- **涉及单元**: merge-7b7b5d5a8196
- **分类**: Fixes defect（本提交修复了此 bug）

### 7. [Low] InitCompSink 错误路径无回滚

- **维度**: Resource and memory
- **影响**: InitCompSink 失败时 compSink_ 和 compSinkSaId_ 已填充但未回滚
- **触发**: compMonitorPtr_ 为空（当前不可达路径）
- **涉及单元**: merge-7be94f4daad0

### 8. [Low] udidHash 匹配被移除

- **维度**: Functional correctness
- **影响**: Enable 路径不再检查 udidHash 与目标设备匹配，可能导致设备信息错误
- **触发**: 多设备场景下设备标识不匹配
- **涉及单元**: merge-660ebe708b80

### 9. [Low] OnGetDescriptors 测试依赖泄漏状态

- **维度**: Functional correctness
- **影响**: OnGetDescriptors_001 断言依赖前置测试泄漏的 syncDeviceInfoMap_ 状态
- **触发**: 单独运行该测试用例
- **涉及单元**: merge-df3392cc534b

### 10. [Low] 误导性日志 — GetDeviceId vs GetUdidHash

- **维度**: Functional correctness
- **影响**: 错误日志显示 GetDeviceId() 值但查找使用 GetUdidHash()，导致排查困难
- **触发**: Enable 失败路径
- **涉及单元**: merge-f9cbdac4550c

## 不确定行为风险

| 风险 | 维度 | 置信度 | 说明 |
|------|------|--------|------|
| 82a3c808df82: 待处理请求清除 | Functional correctness | Medium | Init 成功时清空 pendingRequests 但未检查是否有进行中请求 |
| 82a3c808df82: 权限执行不一致 | Security and privacy | Medium | 部分入口检查权限，部分不检查 |

## 质量维度统计

| 维度 | 深度审查单元数 |
|------|---------------|
| Functional correctness | 17 |
| Build and portability | 3 |

## 分类分布

| 分类 | 数量 |
|------|------|
| No material quality impact | 7 |
| Quality improvement | 5 |
| Introduces defect | 4 |
| Fixes defect | 3 |
| Residual quality risk | 3 |
| Needs manual review | 0 |

## 覆盖与缺口

- **审查深度**: 17 个 full-review + 7 个 focused-review
- **Challenge**: 0 个单元需要独立 challenge（无 NAPI/ANI/NDK/security/state-lifecycle 信号）
- **Cluster**: 1 个跨单元集群（18 个成员），发现 2 个交互缺陷
- **验证缺口**: 所有审查为静态代码分析；未执行编译或运行时验证
- **设备侧未验证项**: 设备上下线、分布式硬件使能/去使能、IPC 跨版本通信

## 附录：协议核算

- Index digest: sha256:adea0290dcdb4b7b9d767a6a478a0dcd7ba3a63552e8513a169c4efd633c9e64
- Frozen scan HEAD: 5061f60a70ddf0a5e60feae364c792b228b4bd94
- Risk policy version: 2
- Review protocol version: 2
- Primary wave: 1
- Sampling: 0 of 0 low-risk units escalated (none eligible)
- Challenge required: 0 units
- Cluster count: 1 (18 members)
