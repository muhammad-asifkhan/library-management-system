#pragma once
#include <algorithm>
#include <type_traits>
#include <vector>
#include "../core/BookRequest.hpp"

template<typename T>
bool ArrayQueue<T>::enqueue(const T& item) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (isFull()) resize(capacity*2);
    data[rear]=item; rear=(rear+1)%capacity; count++;
    return true;
}
template<typename T>
bool ArrayQueue<T>::dequeue(T& item) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (isEmpty()) return false;
    item=data[front]; front=(front+1)%capacity; count--;
    return true;
}
template<typename T>
bool ArrayQueue<T>::peek(T& item) const {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (isEmpty()) return false;
    item=data[front]; return true;
}
template<typename T>
void ArrayQueue<T>::resize(int newCapacity) {
    std::unique_ptr<T[]> newArr(new T[newCapacity]);
    for(int i=0;i<count;++i) newArr[i]=data[(front+i)%capacity];
    data=std::move(newArr); front=0; rear=count; capacity=newCapacity;
}
template<typename T>
bool ArrayQueue<T>::removeRequest(int requestId) {
    std::lock_guard<std::mutex> lock(queueMutex);
    int idx=-1;
    for(int i=0;i<count;++i) if(data[(front+i)%capacity].getRequestId()==requestId) {idx=i;break;}
    if(idx==-1) return false;
    // shift left
    for(int i=idx;i<count-1;++i) data[(front+i)%capacity]=data[(front+i+1)%capacity];
    rear=(rear-1+capacity)%capacity; count--;
    return true;
}
template<typename T>
bool ArrayQueue<T>::cancelRequest(int studentId, int bookId) {
    std::lock_guard<std::mutex> lock(queueMutex);
    int idx=-1;
    for(int i=0;i<count;++i) {
        auto& req=data[(front+i)%capacity];
        if constexpr (std::is_same<T,BookRequest>::value) {
            if(req.getStudentId()==studentId && req.getBookId()==bookId) {idx=i;break;}
        }
    }
    if(idx==-1)return false;
    for(int i=idx;i<count-1;++i) data[(front+i)%capacity]=data[(front+i+1)%capacity];
    rear=(rear-1+capacity)%capacity; count--;
    return true;
}
template<typename T>
std::vector<T> ArrayQueue<T>::getRequestsForBook(int bookId) const {
    std::vector<T> res;
    std::lock_guard<std::mutex> lock(queueMutex);
    for(int i=0;i<count;++i) {
        const auto& req=data[(front+i)%capacity];
        if constexpr (std::is_same<T,BookRequest>::value) {
            if(req.getBookId()==bookId) res.push_back(req);
        }
    }
    return res;
}
template<typename T>
int ArrayQueue<T>::getPosition(int studentId, int bookId) const {
    std::lock_guard<std::mutex> lock(queueMutex);
    for(int i=0;i<count;++i) {
        const auto& req=data[(front+i)%capacity];
        if constexpr (std::is_same<T,BookRequest>::value) {
            if(req.getStudentId()==studentId && req.getBookId()==bookId) return i+1;
        }
    }
    return -1;
}
// LinkedListQueue:
template<typename T>
bool LinkedListQueue<T>::enqueue(const T& item) {
    std::lock_guard<std::mutex> lock(queueMutex);
    auto newNode=std::make_unique<QueueNode>(item);
    if(!head) { head=std::move(newNode); tail=head.get(); }
    else { tail->next=std::move(newNode); tail=tail->next.get(); }
    count++; return true;
}
template<typename T>
bool LinkedListQueue<T>::dequeue(T& item) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if(!head) return false;
    item=head->data; head=std::move(head->next); if(!head)tail=nullptr; count--; return true;
}
template<typename T>
bool LinkedListQueue<T>::peek(T& item) const {
    std::lock_guard<std::mutex> lock(queueMutex);
    if(!head) return false;
    item=head->data;
    return true;
}
template<typename T>
bool LinkedListQueue<T>::removeRequest(int requestId) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if(!head) return false;
    if(head->data.getRequestId()==requestId) {
        head=std::move(head->next);
        if(!head) tail=nullptr;
        count--;
        return true;
    }
    QueueNode* p=head.get();
    while(p->next && p->next->data.getRequestId()!=requestId) p=p->next.get();
    if(!p->next) return false;
    if(p->next.get()==tail) tail=p;
    p->next=std::move(p->next->next);
    count--;
    return true;
}
template<typename T>
bool LinkedListQueue<T>::cancelRequest(int studentId, int bookId) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if(!head) return false;
    if constexpr(std::is_same<T,BookRequest>::value) {
        if(head->data.getStudentId()==studentId&&head->data.getBookId()==bookId) {
            head=std::move(head->next);
            count--;
            if(!head) tail=nullptr;
            return true;
        }
        QueueNode* p=head.get();
        while(p->next&&(p->next->data.getStudentId()!=studentId||p->next->data.getBookId()!=bookId)) p=p->next.get();
        if(!p->next) return false;
        if(p->next.get()==tail) tail=p;
        p->next=std::move(p->next->next);
        count--;
        return true;
    } else return false;
}
template<typename T>
std::vector<T> LinkedListQueue<T>::getRequestsForBook(int bookId) const {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::vector<T> v; QueueNode* p=head.get();
    while(p) { if constexpr(std::is_same<T,BookRequest>::value) {
        if(p->data.getBookId()==bookId) v.push_back(p->data); } p=p->next.get(); }
    return v;
}
template<typename T>
int LinkedListQueue<T>::getPosition(int studentId, int bookId) const {
    std::lock_guard<std::mutex> lock(queueMutex);
    int idx=1; QueueNode* p=head.get();
    while(p) {
        if constexpr(std::is_same<T,BookRequest>::value) {
            if(p->data.getStudentId()==studentId&&p->data.getBookId()==bookId) return idx;
        }
        p=p->next.get(); ++idx;
    }
    return -1;
}
template<typename T>
std::vector<T> ArrayQueue<T>::getAll() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::vector<T> result;
    for(int i=0;i<count;++i) result.push_back(data[(front+i)%capacity]);
    return result;
}
template<typename T>
std::vector<T> LinkedListQueue<T>::getAll() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    std::vector<T> result;
    QueueNode* p=head.get();
    while(p) { result.push_back(p->data); p=p->next.get(); }
    return result;
}

