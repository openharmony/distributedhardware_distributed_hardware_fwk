# 分布式硬件使能知识

本文只记录分布式硬件使能/去使能的流程和状态管理（含数据库存储架构）。设备上下线事件见 `dh-device-lifecycle.md`，任务调度机制见 `dh-task-pipeline.md`，DHType 类型差异见 `dh-type-taxonomy.md`。

## 使能流程阶段

| 阶段 | 入口 | 关键操作 | 失败后果 | 锚点代码 |
| --- | --- | --- | --- | --- |
| 参数获取 | `GetEnableParam` | 从 ResourceManager 查询 CapabilityInfo | 返回 ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND | `component_manager.h:253` |
| 版本协商 | `GetVersion` | 从 VersionManager 获取版本 | 返回 ERR_DH_FWK_COMPONENT_COMPVERSION_NOT_FOUND | `component_manager.h:255` |
| Source 启动 | `StartSource` | 调用 IDistributedHardwareSource::Enable | 资源端驱动未注册 | `component_manager.h:248` |
| Sink 启动 | `StartSink` | 调用 IDistributedHardwareSink::Enable | 使用端驱动未注册 | `component_manager.h:250` |
| 状态记录 | `SaveEnabledDevice` | TaskBoard 记录使能状态 | 状态丢失，重复使能 | `task_board.h:38` |

## 使能前置条件

| 条件 | 检查方法 | 不满足时的错误码 | 处理建议 |
| --- | --- | --- | --- |
| 设备已上线 | `AccessManager::IsDeviceOnline(uuid)` | 无显式错误，但后续查询失败 | 确保上线流程完成 |
| CapabilityInfo 存在 | `CapabilityInfoManager::GetDataByKey(uuid)` | ERR_DH_FWK_RESOURCE_CAPABILITY_MAP_NOT_FOUND | 等待数据同步完成 |
| 版本信息存在 | `VersionInfoManager::GetVersionInfoByUuid(uuid)` | ERR_DH_FWK_COMPONENT_COMPVERSION_NOT_FOUND | 等待版本同步完成 |
| 组件已加载 | `ComponentLoader::GetHandler(dhType)` | ERR_DH_FWK_LOADER_HANDLER_IS_NULL | 检查配置文件 |

## 数据库存储架构

| 数据库 | StoreId | 用途 | 管理器 | 数据来源 |
| --- | --- | --- | --- | --- |
| CapabilityInfo | capability_info | 存储设备硬件能力信息 | CapabilityInfoManager | 从远端数据库同步 |
| MetaInfo | global_meta_info | 存储分布式硬件基本信息 | MetaInfoManager | 从对端数据库同步 |
| VersionInfo | version_info | 存储版本信息 | VersionInfoManager | 设备上线时同步 |

数据库类型为 DistributedKv（分布式键值数据库），具体实现为 SingleKvStore，命名空间 `DistributedKv`。

| 类名 | 文件位置 | 功能 | 锚点 |
| --- | --- | --- | --- |
| DBAdapter | `resourcemanager/db_adapter.h` | KV 存储初始化、数据增删改查、变更监听 | `db_adapter.h` |
| CapabilityInfoManager | `resourcemanager/capability_info_manager.h` | 设备硬件能力信息管理 | `capability_info_manager.h` |
| MetaInfoManager | `resourcemanager/meta_info_manager.h` | 分布式硬件基本信息管理 | `meta_info_manager.h` |

**数据同步触发方式**：
- 分布式相机/音频：使能成功后主动调用 `OnDataSyncTrigger` 触发数据同步到 meta_info
- 分布式通信（MODEM）：通过 KvStore 自动同步机制，设备上线/数据变更时自动触发

## 去使能流程阶段

