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

void UdpTransmitter(const char *serverIP, int serverPort, const Message &message) {
    sockaddr_in servaddr{};

    WSADATA wsaData;
    auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("[UdpTransmitter]29 Socket creation failed");
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
        perror("[UdpTransmitter]46 sendto failed");
    } else {
        printf("[UdpTransmitter]48 Sent %llu bytes\n", sent_bytes);
    }

    CloseSocket(sockfd);
    WSACleanup();
}

void UdpReceiver(int port) {
    sockaddr_in servaddr, cliaddr;

    WSADATA wsaData;
    auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        printf("[UdpReceiver]60 socket failed with error: %ld\n", WSAGetLastError());
        return;
    }

    SetNonBlocking(sockfd);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);


    if (bind(sockfd, reinterpret_cast<const sockaddr *>(&servaddr), sizeof(servaddr)) == -1) {
        perror("[UdpReceiver]69 Bind failed");
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
        MessageMapSingleton::GetInstance().ProcessMessage(buffer, n);
    }

    CloseSocket(sockfd);
    WSACleanup();
}


void TcpTransmitter(const char *serverIP, int serverPort) {
    sockaddr_in servaddr{};

    WSADATA wsaData;
    auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == -1) {
        printf("[TcpTransmitter]132 socket failed with error: %ld\n", WSAGetLastError());
        return;
    }

    SetNonBlocking(sockfd);

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(serverIP);
    servaddr.sin_port = htons(serverPort);

    if (connect(sockfd, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr)) < 0) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            fd_set writefds;
            FD_ZERO(&writefds);
            FD_SET(sockfd, &writefds);

            // Set timeout as needed
            timeval timeout;
            timeout.tv_sec = 5; // 5 seconds timeout
            timeout.tv_usec = 0;

            int result = select(1, NULL, &writefds, NULL, &timeout);
            if (result > 0) {
                // Socket is writable now, check if connection was successful
                int so_error;
                socklen_t len = sizeof(so_error);

                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *) &so_error, &len);

                if (so_error == 0) {
                    printf("Connection established!\n");
                } else {
                    printf("[TcpTransmitter]109 Connection failed with error: %d\n", so_error);
                    return;
                }
            } else if (result == 0) {
                // Timeout handling
                printf("[TcpTransmitter]109 Connection attempt timed out...\n");
                return;
            } else {
                // select failed
                printf("[TcpTransmitter]109 select call failed with error: %ld\n", WSAGetLastError());
                return;
            }
        } else {
            // Handle other errors
            printf("[TcpTransmitter]109 Connection with the server failed...%ld\n", WSAGetLastError());
            return;
        }
    }

    while (!MessageMapSingleton::GetInstance().must_stop_) {
        char *message = MessageMapSingleton::GetInstance().GetCustomMessageContainer().getMessage();
        if (message != nullptr) {
            size_t sent_bytes = send(sockfd, message, sizeof(Message), 0);
            if (sent_bytes == -1) {
                perror("[TcpTransmitter]187 sendto failed");
            } else {
                printf("[TcpTransmitter]189 Sent %llu bytes\n", sent_bytes);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    CloseSocket(sockfd);
    WSACleanup();
}

void TcpReceiver(int port) {
    SOCKET connfd;
    sockaddr_in servaddr{}, cliaddr{};

    WSADATA wsaData;
    auto iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("[TcpReceiver]134 Socket creation failed");
        return;
    }

    SetNonBlocking(sockfd);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(port);

    if (bind(sockfd, reinterpret_cast<const sockaddr *>(&servaddr), sizeof(servaddr)) == -1) {
        perror("[TcpReceiver]148 Bind failed");
        CloseSocket(sockfd);
        return;
    }

    if (listen(sockfd, 10) == -1) {
        perror("[TcpReceiver]154 Listen failed");
        CloseSocket(sockfd);
        return;
    }

    socklen_t len = sizeof(cliaddr);

    while (!MessageMapSingleton::GetInstance().must_stop_) {
        connfd = accept(sockfd, reinterpret_cast<sockaddr *>(&cliaddr), &len);
        if (connfd < 0) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        SetNonBlocking(connfd);

        char buffer[2048];
        int n;
        while ((n = recv(connfd, buffer, sizeof(buffer), 0)) > 0) {
            // TODO process message with data == 10 and save somewhere

            buffer[n] = '\0'; // Null-terminate the string
            const Message msg = DeserializeMessage(buffer);

            printf("TcpReceiver Received msg id = %li, type = %llu, size = %llu, data = %li, \n", msg.id, msg.type,
                   msg.size,
                   msg.data);
        }

        if (n < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
    }

    CloseSocket(sockfd);
    WSACleanup();
}
