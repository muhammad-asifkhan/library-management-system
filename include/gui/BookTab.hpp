#ifndef BOOKTAB_HPP
#define BOOKTAB_HPP

#include <imgui.h>
#include "../core/LibrarySystem.hpp"

class BookTab {
public:
    BookTab(LibrarySystem& libSys) : library(libSys) {}
    void render();
private:
    LibrarySystem& library;
};

#endif // BOOKTAB_HPP

