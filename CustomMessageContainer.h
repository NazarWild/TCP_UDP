#ifndef CUSTOMMESSAGECONTAINER_H
#define CUSTOMMESSAGECONTAINER_H

#include <mutex>
#include <condition_variable>

class CustomMessageContainer {
    struct Node {
        char *message;
        Node *next;

        Node(char *msg) : message(msg), next(nullptr) {
        }
    };

    Node *head;
    Node *tail;
    std::mutex mtx;
    std::condition_variable cv;
    bool finished;

public:
    CustomMessageContainer() : head(nullptr), tail(nullptr), finished(false) {
    }

    ~CustomMessageContainer();

    void addMessage(char *new_message);

    char *getMessage();

    void finish();
};


#endif //CUSTOMMESSAGECONTAINER_H
