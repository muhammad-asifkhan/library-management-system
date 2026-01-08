#include "../test_utils.hpp"
#include "../../include/data_structures/BST.hpp"
#include "../../include/core/Book.hpp"
int main() {
    BST<Book> catalog;
    CHECK(catalog.isEmpty());
    Book book1(1001, "TitleX", "AuthorX", "1234567890123", 3);
    catalog.insert(book1);
    CHECK(!catalog.isEmpty());
    auto* found = catalog.search(1001);
    CHECK(found && found->getId() == 1001);
    return 0;
}

