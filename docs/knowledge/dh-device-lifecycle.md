# 设备生命周期知识

本文只记录设备上线/下线事件处理流程。分布式硬件使能/去使能的具体步骤见 `dh-component-enable.md`，任务调度机制见 `dh-task-pipeline.md`。

## 设备标识对照表

| 标识符 | 含义 | 获取时机 | 用途 | 禁止操作 |
| --- | --- | --- | --- | --- |
| networkId | 设备网络地址 | 设备上线后动态分配 | IPC 调用、数据同步、任务路由 | 不持久化、不跨会话使用 |
| uuid | 设备唯一标识 | 设备上线时获取 | 数据库 key、状态缓存、业务逻辑 | 不用于 IPC 远程调用 |
| udid | 设备绝对标识 | 设备上线时获取 | 设备认证、跨重置识别 | 不直接用于数据库 key |
| deviceId | uuid hash | uuid 计算 | 数据库短 key、日志 | 不用于 IPC |
| udidHash | udid hash | udid 计算 | 跨重置持久标识 | 不用于 IPC |

**核心规则**：networkId 用于通信，uuid 用于本地状态，udid 用于持久识别。混用会导致查询失败。

## 上线流程时序

| 步骤 | 触发方 | 处理类 | 关键操作 | 锚点代码 |
| --- | --- | --- | --- | --- |
| 1 | DeviceManager | `AccessManager` | `OnDeviceOnline` 回调 | `services/.../accessmanager/access_manager.h:45` |
| 2 | AccessManager | `DistributedHardwareManager` | `SendOnLineEvent` | `services/.../distributed_hardware_manager.h:34` |
| 3 | DHManager | `OnlineTask` | 创建上线任务 | `services/.../task/online_task.h` |
| 4 | OnlineTask | `ResourceManager` | 同步远端设备硬件信息 | `services/.../resourcemanager/capability_info_manager.h` |
| 5 | OnlineTask | `ComponentManager` | 使能分布式硬件部件 | `services/.../componentmanager/component_manager.h` |

**时序约束**：ResourceManager 初始化必须在 ComponentManager 使能之前完成。违反时序会导致 Enable 查询不到 CapabilityInfo。

## 下线流程时序

| 步骤 | 触发方 | 处理类 | 关键操作 | 锚点代码 |
| --- | --- | --- | --- | --- |
| 1 | DeviceManager | `AccessManager` | `OnDeviceOffline` 回调 | `services/.../accessmanager/access_manager.h:46` |
| 2 | AccessManager | `DistributedHardwareManager` | `SendOffLineEvent` | `services/.../distributed_hardware_manager.h:36` |
| 3 | DHManager | `OfflineTask` | 创建下线任务 | `services/.../task/offline_task.h` |
| 4 | OfflineTask | `ComponentManager` | 去使能分布式硬件部件 | `services/.../componentmanager/component_manager.h` |
| 5 | OfflineTask | `ResourceManager` | 清理远端设备数据 | `services/.../resourcemanager/capability_info_manager.h` |

## 反模式/修改前检查

- **禁止**用 udid 直接查询 CapabilityInfo 数据库，数据库 key 是 uuid。
- **禁止**在上线回调中直接调用 Enable，必须通过 SendOnLineEvent 走任务流程。
- **禁止**跳过 OnlineTask 的 SYNC_ONLINE_INFO 步骤，否则 ResourceManager 无数据。
- 修改 AccessManager 回调逻辑时，检查是否影响 SendOnLineEvent/SendOffLineEvent 的调用时机。
- 新增设备标识字段时，检查是否已添加到 DeviceInfo 结构体（`common/utils/include/device_type.h:61`）。

## 测试指引

- AccessManager 回调逻辑修改：使用 `AccessManagerTest`。
- SendOnLineEvent/SendOffLineEvent 修改：使用 `DistributedHardwareManagerTest`。
- OnlineTask/OfflineTask 修改：使用 `OnlineTaskTest`、`OfflineTaskTest`。
- ResourceManager 数据同步修改：使用 `ResourceManagerTest`、`CapabilityInfoManagerTest`。
- 涉及真实设备上下线时，补充板侧 log 和设备状态截图。