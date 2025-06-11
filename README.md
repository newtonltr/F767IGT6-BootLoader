# MAG VCU Bootloader

基于STM32F767微控制器的智能网络引导加载程序，支持TCP文件传输和在线应用程序更新(IAP)。

## 项目概述

MAG VCU Bootloader是一个完整的嵌入式引导加载程序解决方案，专为STM32F767平台设计。项目集成了ThreadX实时操作系统和NetXDuo网络栈，提供可靠的网络文件传输和在应用程序升级功能。

### 核心特性

- **硬件平台**: STM32F767IGTx (Cortex-M7, 216MHz)
- **实时操作系统**: ThreadX RTOS
- **网络协议栈**: NetXDuo TCP/IP协议栈
- **网络功能**: TCP服务器，支持以太网连接
- **IAP功能**: 在线应用程序升级，支持bin文件格式
- **安全机制**: CRC16数据校验，确保传输可靠性
- **文件系统**: 轻量级文件系统，支持固件缓存
- **开发工具**: 配套Python客户端工具，简化使用流程

### 主要功能

1. **网络引导**: 通过TCP连接接收应用程序文件
2. **文件验证**: CRC16校验确保数据完整性
3. **固件升级**: 自动将新固件写入Flash并跳转执行
4. **错误处理**: 完善的重连和重试机制
5. **进度反馈**: 实时显示传输和烧录进度

## 快速开始

### 硬件要求

- STM32F767IGTx微控制器开发板
- 以太网接口
- JTAG/SWD调试接口
- 至少512KB Flash和512KB RAM

### 软件要求

- **构建环境**: CMake 3.22+, ARM GCC工具链
- **开发环境**: STM32CubeMX, MDK-ARM或VSCode
- **客户端工具**: Python 3.6+

### 构建步骤

1. **克隆仓库**:
```bash
git clone <repository-url>
cd mag_bootloader
```

2. **配置构建**:
```bash
mkdir build
cd build
cmake ..
```

3. **编译项目**:
```bash
make -j$(nproc)
```

4. **烧录固件**:
```bash
# 使用OpenOCD或STM32CubeProgrammer烧录生成的.elf文件
```

### 使用流程

1. **启动Bootloader**: 上电后，设备自动进入Bootloader模式
2. **网络连接**: 设备通过以太网连接到网络（默认监听端口7000）
3. **运行客户端**: 使用Python工具传输应用程序文件
```bash
cd tools/file_transfer
python file_transfer_client.py
```
4. **输入参数**: 按提示输入服务器IP和文件路径
5. **自动升级**: 系统自动完成文件传输、验证和升级

## 项目结构

```
mag_bootloader/
├── Core/                   # STM32 HAL核心代码
│   ├── Inc/               # 头文件
│   └── Src/               # 源文件
├── Drivers/               # STM32 HAL驱动程序
├── Middlewares/           # 中间件组件
├── ThirdPartys/          # 第三方库
│   ├── ThreadX/          # ThreadX RTOS
│   └── NetXDuo/          # NetXDuo网络协议栈
├── Threads/              # 应用线程实现
│   ├── inc/              # 线程头文件
│   └── src/              # 线程源代码
├── Bsp/                  # 板级支持包
│   ├── inc/              # BSP头文件
│   └── src/              # BSP源代码
├── tools/                # 开发工具
│   └── file_transfer/    # Python文件传输客户端
├── MDK-ARM/              # Keil MDK工程文件
├── cmake/                # CMake构建配置
├── CMakeLists.txt        # 主构建配置
├── mag_vcu.ioc          # STM32CubeMX配置文件
└── README.md            # 项目说明文档
```

### 核心组件

- **thread_socket.c**: TCP服务器实现，处理文件传输协议
- **thread_init.c**: 系统初始化和应用程序跳转
- **emb_flash.c**: Flash编程操作
- **lite_file_sys.c**: 轻量级文件系统
- **crc_tool.c**: CRC16校验算法

## 工具说明

### Python文件传输客户端

位于 `tools/file_transfer/` 目录的Python客户端工具，提供用户友好的文件传输界面。

**主要特性**:
- 自动网络连接管理
- 智能重连和重试机制
- 实时传输进度显示
- 完整的错误处理和状态反馈
- 默认配置简化使用流程

**快速使用**:
```bash
cd tools/file_transfer
python file_transfer_client.py
```

详细使用说明请参考：[文件传输客户端文档](tools/file_transfer/README.md)

## 网络协议

### 文件传输协议

使用自定义的TCP协议进行文件传输，数据包格式如下：

