#ifndef HISTORYTAB_HPP
#define HISTORYTAB_HPP

#include <imgui.h>
#include "../core/LibrarySystem.hpp"

class HistoryTab {
public:
    HistoryTab(LibrarySystem& libSys) : library(libSys) {}
    void render();
private:
    LibrarySystem& library;
};

#endif // HISTORYTAB_HPP

