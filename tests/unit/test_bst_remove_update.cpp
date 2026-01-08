#include "../test_utils.hpp"
#include "../../include/data_structures/BST.hpp"
#include "../../include/core/Book.hpp"

static Book mkBook(int id) {
    return Book(id, "T" + std::to_string(id), "A", "1234567890123", 1);
}

int main() {
    BST<Book> catalog;

    // Build a 3-node tree (2 is root, 1 left, 3 right)
    catalog.insert(mkBook(2));
    catalog.insert(mkBook(1));
    catalog.insert(mkBook(3));
    CHECK(catalog.size() == 3);

    // Remove leaf
    CHECK(catalog.remove(1));
    CHECK(catalog.size() == 2);
    CHECK(catalog.search(1) == nullptr);

    // Remove node with one child (rebuild shape: insert 1 back, then remove 2)
    catalog.insert(mkBook(1));
    CHECK(catalog.size() == 3);
    CHECK(catalog.remove(2));
    CHECK(catalog.size() == 2);
    CHECK(catalog.search(2) == nullptr);
    CHECK(catalog.search(1) != nullptr);
    CHECK(catalog.search(3) != nullptr);

    // Remove node with two children (rebuild: 2 root again)
    catalog.clear();
    catalog.insert(mkBook(2));
    catalog.insert(mkBook(1));
    catalog.insert(mkBook(3));
    CHECK(catalog.remove(2));
    CHECK(catalog.size() == 2);
    CHECK(catalog.search(2) == nullptr);

    // Update in-place (same ID)
    Book updated = mkBook(3);
    updated.setTitle("Updated");
    CHECK(catalog.update(3, updated));
    auto* found3 = catalog.search(3);
    CHECK(found3 && found3->getTitle() == "Updated");

    // Update ID change (3 -> 10) should move node
    Book moved = mkBook(10);
    moved.setTitle("Moved");
    CHECK(catalog.update(3, moved));
    CHECK(catalog.search(3) == nullptr);
    auto* found10 = catalog.search(10);
    CHECK(found10 && found10->getTitle() == "Moved");

    return 0;
}


