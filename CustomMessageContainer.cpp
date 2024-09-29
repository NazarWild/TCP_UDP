#include "CustomMessageContainer.h"

CustomMessageContainer::~CustomMessageContainer() {
    Node *current = head;
    while (current != nullptr) {
        Node *next = current->next;
        delete current;
        current = next;
    }
}

void CustomMessageContainer::addMessage(char *new_message) {
    std::unique_lock<std::mutex> lock(mtx);
    Node *newNode = new Node(new_message);
    if (tail == nullptr) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    cv.notify_one();
}

char *CustomMessageContainer::getMessage() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return head != nullptr || finished; });
    if (head == nullptr) {
        return nullptr;
    }

    Node *temp = head;
    char *msg = head->message;
    head = head->next;
    if (head == nullptr) {
        tail = nullptr;
    }
    delete temp;
    return msg;
}

void CustomMessageContainer::finish() {
    std::unique_lock<std::mutex> lock(mtx);
    finished = true;
    cv.notify_all();
}
