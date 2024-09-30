#ifndef MESSAGECONTAINERSINGLETON_H
#define MESSAGECONTAINERSINGLETON_H

#include <mutex>
#include "MessageContainer.h"
#include "CustomMessageContainer.h"

class MessageMapSingleton {
public:
    static MessageMapSingleton &GetInstance() {
        static MessageMapSingleton instance;
        return instance;
    }

    MessageMapSingleton(const MessageMapSingleton &) = delete;

    MessageMapSingleton &operator=(const MessageMapSingleton &) = delete;

    void ProcessMessage(const char *buffer);

    CustomMessageContainer &GetCustomMessageContainer();

    void StopThreads();

    bool must_stop_ = false;

private:
    void insertMessage(const Message &message);

    MessageMapSingleton() = default;

    std::mutex mutex;
    MessageContainer map_;
    CustomMessageContainer map_for_send_;
};

#endif //MESSAGECONTAINERSINGLETON_H
