#ifndef STACK_HPP
#define STACK_HPP

#include <stack>
#include <mutex>

/**
 * @brief Generic stack with mutex for thread safety
 */
template<typename T>
class Stack {
private:
    std::stack<T> stk;
    mutable std::mutex mtx;
public:
    Stack() = default;
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx);
        stk.push(item);
    }
    bool pop(T& item) {
        std::lock_guard<std::mutex> lock(mtx);
        if (stk.empty()) return false;
        item = stk.top();
        stk.pop();
        return true;
    }
    bool top(T& item) const {
        std::lock_guard<std::mutex> lock(mtx);
        if (stk.empty()) return false;
        item = stk.top();
        return true;
    }
    bool isEmpty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return stk.empty();
    }
    int size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return (int)stk.size();
    }
    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        while (!stk.empty()) stk.pop();
    }
};

#endif // STACK_HPP

