# 分布式硬件框架指引

## 项目定位

本仓库对应 OpenHarmony `foundation/distributedhardware/distributed_hardware_fwk`，为分布式硬件子系统提供统一的硬件接入、查询和使能能力。按以下目录定位问题：

- `services/distributedhardwarefwkservice/`：SA 服务实现，含 AccessManager、ComponentManager、ResourceManager、Task 等。
- `interfaces/inner_kits/`：北向 SDK，供上层应用调用分布式硬件能力。
- `interfaces/kits/napi/`：JS API 层，供应用层调用。
- `av_transport/`：音视频传输引擎，含 ControlCenter、Sender、Receiver、Pipeline。
- `common/utils/`：公共接口定义（DHType、错误码、IDistributedHardware 接口）。
- `utils/`：工具类（日志、HiSysEvent、HiTrace）。

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行：

```sh
./build.sh --product-name rk3568 --build-target distributedhardwarefwksvr --ccache
./build.sh --product-name rk3568 --build-target libdhfwk_sdk --ccache
```

单元测试：
```sh
./build.sh --product-name rk3568 --build-target test --ccache
```

涉及真实设备上下线、分布式硬件使能的行为，需板侧证据。提交使用 `git commit -s`。

## 知识索引

| 场景 | 先读 |
| --- | --- |
| 设备上线/下线流程处理 | `docs/knowledge/dh-device-lifecycle.md` |
| 分布式硬件使能/去使能 | `docs/knowledge/dh-component-enable.md` |
| 任务执行顺序和并发控制 | `docs/knowledge/dh-task-pipeline.md` |
| DHType 类型判断和配置差异 | `docs/knowledge/dh-type-taxonomy.md` |
| IPC 调用链和 SA 交互（含 callingUid/userId 区分） | `docs/knowledge/dh-ipc-service.md` |

## 项目约束

- **设备标识不可混用**：networkId、uuid、udid、deviceId 含义不同，混用会导致数据查询失败。见 `dh-device-lifecycle.md`。
- **使能必须等上线完成**：Enable 前必须确保 SendOnLineEvent 成功，否则 ResourceManager 无数据。见 `dh-component-enable.md`。
- **Task 不可并行执行同 key**：相同 networkId+dhId 的 Enable/Disable 任务会冲突，TaskBoard 会阻塞等待。见 `dh-task-pipeline.md`。
- **自动使能组件无 source_feature_filter**：配置文件中无此字段的组件（如 MODEM）自动使能，有此字段（空字符串）的需主动使能。见 `dh-component-enable.md`。
- **错误码范围固定**：DHFWK 错误码范围 [-10000, -19999]，子模块按范围分配。新增错误码不可越界。见 `common/utils/include/distributed_hardware_errno.h`。