#ifndef QUEUETAB_HPP
#define QUEUETAB_HPP

#include <imgui.h>
#include "../core/LibrarySystem.hpp"

class QueueTab {
public:
    QueueTab(LibrarySystem& libSys) : library(libSys) {}
    void render();
private:
    LibrarySystem& library;
};

#endif // QUEUETAB_HPP

