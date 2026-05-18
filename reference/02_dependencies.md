# 2. 模块依赖与外部接口

## 2.1 DHType枚举定义

### 代码位置

```yaml
文件: common/utils/include/device_type.h
枚举: enum class DHType : uint32_t
```

### DHType对照表

```yaml
DHType枚举:
  UNKNOWN: 0x0
  CAMERA: 0x01          # 分布式相机
  AUDIO: 0x02           # 分布式音频
  SCREEN: 0x08          # 分布式屏幕
  GPS: 0x10             # GPS
  INPUT: 0x20          # 分布式输入
  HFP: 0x40            # HFP External device
  A2D: 0x80            # A2DP External device
  VIRMODEM_AUDIO: 0x100  # Cellular call AUDIO
  MODEM: 0X200          # 分布式通信(分布式modem)
  MAX_DH: 0x80000000
```

## 2.2 分布式通信(分布式modem)

### 定义

```yaml
名称: 分布式通信
别名: [分布式modem, MODEM, DistributedCommunication]
DHType值: MODEM = 0X200
定位: 周边模块依赖
```

### 配置文件

```yaml
配置文件: distributed_hardware_components_cfg.json
特点:
  分布式通信(DHType=0x200): 无source_feature_filter字段
  分布式音频(DHType=0x02): source_feature_filter为空字符串
  分布式相机(DHType=0x01): source_feature_filter为空字符串
```

## 2.3 数据库存储

### 数据库类型

```yaml
数据库类型: DistributedKv (分布式键值数据库)
具体实现: SingleKvStore
命名空间: DistributedKv
头文件: distributed_kv_data_manager.h, kvstore_observer.h
```

### meta_info数据库

```yaml
StoreId: global_meta_info
用途: 存储分布式硬件基本信息
管理器: MetaInfoManager
存储内容:
  - 分布式相机基本信息
  - 分布式音频基本信息
  - 分布式通信基本信息
数据来源: 从对端数据库同步
同步触发: 用于分布式通信自动使能流程
```

### 数据同步触发

```yaml
分布式相机数据同步:
  触发方式: OnDataSyncTrigger
  触发时机: 使能成功后
  触发方: 分布式相机主动调用
  目的: 主动触发数据同步到meta_info数据库

分布式通信数据同步:
  触发方式: 自动(通过KvStore同步)
  触发时机: 设备上线/数据变更
  触发方: 数据库自动同步机制
```

### 数据适配器

```yaml
类名: DBAdapter
文件: services/distributedhardwarefwkservice/include/resourcemanager/db_adapter.h
功能:
  - 初始化KV存储(自动同步/本地存储)
  - 数据增删改查
  - 数据同步(SyncDataByNetworkId)
  - 变更监听(KvStoreObserver)
  - 设备数据移除(RemoveDeviceData)
```

## 2.4 使能流程区分

### 自动使能 vs 主动使能

```yaml
自动使能:
  条件: 配置文件中无source_feature_filter字段
  示例: 分布式通信(分布式modem)
  流程: 自动走使能流程

主动使能:
  条件: 配置文件中source_feature_filter字段为空字符串
  示例: [分布式音频, 分布式相机]
  流程: 只能走主动使能
```

### 判断逻辑

```yaml
判断规则:
  检查配置: distributed_hardware_components_cfg.json
  判断字段: source_feature_filter
  逻辑:
    - 字段不存在: 自动使能流程
    - 字段存在且为空字符串: 主动使能流程
```