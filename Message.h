#ifndef MESSAGE_H
#define MESSAGE_H


#include <cstdint>

typedef struct Message {
    uint16_t size;
    uint8_t type;
    uint64_t id;
    uint64_t data;
    Message *next;
} Message;

char *SerializeMessage(const Message &msg);

Message DeserializeMessage(const char *buffer);

#endif //MESSAGE_H
