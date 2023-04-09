#ifndef QUEUES_H
#define QUEUES_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "message_structs.hpp"

class SocketQueue {
    struct node {
        struct node* next;
        int *client_socket;
    };

    typedef struct node node_t;

    node_t* head = NULL;
    node_t* tail = NULL;
private:
    mutable std::mutex m_mutex;
public:
    void enqueue(int *client_socket) {
        std::lock_guard<std::mutex> lock(m_mutex);
        node_t *newnode = (node_t*)malloc(sizeof(node_t));
        newnode->client_socket = client_socket;
        newnode->next = NULL;
        if (tail == NULL) {
            head = newnode;
        }
        else {
            tail->next = newnode;
        }
        tail = newnode;
    }

    int* dequeue() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (head == NULL) {
            return NULL;
        }
        node_t *temp = head;
        head = head->next;
        if (head == NULL) {
            tail = NULL;
        }
        int *ret = temp->client_socket;
        free(temp);
        return ret;
    }
};


class FileQueue {
private:
    std::queue<std::pair<std::string, std::string>> file_queue;
    mutable std::mutex f_mutex;
    std::condition_variable fq_cv;

public:
    void enqueue(const std::string& filename, const std::string& host) {
        std::lock_guard<std::mutex> lock(f_mutex);
        file_queue.push(std::make_pair(filename, host));
        fq_cv.notify_one();
    }

    std::pair<std::string, std::string> dequeue() {
        std::unique_lock<std::mutex> lock(f_mutex);
        fq_cv.wait(lock, [this] { return !file_queue.empty(); });
        auto file_pair = file_queue.front();
        file_queue.pop();
        return file_pair;
    }
    bool empty() const {
        std::lock_guard<std::mutex> lock(f_mutex);
        return file_queue.empty();
    }

};
#endif