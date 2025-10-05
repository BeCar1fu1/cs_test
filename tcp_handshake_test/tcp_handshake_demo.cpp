// 文件名: tcp_handshake_demo.cpp
// 编译: g++ -std=c++17 tcp_handshake_demo.cpp -o tcp_handshake_demo
// 运行: ./tcp_handshake_demo

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;

const int SERVER_PORT = 54321;

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

void print_ascii_diagram() {
    std::cout << "三次握手（3-way handshake）ASCII 示意：\n\n";
    std::cout << "Client                          Server\n";
    std::cout << "  | -- SYN(seq=x) ------------> |\n";
    std::cout << "  | <--- SYN+ACK(seq=y,ack=x+1) |\n";
    std::cout << "  | -- ACK(ack=y+1) ---------->  |\n\n";
    std::cout << "connect() 在客户端触发 -> 内核发送 SYN\n";
    std::cout << "accept() 在服务端返回 -> 三次握手完成，连接进入 ESTABLISHED\n\n";
}
//设置文件描述符为非阻塞模式
int set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
//服务端处理函数
void server_process() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("server socket"); return; }

    int on = 1;

    //setsockopt是设置socket选项的系统调用
    //SO_REUSEADDR选项允许重用本地地址和端口，避免"Address already in use"错误
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    // 监听本地回环地址
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //htons函数将主机字节序转换为网络字节序
    addr.sin_port = htons(SERVER_PORT);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(listen_fd); return; }
    if (listen(listen_fd, 10) < 0) { perror("listen"); close(listen_fd); return; }

    std::cout << "[Server] listening on 127.0.0.1:" << SERVER_PORT << "\n";
    // 等待一会儿，让客户端先发起 connect
    std::this_thread::sleep_for(200ms);

    // accept 会在三次握手完成后返回一个已连接的 socket
    struct sockaddr_in peer;
    socklen_t plen = sizeof(peer);
    int conn_fd = accept(listen_fd, (struct sockaddr*)&peer, &plen);
    if (conn_fd < 0) { perror("accept"); close(listen_fd); return; }

    // 打印连接的客户端地址
    char peer_ip[INET_ADDRSTRLEN];
    //inet_ntop函数将网络地址转换为字符串表示
    inet_ntop(AF_INET, &peer.sin_addr, peer_ip, sizeof(peer_ip));
    std::cout << "[Server] accept() returned. peer=" << peer_ip << ":" << ntohs(peer.sin_port) << "\n";

    // 查询 TCP_INFO
    struct tcp_info info;
    socklen_t infolen = sizeof(info);
    if (getsockopt(conn_fd, IPPROTO_TCP, TCP_INFO, &info, &infolen) == 0) {
        std::cout << "[Server] accepted socket tcp_state=" << tcp_state_name(info.tcpi_state)
                  << " (tcpi_state=" << (int)info.tcpi_state << ")\n";
    } else {
        perror("[Server] getsockopt(TCP_INFO)");
    }

    // 读客户端可能发来的数据
    char buf[128];
    ssize_t n = recv(conn_fd, buf, sizeof(buf)-1, 0);
    if (n > 0) {
        buf[n] = 0;
        std::cout << "[Server] recv: " << buf << "\n";
    }

    // 简单回复
    const char* reply = "Hello from server";
    send(conn_fd, reply, strlen(reply), 0);

    close(conn_fd);
    close(listen_fd);
    std::cout << "[Server] closed\n";
}

void client_process() {
    std::cout << "[Client] start\n";
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("client socket"); return; }

    set_nonblock(sock);

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);
    serv.sin_port = htons(SERVER_PORT);

    int ret = connect(sock, (struct sockaddr*)&serv, sizeof(serv));
    if (ret == 0) {
        std::cout << "[Client] connect() returned immediately (rare). Socket may be ESTABLISHED\n";
    } else {
        if (errno == EINPROGRESS) {
            std::cout << "[Client] non-blocking connect in progress -> likely kernel sent SYN (state should be SYN_SENT)\n";
        } else {
            perror("connect");
            close(sock);
            return;
        }
    }

    // 循环轮询 TCP_INFO，观察状态变化
    for (int i = 0; i < 50; ++i) { // 最多观察 50 次 (~5s)
        struct tcp_info info;
        socklen_t len = sizeof(info);
        if (getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, &len) == 0) {
            std::cout << "[Client] poll #" << i << " tcp_state=" << tcp_state_name(info.tcpi_state)
                      << " (tcpi_state=" << (int)info.tcpi_state << ")\n";
            if (info.tcpi_state == TCP_ESTABLISHED) {
                std::cout << "[Client] observed ESTABLISHED -> handshake finished\n";
                break;
            }
        } else {
            perror("[Client] getsockopt(TCP_INFO)");
        }

        // 用 select 等待 socket 可写（意味着 connect 完成或失败）
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100 * 1000; // 100ms
        int s = select(sock + 1, NULL, &wfds, NULL, &tv);
        if (s > 0 && FD_ISSET(sock, &wfds)) {
            // 检查 connect 是否成功
            int err = 0;
            socklen_t elen = sizeof(err);
            if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &elen) == 0) {
                if (err == 0) {
                    std::cout << "[Client] select shows writable and SO_ERROR==0 -> connect succeeded\n";
                } else {
                    std::cout << "[Client] connect failed, SO_ERROR=" << err << " (" << strerror(err) << ")\n";
                    close(sock);
                    return;
                }
            }
        }
        std::this_thread::sleep_for(100ms);
    }

    // 一旦建立，发送数据演示应用层通信
    const char* msg = "Hello from client";
    ssize_t wn = send(sock, msg, strlen(msg), 0);
    if (wn > 0) std::cout << "[Client] sent: " << msg << "\n";

    // 读服务端回复
    char buf[128];
    ssize_t rn = recv(sock, buf, sizeof(buf)-1, 0);
    if (rn > 0) {
        buf[rn] = 0;
        std::cout << "[Client] recv: " << buf << "\n";
    }

    close(sock);
    std::cout << "[Client] closed\n";
}

int main() {
    print_ascii_diagram();

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // child -> server
        server_process();
        return 0;
    } else {
        // parent -> client (给 server 一点时间来准备)
        std::this_thread::sleep_for(100ms);
        client_process();

        // wait child
        int status = 0;
        waitpid(pid, &status, 0);
        std::cout << "[Main] child exited, demo finished\n";
    }
    return 0;
}
