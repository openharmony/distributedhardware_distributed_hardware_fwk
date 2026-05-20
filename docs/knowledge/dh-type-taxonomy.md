# 分布式硬件类型分类知识

本文只记录 DHType 枚举定义、配置差异和子类型扩展。设备上下线见 `dh-device-lifecycle.md`，使能流程差异见 `dh-component-enable.md`，原有枚举对照见 `reference/02_dependencies.md`。

## DHType 主类型枚举

| DHType | 值 | 名称 | 使能类型 | 配置特点 | 锚点 |
| --- | --- | --- | --- | --- | --- |
| CAMERA | 0x01 | 分布式相机 | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:26` |
| AUDIO | 0x02 | 分布式音频 | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:27` |
| SCREEN | 0x08 | 分布式屏幕 | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:28` |
| GPS | 0x10 | GPS | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:29` |
| INPUT | 0x20 | 分布式输入 | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:30` |
| HFP | 0x40 | HFP 外设 | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:31` |
| A2D | 0x80 | A2DP 外设 | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:32` |
| VIRMODEM_AUDIO | 0x100 | 蜂窝通话音频 | 主动使能 | source_feature_filter 为空字符串 | `device_type.h:33` |
| MODEM | 0x200 | 分布式通信 | 自动使能 | 无 source_feature_filter 字段 | `device_type.h:34` |

**核心差异**：MODEM 无 source_feature_filter 字段，判定为自动使能；其他类型有字段但为空，判定为主动使能。

## DHSubtype 子类型枚举

| DHSubtype | 值 | 所属主类型 | 用途 | 锚点 |
| --- | --- | --- | --- | --- |
| CAMERA | 0x01 | CAMERA | 相机子类型 | `device_type.h:40` |
| SCREEN | 0x08 | SCREEN | 屏幕子类型 | `device_type.h:41` |
| INPUT | 0x20 | INPUT | 输入子类型 | `device_type.h:42` |
| MODEM_MIC | 0x100 | MODEM/AUDIO | MODEM 麦克风 | `device_type.h:43` |
| MODEM_SPEAKER | 0x200 | MODEM/AUDIO | MODEM 扬声器 | `device_type.h:44` |
| AUDIO_MIC | 0x400 | AUDIO | 音频麦克风 | `device_type.h:45` |
| AUDIO_SPEAKER | 0x800 | AUDIO | 音频扬声器 | `device_type.h:46` |

子类型用于区分同一主类型的不同硬件实例，如 AUDIO 的麦克风和扬声器。

## DHType 前缀映射

| DHType | DHId 前缀 | 用途 | 锚点 |
| --- | --- | --- | --- |
| CAMERA | "Camera" | 分布式相机 DHId 前缀 | `device_type.h:85` |
| SCREEN | "Screen" | 分布式屏幕 DHId 前缀 | `device_type.h:86` |
| INPUT | "Input" | 分布式输入 DHId 前缀 | `device_type.h:87` |

DHId 格式为 `前缀_设备标识_实例编号`，如 `Camera_uuid123_0`。

## 配置文件字段差异

| 字段 | 存在性 | 含义 | 自动使能判定 |
| --- | --- | --- | --- |
| source_feature_filter | 不存在 | 无过滤条件 | 自动使能（如 MODEM） |
| source_feature_filter | 存在且为空字符串 "" | 需主动使能 | 主动使能（如 CAMERA） |
| source_feature_filter | 存在且有值 | 特定功能过滤 | 按功能匹配 |

配置文件路径：`etc/distributedhardware/distributed_hardware_components_cfg.json`（见 `constants.h:57`）。

## 反模式/修改前检查

- **禁止**用 DHType 值直接比较判断类型，应使用枚举名（如 `DHType::CAMERA`）。
- **禁止**新增 DHType 时遗漏 DHTypeStrMap 映射，否则日志输出会失败。
- **禁止**混淆 DHType 和 DHSubtype，子类型只在特定场景使用。
- 新增 DHType 时，检查是否需要在 DHTypePrefixMap 中添加前缀映射。
- 新增 DHType 时，必须在配置文件中明确 source_feature_filter 字段设置。
- 修改 DHType 枚举值时，检查是否影响数据库存储和远端同步。

## 测试指引

- DHType 枚举修改：检查 `device_type.h` 编译通过，无测试类。
- ComponentLoader 配置解析修改：使用 `ComponentLoaderTest`。
- 使能类型判定逻辑修改：使用 `ComponentManagerTest`，验证自动使能/主动使能流程。
- DHSubtype 使用逻辑修改：使用 `ComponentManagerTest` 中的 GetDHIdByDHSubtype 测试。