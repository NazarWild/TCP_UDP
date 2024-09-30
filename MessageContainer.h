#ifndef MESSAGECONTAINER_H
#define MESSAGECONTAINER_H

#include "Message.h"

#define TABLE_SIZE 100

class MessageContainer {
public:
    MessageContainer();

    ~MessageContainer();

    void insert(uint16_t size, uint8_t type, uint64_t id, uint64_t data);

    Message *find(uint64_t id);

private:
    Message *create_message(uint16_t size, uint8_t type, uint64_t id, uint64_t data);

    unsigned int hash(uint64_t MessageId);


    Message **table;
};

#endif //MESSAGECONTAINER_H
