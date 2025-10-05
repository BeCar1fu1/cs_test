# TCP Demo

## TCP Handshake Demo
本项目演示了TCP三次握手过程的原理和实现
### 文件说明
- tcp_handshake_demo.cpp：主程序源码
- tcp_handshake_demo：编译后的可执行文件
### 编译方法
 ```bash
 g++ -std=c++17 tcp_handshake_demo.cpp -o tcp_handshake_demo
 ```
## TCP CLOSEWAIT Demo
### 文件说明
- tcp_closewait_test.cpp: 主程序源代码
- tcp_closewait_test: 编译后的可执行文件

###编译方法
```bash
g++ -std=c++17 tcp_closewait_test.cpp -o tcp_closewait_test
```
会出现大量的close_wait状态的连接，主要是因为主动关闭方已经调用了close发送了FIN给服务端，但是服务端没有及时的调用close来关闭连接。这种情况如何处理呢？

主动方（Active closer）                    被动方（Passive closer）
---------------------------------------------------------------
close() -> 发 FIN  -------------------->  收到 FIN → 进入 CLOSE_WAIT
                                           |
                                           | 应用层应当调用 close()
                                           V
                                       发 FIN → LAST_ACK
<-------------------- ACK 回应 FIN       |
FIN_WAIT1/2 → TIME_WAIT <--------------- ACK (完成关闭)

| 检测方式                     | 说明                                      |
|-----------------------------|--------------------------------------------|
| recv()返回 0                | 最标准的检测手段                           |
| epoll 的 EPOLLHUP/EPOLLRDHUP| Linux 常用做法，事件触发时表示对方关闭      |
| getsockopt(TCP_INFO)        | 轮询检测 tcp_info.tcpi_state 是否是 CLOSE_WAIT |
| SO_KEEPALIVE                | 系统级检测死连接（长期没有数据传输）        |
> 注：EPOLL Read Hang Up  检测到对方关闭了写端（即 TCP FIN 收到）
    如果你在服务端用 epoll 管理 socket；
    当对方（客户端）调用 close() 或 shutdown(SHUT_WR)；
    内核会在该 socket 上触发 EPOLLRDHUP 事件；
    表示“对端不会再发数据了”。
## TCP TIMEWAIT Demo
### 文件说明
- tcp_timewait_demo.cpp：主程序源码
- tcp_timewait_demo：编译后的可执行文件

### 编译方法
```bash
g++ -std=c++17 tcp_timewait_demo.cpp -o tcp_timewait_demo
```

### 现象说明
本程序演示了 TCP 协议中 TIME_WAIT 状态的产生。
客户端会连续创建多个短连接并主动关闭，每次关闭后客户端会进入 TIME_WAIT 状态。
可通过如下命令观察端口上的 TIME_WAIT 连接：
```bash
ss -tan | grep 54321
```

### 原理简述
TIME_WAIT 是 TCP 连接主动关闭方在完成四次挥手后进入的状态，目的是确保最后一个 ACK 能被对方收到，防止旧数据包影响后续连接。
TIME_WAIT的存在原因：
1.要保证最后一个ACK能够被对方接收到
2.防止就连接的延迟包影响新的连接
Linux 会让 TIME_WAIT 状态保持约 60 秒（默认 tcp_fin_timeout=60）
‵‵‵
        主动关闭方(Client)                   被动关闭方(Server)
        -------------------------------------------------------------
        1. Client 发送 FIN ------------------> Server 收到 FIN，进入 CLOSE_WAIT
        2. Server 发送 ACK <------------------
        3. Server 发送 FIN <------------------ 被动关闭方调用 close()
        4. Client 回复 ACK ------------------> (Client 进入 TIME_WAIT)
‵‵‵

| 方法                           | 原理                              | 说明                                                         |
|--------------------------------|-----------------------------------|--------------------------------------------------------------|
| **1. 复用短连接（KeepAlive）** | 避免频繁创建和关闭连接            | HTTP/1.1 默认开启，减少 TIME_WAIT 数量                         |
| **2. 启用端口复用**            | 允许 TIME_WAIT 端口快速重用        | `setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))`|
| **3. 系统参数调优**            | 缩短 TIME_WAIT 超时时间或端口复用   | 可通过内核参数调整，见下方配置                                 |
| **4. 服务端被动关闭**          | 把主动关闭交给客户端              | 服务端不主动关闭，减少服务端 TIME_WAIT 数量                    |

