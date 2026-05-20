# 任务执行管道知识

本文只记录任务创建、调度、执行的流程和并发控制。设备上下线事件见 `dh-device-lifecycle.md`，使能/去使能具体步骤见 `dh-component-enable.md`。

## 任务类型和步骤

| TaskType | 含义 | TaskStep 序列 | 创建入口 | 锚点 |
| --- | --- | --- | --- | --- |
| ON_LINE | 设备上线 | SYNC_ONLINE_INFO → REGISTER_ONLINE_DISTRIBUTED_HARDWARE | `SendOnLineEvent` | `impl_utils.h:30` |
| OFF_LINE | 设备下线 | UNREGISTER_OFFLINE_DISTRIBUTED_HARDWARE → CLEAR_OFFLINE_INFO → WAIT_UNREGISTGER_COMPLETE | `SendOffLineEvent` | `impl_utils.h:31` |
| ENABLE | 使能硬件 | DO_ENABLE → ENABLE_SINK | `ComponentManager::Enable` | `impl_utils.h:28` |
| DISABLE | 去使能硬件 | DO_DISABLE → DISABLE_SINK | `ComponentManager::Disable` | `impl_utils.h:29` |
| META_ENABLE | 元数据使能 | META_ENABLE_TASK → DO_MODEM_META_ENABLE | MODEM 特殊流程 | `impl_utils.h:32` |
| META_DISABLE | 元数据去使能 | META_DISABLE_TASK → DO_MODEM_META_DISABLE | MODEM 特殊流程 | `impl_utils.h:33` |
| EXIT_DFWK | 框架退出 | 停止所有任务 | `Release` | `impl_utils.h:34` |

## 任务状态流转

| TaskState | 含义 | 可转换到 | 触发条件 |
| --- | --- | --- | --- |
| INIT | 初始 | RUNNING | 任务被 TaskBoard 添加 |
| RUNNING | 执行中 | SUCCESS / FAIL | 任务执行完成 |
| SUCCESS | 成功 | 无 | 所有 TaskStep 完成 |
| FAIL | 失败 | 无 | 任一 TaskStep 失败或超时 |

状态定义见 `impl_utils.h:53-58`。

## TaskBoard 并发控制

| 操作 | 方法 | 并发行为 | 锚点 |
| --- | --- | --- | --- |
| 添加任务 | `AddTask` | 相同 taskId 的任务会被覆盖 | `task_board.h:35` |
| 移除任务 | `RemoveTask` | 任务完成后移除 | `task_board.h:36` |
| 等待完成 | `WaitForALLTaskFinish` | 阻塞直到所有任务完成 | `task_board.h:37` |
| 记录使能 | `SaveEnabledDevice` | 以 networkId+dhId 为 key | `task_board.h:38` |

**核心规则**：相同 networkId+dhId 组合的 Enable/Disable 任务不可并行，TaskBoard 会阻塞后续任务直到前一个完成。

## 任务执行顺序约束

| 父任务 | 子任务 | 执行要求 | 原因 |
| --- | --- | --- | --- |
| OnlineTask | EnableTask | 串行，Online 完成 Enable 才开始 | Enable 需要 CapabilityInfo |
| OfflineTask | DisableTask | 串行，Disable 完成才清理数据 | 数据清理需去使能完成 |
| EnableTask | 子 EnableTask | 并行，不同 dhId 可同时使能 | 不同硬件无依赖 |

**禁止**修改任务执行顺序，违反约束会导致数据不一致或使能失败。

## TaskFactory 创建规则

| 任务类型 | 创建方法 | 参数来源 | 锚点 |
| --- | --- | --- | --- |
| ON_LINE | `CreateOnlineTask` | networkId, uuid, udid, deviceType | `task_factory.h` |
| OFF_LINE | `CreateOfflineTask` | networkId, uuid, udid, deviceType | `task_factory.h` |
| ENABLE | `CreateEnableTask` | TaskParam (networkId, uuid, dhId, dhType) | `task_factory.h` |
| DISABLE | `CreateDisableTask` | TaskParam (networkId, uuid, dhId, dhType) | `task_factory.h` |

TaskParam 结构见 `impl_utils.h:77-100`。

## 反模式/修改前检查

- **禁止**在相同 networkId+dhId 上并行发起 Enable 和 Disable，会产生竞态。
- **禁止**跳过 OnlineTask 的 SYNC_ONLINE_INFO 步骤，否则 EnableTask 无数据。
- **禁止**在 OfflineTask 未完成时触发新设备上线，状态可能不一致。
- 修改 TaskStep 序列时，检查是否影响 TaskBoard 的等待逻辑。
- 新增 TaskType 时，必须在 TaskFactory 中添加对应的创建方法。
- 任务超时默认由 ComponentMonitor 处理，超时错误码为 ERR_DH_FWK_TASK_TIMEOUT。

## 测试指引

- Task 基类修改：使用 `TaskTest`。
- OnlineTask/OfflineTask 修改：使用 `OnlineTaskTest`、`OfflineTaskTest`。
- EnableTask/DisableTask 修改：使用 `EnableTaskTest`、`DisableTaskTest`。
- TaskBoard 并发逻辑修改：在 TaskTest 中验证 `WaitForALLTaskFinish`。
- TaskFactory 创建逻辑修改：检查各任务类型的测试覆盖。