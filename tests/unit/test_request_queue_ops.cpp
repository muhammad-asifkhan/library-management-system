#include "../test_utils.hpp"
#include "../../include/data_structures/Queue.hpp"

int main() {
    ArrayQueue<BookRequest> q(4);

    BookRequest r1(100001, 2001, 3);
    BookRequest r2(100002, 2001, 3);
    BookRequest r3(100003, 2001, 1);

    CHECK(q.enqueue(r1));
    CHECK(q.enqueue(r2));
    CHECK(q.enqueue(r3));
    CHECK(q.size() == 3);

    CHECK(q.getPosition(100001, 2001) == 1);
    CHECK(q.getPosition(100002, 2001) == 2);
    CHECK(q.getPosition(100003, 2001) == 3);
    CHECK(q.getPosition(999999, 2001) == -1);

    // Cancel middle request
    CHECK(q.cancelRequest(100002, 2001));
    CHECK(q.size() == 2);
    CHECK(q.getPosition(100001, 2001) == 1);
    CHECK(q.getPosition(100003, 2001) == 2);

    // Cancel non-existing should fail
    CHECK(!q.cancelRequest(100002, 2001));

    return 0;
}


