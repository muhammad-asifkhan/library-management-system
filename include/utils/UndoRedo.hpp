#ifndef UNDOREDO_HPP
#define UNDOREDO_HPP
#include <stack>
#include <mutex>
template<typename T>
class UndoRedoSystem {
    std::stack<T> undoStack, redoStack;
    mutable std::mutex mtx;
public:
    void recordAction(const T& t) { std::lock_guard<std::mutex> l(mtx); undoStack.push(t); while (!redoStack.empty()) redoStack.pop(); }
    bool canUndo() const { return !undoStack.empty(); }
    bool canRedo() const { return !redoStack.empty(); }
    T undo() { std::lock_guard<std::mutex> l(mtx); if (undoStack.empty()) throw std::runtime_error("No undo"); auto t = undoStack.top(); undoStack.pop(); redoStack.push(t); return t; }
    T redo() { std::lock_guard<std::mutex> l(mtx); if (redoStack.empty()) throw std::runtime_error("No redo"); auto t = redoStack.top(); redoStack.pop(); undoStack.push(t); return t; }
    void clear() { std::lock_guard<std::mutex> l(mtx); while (!undoStack.empty()) undoStack.pop(); while (!redoStack.empty()) redoStack.pop(); }
};
#endif // UNDOREDO_HPP

