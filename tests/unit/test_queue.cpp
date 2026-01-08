#include "../test_utils.hpp"
#include "../../include/data_structures/Queue.hpp"
int main() {
    ArrayQueue<int> q(4);
    q.enqueue(1); q.enqueue(2); int v = 0;
    CHECK(q.dequeue(v) && v==1);
    LinkedListQueue<int> lq;
    lq.enqueue(99); lq.enqueue(100);
    CHECK(lq.dequeue(v) && v==99);
    return 0;
}