| 阶段 | 入口 | 关键操作 | 失败后果 | 锚点代码 |
| --- | --- | --- | --- | --- |
| 状态检查 | `IsEnabledDevice` | 检查 TaskBoard 使能记录 | 未使能状态，跳过 | `task_board.h:43` |
| Source 停止 | `StopSource` | 调用 IDistributedHardwareSource::Disable | 资源端驱动残留 | `component_manager.h:249` |
| Sink 停止 | `StopSink` | 调用 IDistributedHardwareSink::Disable | 使用端驱动残留 | `component_manager.h:251` |
| 数据清理 | `RemoveEnabledDevice` | TaskBoard 清除使能状态 | 状态不一致 | `task_board.h:39` |

## 自动使能 vs 主动使能

### 配置判断规则

| 类型 | source_feature_filter | 典型组件 | 判断逻辑 |
| --- | --- | --- | --- |
| 自动使能 | **不存在**（无字段） | MODEM (0x200) | 字段缺失 → 自动使能 |
| 主动使能 | **存在且为空** "" | CAMERA、AUDIO、SCREEN、INPUT | 字段存在 → 主动使能 |

配置路径：`etc/distributedhardware/distributed_hardware_components_cfg.json`（见 `constants.h:57`）。

### 触发与数据差异

| 维度 | 自动使能（MODEM） | 主动使能（CAMERA/AUDIO） |
| --- | --- | --- |
| 触发入口 | OnlineTask 内部 → META_ENABLE | 应用 EnableSink → EnableTask |
| 触发时机 | 设备上线后自动执行 | 应用显式调用 |
| 数据依赖 | MetaInfo（KvStore 自动同步） | CapabilityInfo（上线同步） |
| 参数获取 | GetEnableMetaParam | GetEnableCapParam |
| Source 启动 | EnableMetaSourceInternal | StartSource |

**关键差异**：自动使能在上线流程中执行，依赖 MetaInfo 自动同步；主动使能需应用调用，依赖 CapabilityInfo。

### 典型组件对照

| DHType | 名称 | 使能类型 | source_feature_filter | 触发条件 |
| --- | --- | --- | --- | --- |
| 0x200 | MODEM | 自动 | 不存在 | 设备上线 + MetaInfo 同步 |
| 0x01 | CAMERA | 主动 | 存在（空） | 应用调用 EnableSink |
| 0x02 | AUDIO | 主动 | 存在（空） | 应用调用 EnableSink |
| 0x08 | SCREEN | 主动 | 存在（空） | 应用调用 EnableSink |

**规则**：新增组件必须明确配置 source_feature_filter，否则判定为自动使能。

## 业务状态管理

| 状态 | 含义 | 触发时机 | 锚点 |
| --- | --- | --- | --- |
| BusinessState::IDLE | 空闲 | 设备上线/去使能完成 | `component_manager.h:64` |
| BusinessState::BUSINESS | 业务运行 | 使能成功后 | `component_manager.h:64` |

状态变更通过 `UpdateBusinessState` 方法，需在 ComponentManager 的业务逻辑中调用。

## 反模式/修改前检查

- **禁止**在 SendOnLineEvent 未完成时调用 Enable，ResourceManager 无数据会失败。
- **禁止**跳过 TaskBoard 状态记录，否则 Disable 无法判断是否已使能。
- **禁止**修改 EnableParam 的 dhId 字段，dhId 由远端设备决定，本地不可修改。
- 修改使能逻辑时，检查是否影响 EnableSink/EnableSource 的引用计数机制。
- 新增 DHType 时，必须在配置文件中明确 source_feature_filter 字段。

## 测试指引

- ComponentManager 使能逻辑修改：使用 `ComponentManagerTest`、`ComponentEnableTest`、`ComponentDisableTest`。
- EnableTask/DisableTask 修改：使用 `EnableTaskTest`、`DisableTaskTest`。
- TaskBoard 状态管理修改：使用 `TaskBoardTest`（无独立测试文件，在 TaskTest 中覆盖）。
- ResourceManager 数据查询修改：使用 `ResourceManagerTest`。
- 涉及真实设备使能/去使能时，补充板侧驱动状态和日志。