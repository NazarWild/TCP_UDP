#ifndef CONNECTION_H
#define CONNECTION_H

#include "Message.h"

#ifdef _WIN32
#include <Winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>

#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
using SOCKET = int;

#endif

void CloseSocket(SOCKET sock);

void SetNonBlocking(SOCKET sock);

void UdpTransmitter(const char *serverIP, int serverPort, const Message &message);

void UdpReceiver(int port);

void TcpTransmitter(const char *serverIP, int serverPort);

void TcpReceiver(int port);

#endif //CONNECTION_H
