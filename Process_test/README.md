# Process Thread Diff
## 进程与线程演示

本项目演示了 Linux 下进程与线程的区别，以及它们对全局变量的影响。

### 文件说明
- process_thread.cpp：主程序源码
- process_thread：编译后的可执行文件

### 编译方法
```bash
g++ -std=c++17 process_thread.cpp -o process_thread -lpthread
```

### 运行方法
```bash
./process_thread
```

### 功能简介
- 展示主进程、子进程和线程的 PID、TID、父进程 ID。
- 观察全局变量在进程和线程中的共享与隔离：
	- 子进程对 global_var 的修改不会影响父进程（进程不共享内存）。
	- 线程对 global_var 的修改会影响主线程（线程共享内存）。
- 每次主进程循环都会创建一个新线程，演示线程的创建和 join。

### 输出示例
```text
==============================
Main Process Info
==============================
getpid()  = ... (进程ID)
getppid() = ... (父进程ID)
pthread_self() = ...
gettid()       = ... (Linux线程ID)
global_var = 0

---- 子进程 ----
getpid()  = ... (子进程PID)
getppid() = ... (父进程PID)
global_var = 100 (父子进程不共享)
--------------

==== 父进程创建线程 ====
[Thread] Start ---
pthread_self() = ...
gettid()       = ...
getpid()       = ...
global_var     = 10 (共享)
[Thread] End ---
```
### 原理
```
父进程 pid=4123
│
├── 子进程 pid=4124 (通过 fork)
│     └── 独立地址空间
│     └── global_var=100
│
└── 主进程内线程们 (pthread_create)
      ├── tid=4123 (主线程)
      ├── tid=4125 (新线程)
      └── 共享内存、global_var=10
```
```
[系统视角]        [用户态 POSIX 视角]

task_struct (TID)
------------------------------------------
| pid=1001, tid=1001 ← 主线程            |
| pid=1001, tid=1002 ← 工作线程1         |
| pid=1001, tid=1003 ← 工作线程2         |
------------------------------------------

pthread_self() 值（仅在进程内有效）
------------------------------------------
| pthread_self() = 0x7f8a3c000700 (主线程)|
| pthread_self() = 0x7f8a2bfff700 (线程1)|
| pthread_self() = 0x7f8a1afff700 (线程2)|
------------------------------------------
```

### 关键点说明
- `fork()` 创建子进程，子进程拥有独立的内存空间。
- `pthread_create()` 创建线程，线程与主线程共享内存。
- `getpid()` 获取进程 ID，`gettid()` 获取线程 ID。
- 适合学习和理解多进程、多线程编程基础。

<!-- 如需更多说明可补充 -->