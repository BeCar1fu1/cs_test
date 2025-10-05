// 文件名: tcp_closewait_demo.cpp
// 编译: g++ -std=c++17 tcp_closewait_demo.cpp -o tcp_closewait_demo
// 运行: ./tcp_closewait_demo
// -------------------------------------------
// 【演示目的】
// 模拟“服务端未关闭连接”导致大量 CLOSE_WAIT 状态
// -------------------------------------------

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <vector>
#include <signal.h>
using namespace std::chrono_literals;
const int SERVER_PORT = 54321;

// 获取 TCP 状态名称
std::string tcp_state_name(unsigned char s) {
    switch (s) {
        case TCP_ESTABLISHED: return "ESTABLISHED";
        case TCP_SYN_SENT:    return "SYN_SENT";
        case TCP_SYN_RECV:    return "SYN_RECV";
        case TCP_FIN_WAIT1:   return "FIN_WAIT1";
        case TCP_FIN_WAIT2:   return "FIN_WAIT2";
        case TCP_TIME_WAIT:   return "TIME_WAIT";
        case TCP_CLOSE:       return "CLOSE";
        case TCP_CLOSE_WAIT:  return "CLOSE_WAIT";
        case TCP_LAST_ACK:    return "LAST_ACK";
        case TCP_LISTEN:      return "LISTEN";
        case TCP_CLOSING:     return "CLOSING";
        default: return "UNKNOWN";
    }
}

// ------------------------ 服务端 ------------------------
void server_process() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    bind(listen_fd, (sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 100);

    std::cout << "[Server] Listening on 127.0.0.1:" << SERVER_PORT << "\n";

    while (true) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        int conn_fd = accept(listen_fd, (sockaddr*)&client, &len);
        if (conn_fd < 0) continue;

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
        std::cout << "[Server] Accepted from " << ip << ":" << ntohs(client.sin_port) << "\n";

        // 模拟业务处理：不关闭连接，造成CLOSE_WAIT堆积
        std::thread([conn_fd]() {
            char buf[128];
            recv(conn_fd, buf, sizeof(buf), 0);

            // 故意不 close(conn_fd);
            // ---- 注意：这里模拟程序忘记释放资源 ----
            struct tcp_info info{};
            socklen_t len = sizeof(info);
            getsockopt(conn_fd, IPPROTO_TCP, TCP_INFO, &info, &len);
            std::cout << "[Server] socket state=" << tcp_state_name(info.tcpi_state)
                      << " (tcpi_state=" << (int)info.tcpi_state << ")\n";

            std::this_thread::sleep_for(10s); // 观察时间
        }).detach();
    }
}

// ------------------------ 客户端 ------------------------
void client_process() {
    for (int i = 0; i < 5; ++i) {  // 创建5个连接
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serv{};
        serv.sin_family = AF_INET;
        serv.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

        connect(sock, (sockaddr*)&serv, sizeof(serv));
        send(sock, "Hello", 5, 0);

        close(sock); // 客户端立即关闭，触发 FIN
        std::cout << "[Client] Closed connection #" << i << "\n";
        std::this_thread::sleep_for(200ms);
    }
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        server_process();
    } else {
        std::this_thread::sleep_for(200ms);
        client_process();

        std::cout << "[Main] Run 'ss -tan | grep 54321' to observe CLOSE_WAIT sockets.\n";
        std::this_thread::sleep_for(15s); // 保持进程存活，方便观察
        kill(pid, SIGTERM);
    }
    return 0;
}
