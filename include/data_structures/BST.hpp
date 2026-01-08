#ifndef BST_HPP
#define BST_HPP

#include <memory>
#include <shared_mutex>
#include <vector>
#include <queue>
#include <algorithm>
#include <iostream>

/**
 * @brief Node for Binary Search Tree
 */
template<typename T>
struct BSTNode {
    T data;
    std::unique_ptr<BSTNode<T>> left;
    std::unique_ptr<BSTNode<T>> right;
    BSTNode<T>* parent;
    BSTNode(const T& value) : data(value), left(nullptr), right(nullptr), parent(nullptr) {}
    bool isLeaf() const { return !left && !right; }
    bool hasOneChild() const { return (left && !right) || (!left && right); }
};

/**
 * @brief Thread-safe Binary Search Tree (BST) template for Book catalog
 */
template<typename T>
class BST {
private:
    std::unique_ptr<BSTNode<T>> root;
    int nodeCount = 0;
    mutable std::shared_mutex treeMutex;
    // Internal helpers
    BSTNode<T>* insertRecursive(BSTNode<T>* node, const T& value, BSTNode<T>* parent);
    BSTNode<T>* searchRecursive(BSTNode<T>* node, int id) const;
    // Non-locking variant used when caller already holds lock
    BSTNode<T>* searchNoLock(int id) const { return searchRecursive(root.get(), id); }
    BSTNode<T>* removeRecursive(BSTNode<T>* node, int id, std::unique_ptr<BSTNode<T>>& nodePtr);
    void inorderRecursive(BSTNode<T>* node, std::vector<T>& result) const;
    void searchRangeRecursive(BSTNode<T>* node, int minId, int maxId, std::vector<T>& result) const;
    int heightRecursive(BSTNode<T>* node) const;
    void clearRecursive(std::unique_ptr<BSTNode<T>>& node);
public:
    BST() = default;
    ~BST() { clear(); }
    // Insert node
    bool insert(const T& value);
    // Search by ID
    T* search(int id) const;
    // Search by title (linear, partial, case-insensitive)
    std::vector<T> searchByTitle(const std::string& title) const;
    // Search by author (optional bonus)
    std::vector<T> searchByAuthor(const std::string& author) const;
    // Inorder traversal (sorted)
    std::vector<T> getAllBooks() const;
    // Update by ID
    bool update(int id, const T& newValue);
    // Remove by ID
    bool remove(int id);
    // Display all
    void displayAll() const;
    // Utility
    bool isEmpty() const { return nodeCount == 0; }
    int size() const { return nodeCount; }
    int height() const { std::shared_lock lock(treeMutex); return heightRecursive(root.get()); }
    void clear();
    // Range search
    std::vector<T> searchRange(int minId, int maxId) const;
};

#include "BST_impl.hpp"

#endif // BST_HPP
