#ifndef TRANSACTIONTAB_HPP
#define TRANSACTIONTAB_HPP

#include <imgui.h>
#include "../core/LibrarySystem.hpp"

class TransactionTab {
public:
    TransactionTab(LibrarySystem& libSys) : library(libSys) {}
    void render();
private:
    LibrarySystem& library;
};

#endif // TRANSACTIONTAB_HPP

