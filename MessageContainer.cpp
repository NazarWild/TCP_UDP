#include "MessageContainer.h"
#include <cstdlib>
#include <cstdio>

MessageContainer::MessageContainer() {
    table = (Message **) malloc(sizeof(Message *) * TABLE_SIZE);
    if (!table) {
        printf("Failed to allocate memory");
        exit(EXIT_FAILURE);
    } else {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = NULL;
        }
    }
}

MessageContainer::~MessageContainer() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Message *temp = table[i];
        while (temp != nullptr) {
            Message *to_free = temp;
            temp = temp->next;
            free(to_free);
        }
    }
    free(table);
}

void MessageContainer::insert(uint16_t size, uint8_t type, uint64_t id, uint64_t data) {
    unsigned int index = hash(id);
    Message *new_message = create_message(size, type, id, data);

    if (!new_message) {
        return;
    }

    if (table[index] == nullptr) {
        table[index] = new_message;
    } else {
        Message *temp = table[index];
        while (temp->next != nullptr) {
            temp = temp->next;
        }
        temp->next = new_message;
    }
}

Message *MessageContainer::find(uint64_t id) {
    unsigned int index = hash(id);
    Message *temp = table[index];

    while (temp != nullptr) {
        if (temp->id == id) {
            return temp;
        }
        temp = temp->next;
    }
    return nullptr;
}

Message *MessageContainer::create_message(uint16_t size, uint8_t type, uint64_t id, uint64_t data) {
    Message *new_message = (Message *) malloc(sizeof(Message));
    if (!new_message) {
        printf("Failed to allocate memory");
        return nullptr;
    }
    new_message->size = size;
    new_message->type = type;
    new_message->id = id;
    new_message->data = data;
    new_message->next = nullptr;
    return new_message;
}

unsigned int MessageContainer::hash(uint64_t MessageId) {
    return MessageId % TABLE_SIZE;
}
