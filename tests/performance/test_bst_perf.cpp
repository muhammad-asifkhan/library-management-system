#include <iostream>
#include <chrono>
#include "../../include/data_structures/BST.hpp"
#include "../../include/core/Book.hpp"
int main() {
    BST<Book> catalog;
    size_t N = 5000;
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i=0;i<N;++i)
        catalog.insert(Book(800000+i, "PerfBook", "PerfAuth", "9780000000000", 1));
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inserted " << N << " books in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << " ms\n";
    return 0;
}

