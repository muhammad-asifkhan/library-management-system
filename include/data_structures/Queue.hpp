#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <memory>
#include <mutex>
#include <vector>
#include <stdexcept>
#include "../core/BookRequest.hpp"
/**
 * @brief Array-based circular queue for BookRequest or generic T
 */
template<typename T>
class ArrayQueue {
private:
    std::unique_ptr<T[]> data;
    int capacity;
    int front;
    int rear;
    int count;
    mutable std::mutex queueMutex;
    void resize(int newCapacity);
public:
    ArrayQueue(int initCap=100) : data(new T[initCap]), capacity(initCap), front(0), rear(0), count(0) {}
    ArrayQueue(ArrayQueue&& other) noexcept : data(std::move(other.data)), capacity(other.capacity), front(other.front), rear(other.rear), count(other.count) {
        other.capacity = 0; other.front = 0; other.rear = 0; other.count = 0;
    }
    ArrayQueue& operator=(ArrayQueue&& other) noexcept {
        if(this != &other) {
            data = std::move(other.data);
            capacity = other.capacity;
            front = other.front;
            rear = other.rear;
            count = other.count;
            other.capacity = 0; other.front = 0; other.rear = 0; other.count = 0;
        }
        return *this;
    }
    bool enqueue(const T& item);
    bool dequeue(T& item);
    bool peek(T& item) const;
    bool isEmpty() const { return count==0; }
    bool isFull() const { return count==capacity; }
    int size() const { return count; }
    bool removeRequest(int requestId);
    bool cancelRequest(int studentId, int bookId);
    std::vector<T> getRequestsForBook(int bookId) const;
    int getPosition(int studentId, int bookId) const;
    std::vector<T> getAll() const;
};
/**
 * @brief Linked-list queue for BookRequest or generic T
 */
template<typename T>
class LinkedListQueue {
private:
    struct QueueNode {
        T data;
        std::unique_ptr<QueueNode> next;
        QueueNode(const T& item) : data(item), next(nullptr) {}
    };
    std::unique_ptr<QueueNode> head;
    QueueNode* tail;
    int count;
    mutable std::mutex queueMutex;
public:
    LinkedListQueue() : head(nullptr), tail(nullptr), count(0) {}
    bool enqueue(const T& item);
    bool dequeue(T& item);
    bool peek(T& item) const;
    bool isEmpty() const { return count==0; }
    int size() const { return count; }
    bool removeRequest(int requestId);
    bool cancelRequest(int studentId, int bookId);
    std::vector<T> getRequestsForBook(int bookId) const;
    int getPosition(int studentId, int bookId) const;
    std::vector<T> getAll() const;
};

#include "Queue_impl.hpp"

#endif // QUEUE_HPP
