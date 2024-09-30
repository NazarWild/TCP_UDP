#include "Connection.h"
#include "MessageContainerSingleton.h"
#include <chrono>
#include <iostream>
#include <thread>

void CloseSocket(SOCKET sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

void SetNonBlocking(SOCKET sock) {
#ifdef _WIN32
    unsigned long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

void WSAInitialize() {
#ifdef _WIN32
    WSADATA wsaData;
    auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }
#endif
}

void WSAClean() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void UdpTransmitter(const char *serverIP, int serverPort, const Message &message) {
    WSAInitialize();

    sockaddr_in servaddr{};
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("[UdpTransmitter] Socket creation failed");
        return;
    }

    SetNonBlocking(sockfd);

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(serverIP);
    servaddr.sin_port = htons(serverPort);

    auto msg = SerializeMessage(message);

    const size_t sent_bytes = sendto(sockfd, msg, sizeof(Message), 0,
                                     reinterpret_cast<struct sockaddr *>(&servaddr), sizeof(servaddr));

    if (sent_bytes == -1) {
        printf("[UdpTransmitter] sendto failed");
    } else {
        printf("[UdpTransmitter] Sent %llu bytes\n", sent_bytes);
    }

    CloseSocket(sockfd);
    WSAClean();
}

void UdpReceiver(int port) {
    WSAInitialize();

    sockaddr_in servaddr, cliaddr;
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("[UdpReceiver] socket failed\n");
        return;
    }

    SetNonBlocking(sockfd);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);


    if (bind(sockfd, reinterpret_cast<const sockaddr *>(&servaddr), sizeof(servaddr)) == -1) {
        printf("[UdpReceiver] Bind failed");
        CloseSocket(sockfd);
        return;
    }

    int len, n;
    len = sizeof(cliaddr);

    while (!MessageMapSingleton::GetInstance().must_stop_) {
        char buffer[2048];
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr *>(&cliaddr), &len);
        if (n < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        buffer[n] = '\0';
        MessageMapSingleton::GetInstance().ProcessMessage(buffer);
    }

    CloseSocket(sockfd);
    WSAClean();
}


void TcpTransmitter(const char *serverIP, int serverPort) {
    WSAInitialize();

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("[TcpTransmitter] Socket creation failed with error: %ld\n", WSAGetLastError());
        WSAClean();
        return;
    }

    sockaddr_in servaddr{};
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(serverIP);
    servaddr.sin_port = htons(serverPort);

    SetNonBlocking(sockfd);

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
#ifdef _WIN32
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            printf("[TcpTransmitter] Connection attempt failed immediately with error: %ld\n", WSAGetLastError());
            CloseSocket(sockfd);
            WSAClean();
            return;
        }
#endif

        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sockfd, &writefds);
        timeval timeout = {5, 0};

        int selRes = select(0, NULL, &writefds, NULL, &timeout);
        if (selRes > 0) {
            int so_error;
            socklen_t len = sizeof(so_error);
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *) &so_error, &len);
            if (so_error != 0) {
                printf("[TcpTransmitter] Connection failed with error: %d\n", so_error);
                CloseSocket(sockfd);
                WSAClean();
                return;
            }
        } else {
            printf("[TcpTransmitter] Connection attempt timed out or failed.\n");
            CloseSocket(sockfd);
            WSAClean();
            return;
        }
    }

    printf("[TcpTransmitter] Connection established!\n");

    CustomMessageContainer &container = MessageMapSingleton::GetInstance().GetCustomMessageContainer();
    int i = 0;
    while (!MessageMapSingleton::GetInstance().must_stop_) {
        char *message;
        while ((message = container.getMessage()) != nullptr) {
            size_t sent_bytes = send(sockfd, message, sizeof(Message), 0);
            if (sent_bytes < 0) {
                printf("Send failed with error: %ld\n", WSAGetLastError());
                break;
            }
            printf("[TcpTransmitter] Sent %zu bytes\n", sent_bytes);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    CloseSocket(sockfd);
    WSAClean();
}

void TcpReceiver(int port) {
    WSAInitialize();

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("[TcpReceiver] Socket creation failed");
        WSAClean();
        return;
    }

    sockaddr_in servaddr{};
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (const sockaddr *) &servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        printf("[TcpReceiver] Bind failed");
        CloseSocket(sockfd);
        WSAClean();
        return;
    }

    if (listen(sockfd, 10) == SOCKET_ERROR) {
        printf("[TcpReceiver] Listen failed");
        CloseSocket(sockfd);
        WSAClean();
        return;
    }

    SetNonBlocking(sockfd);

    printf("[TcpReceiver] Server is listening...\n");

    SOCKET connfd;
    sockaddr_in cliaddr{};
    int cliaddrlen = sizeof(cliaddr);

    while (!MessageMapSingleton::GetInstance().must_stop_) {
        connfd = accept(sockfd, (sockaddr *) &cliaddr, &cliaddrlen);
        if (connfd == INVALID_SOCKET) {
#ifdef _WIN32
            if (WSAGetLastError() != WSAEWOULDBLOCK) {
                printf("[TcpReceiver] Accept failed with error: %d\n", WSAGetLastError());
            }
#endif
            continue;
        } else {
            while (!MessageMapSingleton::GetInstance().must_stop_) {
                char buffer[2048];
                int bytes_read;
                if ((bytes_read = recv(connfd, buffer, sizeof(buffer), 0)) > 0) {
                    buffer[bytes_read] = '\0';
                    const Message msg = DeserializeMessage(buffer);
                    printf("[TcpReceiver] Received msg id = %llu, type = %I32u, size = %u, data = %llu, \n",
                           msg.id,
                           msg.type,
                           msg.size, msg.data);
                }
#ifdef _WIN32
                if (bytes_read == 0 || (bytes_read < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
                    printf("[TcpReceiver] Connection closing...\n");
                    CloseSocket(connfd);
                    break;
                }
#else
                if (bytes_read == 0 || bytes_read < 0) {
                    printf("[TcpReceiver] Connection closing...\n");
                    CloseSocket(connfd);
                    break;
                }
#endif
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    CloseSocket(sockfd);
    WSAClean();
}
