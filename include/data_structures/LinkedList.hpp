#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include <memory>
#include <mutex>
#include <vector>
#include <iostream>

/**
 * @brief Singly Linked List node
 */
template<typename T>
struct ListNode {
    T data;
    std::unique_ptr<ListNode<T>> next;
    ListNode(const T& value) : data(value), next(nullptr) {}
};

/**
 * @brief Thread-safe singly Linked List for students
 */
template<typename T>
class LinkedList {
private:
    std::unique_ptr<ListNode<T>> head;
    ListNode<T>* tail = nullptr;
    int count = 0;
    mutable std::mutex listMutex;
    ListNode<T>* findNode(int id) const;
    // Non-locking lookup (caller must hold lock)
    ListNode<T>* findNodeNoLock(int id) const {
        ListNode<T>* node = head.get();
        while (node) {
            if (node->data.getId() == id) return node;
            node = node->next.get();
        }
        return nullptr;
    }
public:
    LinkedList() = default;
    ~LinkedList() { clear(); }
    // Add (O(1))
    bool add(const T& student);
    // Search student by ID
    T* findStudent(int studentId) const;
    // Update student by ID
    bool updateStudent(int studentId, const T& updatedStudent);
    // Remove student by ID
    bool removeStudent(int studentId);
    // Display all
    void displayAll() const;
    // Get all students
    std::vector<T> getAllStudents() const;
    // Other methods...
    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
    void clear();
    // Search by name (linear, partial, case-insensitive)
    std::vector<T> searchByName(const std::string& name) const;
};

#include "LinkedList_impl.hpp"

#endif // LINKEDLIST_HPP
