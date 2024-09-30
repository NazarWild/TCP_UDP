#include "MessageContainerSingleton.h"

void MessageMapSingleton::ProcessMessage(const char *buffer) {
    const Message msg = DeserializeMessage(buffer);

    printf("ProcessMessage msg id = %li, type = %llu, size = %llu, data = %li, \n", msg.id, msg.type, msg.size,
           msg.data);

    if (msg.data == 10) {
        char *message_ptr = SerializeMessage(msg);
        map_for_send_.addMessage(message_ptr);
    } else {
        insertMessage(msg);
    }
}

CustomMessageContainer &MessageMapSingleton::GetCustomMessageContainer() {
    return map_for_send_;
}

void MessageMapSingleton::StopThreads() {
    must_stop_ = true;
}

void MessageMapSingleton::insertMessage(const Message &message) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!map_.find(message.id)) {
        map_.insert(message.size, message.type, message.id, message.data);
    }
}
