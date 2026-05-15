# Distributed Hardware Framework Kit Demo

## 简介

本演示程序展示了如何使用分布式硬件框架（Distributed Hardware Framework Kit）提供的接口，实现硬件设备访问授权功能。

## 功能说明

### 主要功能

1. **注册硬件访问监听器**
   - 注册相机设备的访问监听回调
   - 当设备访问授权结果返回时，自动处理授权结果

2. **注销硬件访问监听器**
   - 注销已注册的相机设备访问监听回调

### 交互命令

程序启动后会显示交互菜单，支持以下命令：

- `0` - 退出程序
- `1` - 注册硬件访问监听器
- `2` - 注销硬件访问监听器

### 权限要求

程序需要以下系统权限：
- `ohos.permission.DISTRIBUTED_SOFTBUS_CENTER`
- `ohos.permission.DISTRIBUTED_DATASYNC`
- `ohos.permission.ACCESS_SERVICE_DM`

程序启动时会自动设置这些权限。

## 使用方法

### 编译

```bash
# 编译命令（根据实际构建系统调整），编译目标为：
 --build-target distributed_hardware_fwk_kit_demo
```

### 编译产物

编译产物与代码编译产物在同一目录下。

### 运行

```bash
# 执行演示程序
./distributed_hardware_fwk_kit_demo
```

### 交互示例

```
=============== InteractiveRunTestSelect ================
You can respond to instructions for corresponding option:
enter 0 to exit.
enter 1 to register
enter 2 to unregister
>>
```

输入对应数字即可执行相应操作。

## 代码结构

- **AuthorizationResultCallbackTest**: 授权结果回调实现类
- **GetUserInput()**: 获取用户输入
- **Register()**: 注册硬件访问监听器
- **Unregister()**: 注销硬件访问监听器
- **HandleUserInputEvent()**: 处理用户输入事件
- **PrintInteractiveUsage()**: 打印交互菜单
- **SetDMAccessPermission()**: 设置分布式硬件访问权限

## 注意事项

1. 本程序需要在支持分布式硬件的 OpenHarmony 系统上运行
2. 程序以 `system_core` 权限级别运行
3. 默认演示设备类型为相机（DHType::CAMERA）
4. 程序最多循环 10 次交互，输入 0 可提前退出
5. OnAuthorizationResult函数的SetAuthorizationResult中变量设置true为授予权限，不授予权限时设置false

## 相关模块

- `distributed_hardware_fwk_kit`: 分布式硬件框架工具包
- `authorization_result_callback_stub`: 授权结果回调桩实现
