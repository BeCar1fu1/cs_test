// 文件名: tcp_timewait_demo.cpp
// 编译: g++ -std=c++17 tcp_timewait_demo.cpp -o tcp_timewait_demo
// 运行: ./tcp_timewait_demo
//
// 观察命令: ss -tan | grep 54321
// 你会看到客户端出现大量 TIME_WAIT

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <signal.h>
using namespace std::chrono_literals;
const int PORT = 54321;

void server() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    bind(listenfd, (sockaddr*)&addr, sizeof(addr));
    listen(listenfd, 100);
    std::cout << "[Server] Listening on 127.0.0.1:" << PORT << "\n";

    while (true) {
        sockaddr_in cli{};
        socklen_t len = sizeof(cli);
        int conn = accept(listenfd, (sockaddr*)&cli, &len);
        if (conn >= 0) {
            std::thread([conn]() {
                char buf[128];
                recv(conn, buf, sizeof(buf), 0);
                std::this_thread::sleep_for(50ms);  // 模拟轻微延迟
                send(conn, "pong", 4, 0);
                // 服务端被动等待客户端关闭，不调用 close() 立刻
                close(conn);
            }).detach();
        }
    }
}

void client() {
    for (int i = 0; i < 20; ++i) { // 连续创建20个短连接
        int sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serv{};
        serv.sin_family = AF_INET;
        serv.sin_port = htons(PORT);
        inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

        connect(sock, (sockaddr*)&serv, sizeof(serv));
        send(sock, "ping", 4, 0);

        char buf[128];
        recv(sock, buf, sizeof(buf), 0);
        close(sock); // ✅ 客户端主动关闭 -> TIME_WAIT
        std::this_thread::sleep_for(100ms);
    }

    std::cout << "[Client] Finished all connections\n";
}

int main() {
    pid_t pid = fork();
    if (pid == 0) server();
    else {
        std::this_thread::sleep_for(300ms);
        client();

        std::cout << "[Main] Run 'ss -tan | grep 54321' to observe TIME_WAIT\n";
        std::this_thread::sleep_for(20s); // 等待一段时间，方便观察 TIME_WAIT
        kill(pid, SIGTERM);
    }
}
