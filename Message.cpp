#include "Message.h"

#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif


char *SerializeMessage(const Message &msg) {
    char *buffer = new char[sizeof(Message)];
    uint16_t size_net = htons(msg.size);
    uint64_t id_net = htonll(msg.id);
    uint64_t data_net = htonll(msg.data);

    memcpy(buffer, &size_net, sizeof(size_net));
    memcpy(buffer + sizeof(size_net), &msg.type, sizeof(msg.type));
    memcpy(buffer + sizeof(size_net) + sizeof(msg.type), &id_net, sizeof(id_net));
    memcpy(buffer + sizeof(size_net) + sizeof(msg.type) + sizeof(id_net), &data_net, sizeof(data_net));

    return buffer;
}

Message DeserializeMessage(const char *buffer) {
    Message msg;
    uint16_t size_net;
    uint64_t id_net, data_net;

    memcpy(&size_net, buffer, sizeof(size_net));
    msg.size = ntohs(size_net);
    memcpy(&msg.type, buffer + sizeof(size_net), sizeof(msg.type));
    memcpy(&id_net, buffer + sizeof(size_net) + sizeof(msg.type), sizeof(id_net));
    msg.id = ntohll(id_net);
    memcpy(&data_net, buffer + sizeof(size_net) + sizeof(msg.type) + sizeof(id_net), sizeof(data_net));
    msg.data = ntohll(data_net);

    return msg;
}

