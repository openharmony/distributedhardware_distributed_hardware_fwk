# IPC 服务架构知识

本文只记录 SA 服务架构、IPC 接口定义和跨进程调用链（含 callingUid/userId 区分）。设备上下线事件见 `dh-device-lifecycle.md`，使能/去使能流程见 `dh-component-enable.md`。

## SA 服务架构

| 服务 | SA ID | 职责 | 生命周期 | 锚点 |
| --- | --- | --- | --- | --- |
| DistributedHardwareService | 动态分配 | 主服务入口，提供北向 API | 系统启动时拉起 | `distributed_hardware_service.h:37` |
| AccessManager | 无 SA ID | 设备上下线监听（依赖 DeviceManager） | 与主服务同步初始化 | `access_manager.h:30` |
| ComponentManager | 无 SA ID | 组件使能/去能管理 | 与主服务同步初始化 | `component_manager.h:49` |
| AVTransControlCenter | 动态分配 | AV 传输控制中心 | InitializeAVCenter 时创建 | `av_trans_control_center.h:28` |

服务启动状态枚举见 `distributed_hardware_service.h:32-35`。

## IPC 调用身份识别

IPC 调用涉及两个层级身份标识，不可混用：

| 标识 | 名称 | 获取方式 | 含义 | 层级 | 用途 |
| --- | --- | --- | --- | --- | --- |
| callingUid | 调用方Uid | `GetCallingUid()` | 调用方应用身份 | 应用/进程级别 | 权限校验、区分调用方 |
| userId | 系统账号 | `OsAccountManager::GetForegroundOsAccountLocalId()` | 系统用户身份 | 用户级别 | 数据隔离、应用安装 |

**核心规则**：callingUid 用于 IPC 权限校验，userId 用于用户空间隔离。两者是不同层级概念，禁止混用。

系统账号查询接口（命名空间 `AccountSA::OsAccountManager`）：
- `GetForegroundOsAccountLocalId()`：获取前台用户 ID
- `QueryActiveOsAccountIds()`：获取活跃用户 ID 列表
- `QueryOsAccountById(userId)`：查询指定用户信息

## IPC 接口分层

| 层次 | 文件位置 | 用途 | 调用方 |
| --- | --- | --- | --- |
| Stub 层 | `distributed_hardware_stub.h` | 服务端 IPC 接口实现 | SA 内部 |
| Proxy 层 | `distributed_hardware_fwk_kit.h` | 客户端调用代理 | 上层应用 |
| NAPI 层 | `interfaces/kits/napi/` | JS API 封装 | JS 应用 |

## 核心 IPC 方法

| 方法 | 方向 | 参数 | 用途 | 锚点 |
| --- | --- | --- | --- | --- |
| EnableSink | Client→SA | DHDescriptor | 使能 Sink 端硬件 | `distributed_hardware_service.h:63` |
| DisableSink | Client→SA | DHDescriptor | 去使能 Sink 端硬件 | `distributed_hardware_service.h:64` |
| EnableSource | Client→SA | networkId, DHDescriptor | 使能 Source 端硬件 | `distributed_hardware_service.h:65` |
| DisableSource | Client→SA | networkId, DHDescriptor | 去使能 Source 端硬件 | `distributed_hardware_service.h:66` |
| GetDistributedHardware | Client→SA | networkId, enableStep, callback | 查询远端硬件信息 | `distributed_hardware_service.h:57` |
| RegisterPublisherListener | Client→SA | topic, listener | 注册消息发布监听 | `distributed_hardware_service.h:42` |
| InitializeAVCenter | Client→SA | transRole, engineId | 初始化 AV 传输引擎 | `distributed_hardware_service.h:48` |

## 调用链关键点

| 调用场景 | 入口 | 中间层 | 最终处理 | 关键错误码 |
| --- | --- | --- | --- | --- |
| 应用使能 Sink | EnableSink | DistributedHardwareService | ComponentManager::EnableSink | ERR_DH_FWK_COMPONENT_ENABLE_FAILED |
| 应用使能 Source | EnableSource | DistributedHardwareService | ComponentManager::EnableSource | ERR_DH_FWK_COMPONENT_ENABLE_FAILED |
| 查询远端硬件 | GetDistributedHardware | DistributedHardwareService | SyncRemoteDeviceInfoBySoftbus | ERR_DH_FWK_GETDISTRIBUTEDHARDWARE_TIMEOUT |
| AV 初始化 | InitializeAVCenter | DistributedHardwareService | AVTransControlCenter::InitializeAVCenter | ERR_DH_FWK_SERVICE_IPC_SEND_REQUEST_FAIL |

## IPC 接口码定义

接口码定义在 `dhardware_ipc_interface_code.h`，用于 IPC 消息识别。新增 IPC 方法时必须添加对应接口码。

## 回调代理

| 回调接口 | 用途 | 锚点 |
| --- | --- | --- |
| IPublisherListener | 消息发布回调 | `ipublisher_listener.h` |
| IAvTransControlCenterCallback | AV 传输控制回调 | `iav_trans_control_center_callback.h` |
| IGetDhDescriptorsCallback | 获取硬件描述符回调 | `iget_dh_descriptors_callback.h` |
| IHDSinkStatusListener | Sink 状态监听 | `common/utils/include/` |
| IHDSourceStatusListener | Source 状态监听 | `common/utils/include/` |

回调代理通过 Proxy 类实现，如 `PublisherListenerProxy`。

## 反模式/修改前检查

- **禁止**在 IPC 方法中直接访问 ComponentManager 的内部状态，应通过公共接口调用。
- **禁止**新增 IPC 方法时遗漏 Stub 层实现，否则客户端调用会失败。
- **禁止**在回调中使用同步 IPC 调用，可能导致死锁。
- 修改 IPC 接口时，检查是否需要同步更新 NAPI 层封装。
- 新增回调接口时，必须同时实现 Proxy 和 Stub。
- IPC 调用超时默认由 PendingGetDHRequest 清理机制处理。

## 测试指引

- DistributedHardwareService IPC 方法修改：使用 `DistributedHardwareServiceTest`、`DistributedHardwareStubTest`。
- IPC Proxy 修改：使用 `PublisherListenerProxyTest`、`HardwareStatusListenerProxyTest`。
- ComponentManager 使能方法修改：使用 `ComponentManagerTest`。
- AVTransControlCenter 修改：使用 `AvTransControlCenterTest`。
- Fuzz 测试：使用 `distributedfwkstub_fuzzer`、`distributedfwkservices_fuzzer`。