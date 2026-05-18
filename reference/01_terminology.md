# 1. 基础概念与术语

## 1.1 Uid与系统账号区分

### callingUid

```yaml
定义:
  名称: 调用方Uid
  别名: [callingUid, 调用方应用身份]
  获取方式: GetCallingUid()
  含义: 标识调用方应用/进程的身份
  用途:
    - 区分是哪个应用发起的调用
    - 权限校验
  注意: 不是用户ID/系统账号
```

### 系统账号 (OsAccount/用户ID)

```yaml
定义:
  名称: 系统账号
  别名: [用户ID, userId, 用户, 用户空间, 用户空间id, OsAccount]
  标识符:
    key: [localId, userId]
    type: int32
    示例: 100
  作用域: 本地设备
  用途: [数据隔离, 权限控制, 进程管理, 应用安装, 前台用户ID]
  特点: 不同设备的相同userId代表不同的用户空间
```

### 查询接口

```yaml
系统账号查询:
  命名空间: AccountSA::OsAccountManager
  接口:
    GetForegroundOsAccountLocalId:
      返回: int32_t
      用途: 获取前台用户ID
    QueryActiveOsAccountIds:
      返回: std::vector<int32_t>
      用途: 获取活跃用户ID列表
    QueryOsAccountById:
      参数: userId
      返回: OsAccountInfo
      用途: 查询指定用户信息
```

### 区别对照

```yaml
区别:
  callingUid:
    获取方式: GetCallingUid()
    含义: 调用方应用身份
    层级: 应用/进程级别
  
  系统账号(用户ID):
    获取方式: AccountSA::OsAccountManager::GetForegroundOsAccountLocalId()
    含义: 系统用户身份
    层级: 用户级别

结论: 两者是不同层级的概念，不应混淆
```