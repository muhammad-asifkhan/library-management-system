#include "../test_utils.hpp"
#include "../../include/core/LibrarySystem.hpp"
int main() {
    LibrarySystem& library = LibrarySystem::getInstance();
    library.clearAllData();
    Book book(1400, "Integration", "Author", "9781111111111", 1);
    Student student(120000, "Integrated User", "Dept", "mail@x.com");
    CHECK(library.addBook(book));
    CHECK(library.registerStudent(student));
    auto borrowResult = library.issueBook(120000, 1400);
    CHECK(borrowResult.success);
    auto returnResult = library.returnBook(120000, 1400);
    CHECK(returnResult.success);
    return 0;
}

