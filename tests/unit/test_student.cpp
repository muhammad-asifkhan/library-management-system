#include "../test_utils.hpp"
#include "../../include/core/Student.hpp"
int main() {
    Student stu(123456,"Alice","Physics","alice@example.com");
    CHECK(stu.getId() == 123456);
    CHECK(stu.canBorrowMore());
    CHECK(stu.borrowBook(1001));
    CHECK(!stu.hasBook(9999));
    CHECK(stu.hasBook(1001));
    CHECK(stu.returnBook(1001));
    CHECK(!stu.hasBook(1001));
    return 0;
}

