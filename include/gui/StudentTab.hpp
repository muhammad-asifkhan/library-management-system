#ifndef STUDENTTAB_HPP
#define STUDENTTAB_HPP

#include <imgui.h>
#include "../core/LibrarySystem.hpp"

class StudentTab {
public:
    StudentTab(LibrarySystem& libSys) : library(libSys) {}
    void render();
private:
    LibrarySystem& library;
};

#endif // STUDENTTAB_HPP

