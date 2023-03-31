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

public:
    void enqueue(int *client_socket) {
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

class QueryMessageQueue {

private:
    std::queue<QueryMessage> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;

public:
    QueryMessageQueue() = default;

    void enqueue(const QueryMessage& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(message);
        lock.unlock();
        m_cond.notify_one();
    }

    QueryMessage dequeue() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            m_cond.wait(lock);
        }
        auto message = m_queue.front();
        m_queue.pop();
        return message;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
};

class QueryHitMessageQueue {

private:
    std::queue<QueryHitMessage> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;

public:
    QueryHitMessageQueue() = default;

    void enqueue(const QueryHitMessage& message) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_queue.push(message);
        lock.unlock();
        m_cond.notify_one();
    }

    QueryHitMessage dequeue() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            m_cond.wait(lock);
        }
        auto message = m_queue.front();
        m_queue.pop();
        return message;
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }
};

#endif