// BST implementation file — Template must be included at end of header.
#pragma once
#include <cctype>
#include <mutex>
#include <algorithm>
// ... Helper function definitions for BST ...
template<typename T>
BSTNode<T>* BST<T>::insertRecursive(BSTNode<T>* node, const T& value, BSTNode<T>* parent) {
    if (!node) {
        auto newNode = std::make_unique<BSTNode<T>>(value);
        newNode->parent = parent;
        node = newNode.get();
        if (!parent) root = std::move(newNode);
        else if (value < parent->data) parent->left = std::move(newNode);
        else parent->right = std::move(newNode);
        return node;
    }
    if (value < node->data)
        insertRecursive(node->left.get(), value, node);
    else
        insertRecursive(node->right.get(), value, node);
    return node;
}
template<typename T>
bool BST<T>::insert(const T& value) {
    std::unique_lock lock(treeMutex);
    if (!root) {
        root = std::make_unique<BSTNode<T>>(value);
        nodeCount++;
        return true;
    }
    // Use non-locking search to avoid deadlock while holding unique_lock
    if (searchNoLock(value.getId())) {
        throw std::runtime_error("Duplicate ID " + std::to_string(value.getId()));
    }
    insertRecursive(root.get(), value, nullptr);
    nodeCount++;
    return true;
}
template<typename T>
BSTNode<T>* BST<T>::searchRecursive(BSTNode<T>* node, int id) const {
    if (!node) return nullptr;
    if (node->data.getId() == id) return node;
    if (id < node->data.getId()) return searchRecursive(node->left.get(), id);
    else return searchRecursive(node->right.get(), id);
}

