# 分布式硬件框架指引

## 项目定位

本仓库对应 OpenHarmony `foundation/distributedhardware/distributed_hardware_fwk`，为分布式硬件子系统提供统一的硬件接入、查询和使能能力。按以下目录定位问题：

- `services/distributedhardwarefwkservice/`：SA 服务实现，含 AccessManager、ComponentManager、ResourceManager、Task 等。
- `interfaces/inner_kits/`：北向 SDK，供上层应用调用分布式硬件能力。
- `interfaces/kits/napi/`：JS API 层，供应用层调用。
- `av_transport/`：音视频传输引擎，含 ControlCenter、Sender、Receiver、Pipeline。
- `common/utils/`：公共接口定义（DHType、错误码、IDistributedHardware 接口）。
- `utils/`：工具类（日志、HiSysEvent、HiTrace）。

## 常见任务定位

| 任务 | 起始文件 |
| --- | --- |
| 设备上线/下线流程 | `services/distributedhardwarefwkservice/include/accessmanager/access_manager.h` |
| 组件使能/去使能 | `services/distributedhardwarefwkservice/include/componentmanager/component_manager.h` |
| 任务调度/并发 | `services/distributedhardwarefwkservice/include/task/task_board.h` |
| SA 服务入口/IPC | `services/distributedhardwarefwkservice/include/distributed_hardware_service.h` |
| 音视频传输管线 | `av_transport/av_trans_control_center/services/include/av_trans_control_center.h` |
| 错误码 | `common/utils/include/distributed_hardware_errno.h` |

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行：

```sh
./build.sh --product-name rk3568 --build-target distributedhardwarefwksvr --ccache
./build.sh --product-name rk3568 --build-target libdhfwk_sdk --ccache
```

单测目标（按改动范围选择）：

```sh
./build.sh --product-name rk3568 --build-target AccessManagerTest --ccache
./build.sh --product-name rk3568 --build-target ComponentManagerTest --ccache
./build.sh --product-name rk3568 --build-target OnlineTaskTest --ccache
./build.sh --product-name rk3568 --build-target DHEnableTaskTest --ccache
./build.sh --product-name rk3568 --build-target DistributedHardwareServiceTest --ccache
./build.sh --product-name rk3568 --build-target VersionManagerTest --ccache
```

跑前执行 `git diff --check`。涉及真实设备上下线、分布式硬件使能的行为，需板侧证据。提交使用 `git commit -s`。

**Done 定义**：构建通过 + 相关单测通过 + `git diff --check` 无报错。设备侧用例无法本地执行时，在回复中明确列出"未验证的设备侧项"再交付。

## 知识索引

| 场景 | 先读 |
| --- | --- |
| 设备上线/下线流程处理 | `docs/knowledge/dh-device-lifecycle.md` |
| 分布式硬件使能/去使能 | `docs/knowledge/dh-component-enable.md` |
| 任务执行顺序和并发控制 | `docs/knowledge/dh-task-pipeline.md` |
| DHType 类型判断和配置差异 | `docs/knowledge/dh-type-taxonomy.md` |
| IPC 调用链和 SA 交互（含 callingUid/userId 区分） | `docs/knowledge/dh-ipc-service.md` |

### 路径触发

- 改 `interfaces/inner_kits/` 或 `interfaces/kits/napi/` 前，读 `docs/knowledge/dh-ipc-service.md` 并保留 SDK 签名与错误码。
- 改 `av_transport/` 前，读 `docs/knowledge/dh-task-pipeline.md`。
- 改 `services/distributedhardwarefwkservice/` 下任务/调度前，读 `docs/knowledge/dh-task-pipeline.md`。
- 改 `common/utils/` 下 DHType 或错误码前，读 `docs/knowledge/dh-type-taxonomy.md`。

### 术语触发

| 术语 | 先读 |
| --- | --- |
| `networkId` / `uuid` / `udid` / `deviceId` | `docs/knowledge/dh-device-lifecycle.md` |
| `SendOnLineEvent` / `OnLine` / `OffLine` | `docs/knowledge/dh-device-lifecycle.md` |
| `Enable` / `Disable` / `TaskBoard` / 同 key 冲突 | `docs/knowledge/dh-task-pipeline.md` |
| `DHType` / `source_feature_filter` | `docs/knowledge/dh-type-taxonomy.md` |
| `callingUid` / `userId` / SA 鉴权 | `docs/knowledge/dh-ipc-service.md` |

编辑前用一句话说明：任务类别、已读文档、相关约束。

## 项目约束

- **设备标识不可混用**：networkId、uuid、udid、deviceId 含义不同，混用会导致数据查询失败。见 `dh-device-lifecycle.md`。
- **使能必须等上线完成**：Enable 前必须确保 SendOnLineEvent 成功，否则 ResourceManager 无数据。见 `dh-component-enable.md`。
- **Task 不可并行执行同 key**：相同 networkId+dhId 的 Enable/Disable 任务会冲突，TaskBoard 会阻塞等待。见 `dh-task-pipeline.md`。
- **自动使能组件无 source_feature_filter**：配置文件中无此字段的组件（如 MODEM）自动使能，有此字段（空字符串）的需主动使能。见 `dh-component-enable.md`。
- **错误码范围固定**：DHFWK 错误码范围 [-10000, -19999]，子模块按范围分配。新增错误码不可越界。见 `common/utils/include/distributed_hardware_errno.h`。

### 必须先确认（Ask before）

- 改动 `interfaces/inner_kits/` 北向 SDK 或 `interfaces/kits/napi/` JS API 的签名、错误码、ABI——需兼容性评审，禁止静默破坏。
- 新增/重编号 DHType，或改 `common/utils/include/distributed_hardware_errno.h` 错误码范围 [-10000, -19999]。
- 改动 IPC 中 `callingUid`/`userId` 鉴权、`sa_profile` 权限或 caller-identity 校验。
- 改动跨设备协议字段、`SendOnLineEvent`/Enable 顺序、序列化格式。
- 改 `DHType` 配置中 `source_feature_filter` 字段语义。

## 相关文档

- [分布式音频](https://gitcode.com/openharmony/distributedhardware_distributed_audio)
- [设备管理](https://gitcode.com/openharmony/distributedhardware_device_manager)
- [分布式相机](https://gitcode.com/openharmony/distributedhardware_distributed_camera)
- [分布式屏幕](https://gitcode.com/openharmony/distributedhardware_distributed_screen)
- [分布式输入](https://gitcode.com/openharmony/distributedhardware_distributed_input)