```c
struct __attribute__((packed)) file_transfer_protocol_t {
    uint32_t pack_index;    // 数据包序号 (1-based)
    uint32_t total_bytes;   // 文件总大小
    uint32_t total_packs;   // 总包数
    uint32_t data_size;     // 当前包数据大小
    uint8_t data[512];      // 数据负载 (512字节)
    uint16_t crc16;         // CRC16校验码
};
```

**协议特点**:
- 包大小: 530字节 (包含530字节数据+18字节头部+2字节CRC)
- 数据负载: 512字节/包
- 校验算法: CRC16 (与Modbus相同算法)
- 最大文件: 200KB
- 默认端口: 7000

### 服务器响应消息

- `"client connected \r\n"` - 客户端连接确认
- `"recv success \r\n"` - 数据包接收成功
- `"crc error \r\n"` - CRC校验失败
- `"recv incomplete \r\n"` - 数据包大小错误
- `"start program \r\n"` - 开始IAP烧录流程
- `"program complete \r\n"` - IAP流程完成

## 技术规格

### 硬件配置

- **微控制器**: STM32F767IGTx
- **CPU内核**: ARM Cortex-M7, 216MHz
- **Flash存储**: 1MB (Bootloader区域 + APP区域)
- **RAM容量**: 512KB
- **以太网**: 10/100M以太网接口
- **外设**: ADC、CAN、UART、DMA等

### 软件架构

- **操作系统**: ThreadX RTOS v6.x
- **网络协议栈**: NetXDuo v6.x  
- **文件系统**: 自研轻量级文件系统
- **加密算法**: CRC16校验
- **编程语言**: C11标准
- **构建工具**: CMake 3.22+, GCC ARM

### 性能指标

- **启动时间**: < 3秒
- **传输速度**: 约50-100 KB/s (取决于网络条件)
- **文件大小限制**: 最大200KB
- **内存占用**: 约150KB Flash, 64KB RAM
- **网络超时**: 10秒连接超时，支持自动重连

## 开发指南

### 环境配置

1. **安装依赖**:
```bash
# Ubuntu/Debian
sudo apt-get install cmake gcc-arm-none-eabi build-essential

# Windows (使用STM32CubeIDE或Keil MDK)
# 下载并安装STM32CubeIDE或ARM Keil MDK
```

2. **配置调试器**:
```bash
# 安装OpenOCD (Linux)
sudo apt-get install openocd

# 或使用ST-Link Utility (Windows)
```

### 编译选项

- **Debug版本**: 包含调试信息和断言
- **Release版本**: 优化代码大小和性能
- **自定义配置**: 通过CMake参数调整功能

### 调试说明

- 使用JTAG/SWD接口进行在线调试
- 串口输出调试信息（波特率115200）
- 支持RTT实时数据传输
- 网络抓包分析传输协议

### 定制化开发

可通过修改以下配置适应不同需求：

- **网络参数**: 修改`thread_socket.c`中的端口和协议
- **存储配置**: 调整Flash分区和文件系统大小
- **硬件适配**: 修改BSP层适配不同硬件平台
- **协议扩展**: 扩展文件传输协议支持更多功能

## 故障排除

### 常见问题

1. **连接失败**:
   - 检查网络连接和IP配置
   - 确认防火墙设置
   - 验证端口7000是否被占用

2. **传输中断**:
   - 检查网络稳定性
   - 确认文件大小未超过200KB限制
   - 查看串口调试输出

3. **烧录失败**:
   - 确认Flash区域未被保护
   - 检查应用程序地址配置
   - 验证bin文件格式正确

### 日志分析

系统通过串口输出详细的运行日志，包括：
- 网络连接状态
- 文件传输进度
- Flash操作结果
- 错误代码和诊断信息

## 版本信息

- **当前版本**: v1.0
- **兼容性**: STM32F7系列微控制器
- **更新日志**: 见CHANGELOG.md

## 许可证

本项目采用[MIT许可证](LICENSE)，允许自由使用、修改和分发。

## 贡献指南

欢迎提交问题报告、功能请求和代码贡献：

1. Fork本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送分支 (`git push origin feature/AmazingFeature`)
5. 创建Pull Request

## 技术支持

如有技术问题或建议，请通过以下方式联系：

- 提交Issue: [GitHub Issues](../../issues)
- 邮件联系: [newton144578@gmail.com]
- 文档反馈: 欢迎改进文档内容

---

**项目状态**: 🚀 生产就绪  
**维护状态**: ✅ 积极维护  
**最后更新**: 2025/6/11