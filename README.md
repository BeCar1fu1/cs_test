# CS Notes / 计算机八股与专项练习

本仓库用于整理与练习计算机基础知识与常见面试高频考点，包括：

- 计算机网络  
- 操作系统  
- C++  
- Docker / 容器  
- Redis  
- MySQL  
- LLM / 大模型  
- 进程 & 线程  
- TCP 三次握手 / 四次挥手等网络细节  

目标是通过 **系统化笔记 + 可运行的小 Demo / 测试代码**，帮助快速查漏补缺、准备面试。

---

## 仓库结构

```bash
.
├── CS_review/                     # 计算机基础 & 八股文整理
│   ├── assets/                    # 文中使用的图片资源
│   │   ├── 1741058462929-....png
│   │   ├── 1743039774866-....png
│   │   ├── 1756907442208-....png
│   │   ├── 1756907491609-....png
│   │   └── 6d6d7061bd764362fc43....svg+xml
│   ├── Cpp.md                     # C++ 相关知识点
│   ├── Git.md                     # Git / 版本控制
│   ├── LLM.md                     # 大模型 / LLM 相关概念
│   ├── MySql.md                   # MySQL 基础与高频考点
│   ├── Network_CS.md              # 计算机网络（TCP/IP、HTTP 等）
│   ├── Opreator_System.md         # 操作系统（进程、线程、内存等）
│   ├── Redis.md                   # Redis 基础与常见问题
│   └── docker.md                  # Docker 与容器相关知识
│
├── Process_test/                  # 进程 / 线程相关测试与 Demo
│   ├── README.md                  # 本目录说明
│   ├── process_thread             # 已编译的可执行文件（名称可能随平台变化）
│   └── process_thread.cpp         # 进程 & 线程对比的示例代码 / 测试代码
│
├── tcp_handshake_test/            # TCP 三次握手 / 四次挥手相关测试与 Demo
│   ├── README.md                  # 本目录说明
│   ├── tcp_closewait_test         # CLOSE_WAIT 场景测试可执行文件
│   ├── tcp_closewait_test.cpp     # CLOSE_WAIT 场景示例代码
│   ├── tcp_handshake_demo         # 三次握手 Demo 可执行文件
│   ├── tcp_handshake_demo.cpp     # 三次握手 Demo 代码
│   ├── tcp_timewait_demo          # TIME_WAIT 场景 Demo 可执行文件
│   └── tcp_timewait_demo.cpp      # TIME_WAIT 场景 Demo 代码
│
└── README.md                      # 项目总说明（本文）
