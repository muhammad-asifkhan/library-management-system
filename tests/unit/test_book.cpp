#include "../test_utils.hpp"
#include "../../include/core/Book.hpp"
int main() {
    Book book(123,"Test Title","Test Author","1234567890123",5);
    CHECK(book.getId()==123);
    CHECK(book.getTitle()=="Test Title");
    CHECK(book.isAvailable());
    CHECK(book.borrowCopy());
    CHECK(book.getAvailableCount()==4);
    CHECK(book.returnCopy());
    CHECK(book.getAvailableCount()==5);
    return 0;
}