template<typename T>
BSTNode<T>* BST<T>::removeRecursive(BSTNode<T>* node, int id, std::unique_ptr<BSTNode<T>>& nodePtr) {
    if (!node) return nullptr;

    if (id < node->data.getId()) {
        return removeRecursive(node->left.get(), id, nodePtr->left);
    }
    if (id > node->data.getId()) {
        return removeRecursive(node->right.get(), id, nodePtr->right);
    }

    // Found node to remove: `nodePtr` owns `node`
    // Case 1: leaf
    if (!node->left && !node->right) {
        nodePtr.reset();
        return nullptr;
    }

    // Case 2: one child
    if (node->left && !node->right) {
        auto child = std::move(nodePtr->left);
        if (child) child->parent = node->parent;
        nodePtr = std::move(child);
        return nodePtr.get();
    }
    if (!node->left && node->right) {
        auto child = std::move(nodePtr->right);
        if (child) child->parent = node->parent;
        nodePtr = std::move(child);
        return nodePtr.get();
    }

    // Case 3: two children - replace with inorder successor (min of right subtree)
    BSTNode<T>* succParent = node;
    std::unique_ptr<BSTNode<T>>* succPtr = &nodePtr->right;
    BSTNode<T>* succ = node->right.get();

    while (succ->left) {
        succParent = succ;
        succPtr = &succParent->left;
        succ = succ->left.get();
    }

    // Copy successor data into current node
    node->data = succ->data;

    // Remove successor node (it has no left child)
    if ((*succPtr)->right) {
        auto child = std::move((*succPtr)->right);
        if (child) child->parent = succParent;
        *succPtr = std::move(child);
    } else {
        succPtr->reset();
    }

    return node;
}
template<typename T>
T* BST<T>::search(int id) const {
    std::shared_lock lock(treeMutex);
    BSTNode<T>* node = searchRecursive(root.get(), id);
    return node ? &node->data : nullptr;
}
template<typename T>
void BST<T>::inorderRecursive(BSTNode<T>* node, std::vector<T>& result) const {
    if (!node) return;
    inorderRecursive(node->left.get(), result);
    result.push_back(node->data);
    inorderRecursive(node->right.get(), result);
}
template<typename T>
std::vector<T> BST<T>::getAllBooks() const {
    std::shared_lock lock(treeMutex);
    std::vector<T> result;
    inorderRecursive(root.get(), result);
    return result;
}
template<typename T>
bool BST<T>::update(int id, const T& newValue) {
    std::unique_lock lock(treeMutex);

    BSTNode<T>* node = searchRecursive(root.get(), id);
    if (!node) return false;

    if (id != newValue.getId()) {
        // Don't allow duplicate new IDs
        if (searchNoLock(newValue.getId())) return false;

        // Remove old node without re-locking
        removeRecursive(root.get(), id, root);
        nodeCount--;

        // Insert new value without re-locking
        if (!root) {
            root = std::make_unique<BSTNode<T>>(newValue);
            root->parent = nullptr;
        } else {
            insertRecursive(root.get(), newValue, nullptr);
        }
        nodeCount++;
        return true;
    }

    node->data = newValue;
    return true;
}
template<typename T>
void BST<T>::clearRecursive(std::unique_ptr<BSTNode<T>>& node) {
    if (node) {
        clearRecursive(node->left);
        clearRecursive(node->right);
        node.reset();
    }
}
template<typename T>
void BST<T>::clear() {
    std::unique_lock lock(treeMutex);
    clearRecursive(root);
    nodeCount = 0;
}
template<typename T>
void BST<T>::displayAll() const {
    std::vector<T> books = getAllBooks();
    std::cout << "Book Catalog:" << std::endl;
    for (const auto& book : books) {
        std::cout << book.toString() << std::endl;
    }
}
template<typename T>
int BST<T>::heightRecursive(BSTNode<T>* node) const {
    if (!node) return 0;
    return 1 + std::max(heightRecursive(node->left.get()), heightRecursive(node->right.get()));
}
template<typename T>
bool BST<T>::remove(int id) {
    std::unique_lock lock(treeMutex);
    if (!root) return false;
    if (!searchNoLock(id)) return false;

    removeRecursive(root.get(), id, root);
    nodeCount--;
    return true;
}
template<typename T>
std::vector<T> BST<T>::searchByTitle(const std::string& title) const {
    std::shared_lock lock(treeMutex);
    std::vector<T> results;
    std::queue<BSTNode<T>*> nodeQueue;
    if (root) nodeQueue.push(root.get());
    std::string needle = title;
    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
    while (!nodeQueue.empty()) {
        BSTNode<T>* n = nodeQueue.front(); nodeQueue.pop();
        std::string t = n->data.getTitle();
        std::transform(t.begin(), t.end(), t.begin(), ::tolower);
        if (t.find(needle) != std::string::npos) results.push_back(n->data);
        if (n->left) nodeQueue.push(n->left.get());
        if (n->right) nodeQueue.push(n->right.get());
    }
    return results;
}
template<typename T>
std::vector<T> BST<T>::searchByAuthor(const std::string& author) const {
    std::shared_lock lock(treeMutex);
    std::vector<T> results;
    std::queue<BSTNode<T>*> nodeQueue;
    if (root) nodeQueue.push(root.get());
    std::string needle = author;
    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
    while (!nodeQueue.empty()) {
        BSTNode<T>* n = nodeQueue.front(); nodeQueue.pop();
        std::string a = n->data.getAuthor();
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        if (a.find(needle) != std::string::npos) results.push_back(n->data);
        if (n->left) nodeQueue.push(n->left.get());
        if (n->right) nodeQueue.push(n->right.get());
    }
    return results;
}
template<typename T>
std::vector<T> BST<T>::searchRange(int minId, int maxId) const {
    std::shared_lock lock(treeMutex);
    std::vector<T> result;
    searchRangeRecursive(root.get(), minId, maxId, result);
    return result;
}
template<typename T>
void BST<T>::searchRangeRecursive(BSTNode<T>* node, int minId, int maxId, std::vector<T>& result) const {
    if (!node) return;
    if (node->data.getId() >= minId) searchRangeRecursive(node->left.get(), minId, maxId, result);
    if (node->data.getId() >= minId && node->data.getId() <= maxId) result.push_back(node->data);
    if (node->data.getId() <= maxId) searchRangeRecursive(node->right.get(), minId, maxId, result);
}

