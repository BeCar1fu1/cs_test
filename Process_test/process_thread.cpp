#include <iostream>
#include <thread>
#include <sys/types.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <cstring>
#include <unistd.h>

int global_var = 0; // 全局变量

pid_t gettid_sys(){
    return syscall(SYS_gettid) ; // 获取线程ID
}

void* thread_func(void* arg) {
    global_var += 10;
    std::cout << "\n[Thread] Start ---\n";
    std::cout << "  pthread_self() = " << pthread_self() << "\n";
    std::cout << "  gettid()       = " << gettid_sys() << "\n";
    std::cout << "  getpid()       = " << getpid() << "\n";
    std::cout << "  getppid()      = " << getppid() << "\n";
    std::cout << "  global_var     = " << global_var << " (共享)\n";
    std::cout << "[Thread] End ---\n";
    return nullptr;
}

int main() {
    std::cout << "==============================\n";
    std::cout << "Main Process Info\n";
    std::cout << "==============================\n";
    std::cout << "getpid()  = " << getpid() << " (进程ID)\n";
    std::cout << "getppid() = " << getppid() << " (父进程ID)\n";
    std::cout << "pthread_self() = " << pthread_self() << "\n";
    std::cout << "gettid()       = " << gettid_sys() << " (Linux线程ID)\n";
    std::cout << "global_var = " << global_var << "\n\n";

    // 1️⃣ 创建子进程
    pid_t pid = fork();

    if (pid == 0) {
        // 子进程
        global_var += 100;
        std::cout << "---- 子进程 ----\n";
        std::cout << "getpid()  = " << getpid() << " (子进程PID)\n";
        std::cout << "getppid() = " << getppid() << " (父进程PID)\n";
        std::cout << "pthread_self() = " << pthread_self() << "\n";
        std::cout << "gettid()       = " << gettid_sys() << " (子进程主线程TID)\n";
        std::cout << "global_var = " << global_var << " (父子进程不共享)\n";
        std::cout << "--------------\n";
        _exit(0);
    } else {
        // 父进程
        for(int i = 0; i < 3; ++i) {
        std::cout << "\n[Main] Loop " << i << ": global_var=" << global_var << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "\n==== 父进程创建线程"<<i<<" ====\n";
        pthread_t tid;
        pthread_create(&tid, nullptr, thread_func, nullptr);
        pthread_join(tid, nullptr);

        std::cout << "\n[Main] After thread join:\n";
        std::cout << "  getpid()  = " << getpid() << "\n";
        std::cout << "  gettid()  = " << gettid_sys() << "\n";
        std::cout << "  global_var = " << global_var << " (线程修改影响主进程)\n";
    }
    }
    std::cout << "\n[Main] Done.\n";
    return 0;
}