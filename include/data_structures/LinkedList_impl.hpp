#pragma once
#include <algorithm>
#include <cctype>
template<typename T>
ListNode<T>* LinkedList<T>::findNode(int id) const {
    ListNode<T>* node = head.get();
    while (node) {
        if (node->data.getId() == id) return node;
        node = node->next.get();
    }
    return nullptr;
}
template<typename T>
bool LinkedList<T>::add(const T& student) {
    std::lock_guard<std::mutex> lock(listMutex);
    if (findNodeNoLock(student.getId())) throw std::runtime_error("Duplicate ID " + std::to_string(student.getId()));
    auto newNode = std::make_unique<ListNode<T>>(student);
    if (!head) { head = std::move(newNode); tail = head.get(); }
    else { tail->next = std::move(newNode); tail = tail->next.get(); }
    count++;
    return true;
}
template<typename T>
T* LinkedList<T>::findStudent(int studentId) const {
    std::lock_guard<std::mutex> lock(listMutex);
    ListNode<T>* n = findNode(studentId);
    return n ? &n->data : nullptr;
}
template<typename T>
bool LinkedList<T>::updateStudent(int studentId, const T& updatedStudent) {
    std::lock_guard<std::mutex> lock(listMutex);
    ListNode<T>* node = findNodeNoLock(studentId);
    T* s = node ? &node->data : nullptr;
    if (!s) return false;
    std::vector<int> borrowedBooks = s->getBorrowedBooks();
    *s = updatedStudent;
    for (int bookId : borrowedBooks) s->borrowBook(bookId);
    return true;
}
template<typename T>
bool LinkedList<T>::removeStudent(int studentId) {
    std::lock_guard<std::mutex> lock(listMutex);
    if (!head) return false;
    if (head->data.getId() == studentId) {
        head = std::move(head->next);
        if (!head) tail = nullptr;
        count--;
        return true;
    }
    ListNode<T>* prev = head.get();
    while (prev->next && prev->next->data.getId() != studentId) prev = prev->next.get();
    if (!prev->next) return false;
    if (prev->next.get() == tail) tail = prev;
    prev->next = std::move(prev->next->next);
    count--;
    return true;
}
template<typename T>
void LinkedList<T>::displayAll() const {
    std::lock_guard<std::mutex> lock(listMutex);
    ListNode<T>* n = head.get();
    while (n) { std::cout << n->data.toString() << std::endl; n = n->next.get(); }
}
template<typename T>
void LinkedList<T>::clear() {
    std::lock_guard<std::mutex> lock(listMutex);
    while (head) head = std::move(head->next);
    tail = nullptr;
    count = 0; 
}
template<typename T>
std::vector<T> LinkedList<T>::getAllStudents() const {
    std::lock_guard<std::mutex> lock(listMutex);
    std::vector<T> result;
    ListNode<T>* n = head.get();
    while (n) { result.push_back(n->data); n = n->next.get(); }
    return result;
}
template<typename T>
std::vector<T> LinkedList<T>::searchByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(listMutex);
    std::vector<T> result;
    ListNode<T>* n = head.get();
    std::string needle = name;
    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
    while (n) {
        std::string nm = n->data.getName();
        std::transform(nm.begin(), nm.end(), nm.begin(), ::tolower);
        if (nm.find(needle) != std::string::npos) result.push_back(n->data);
        n = n->next.get();
    }
    return result;
}

