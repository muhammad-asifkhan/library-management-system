#pragma once

#include <cstdlib>
#include <iostream>

// CHECK() always evaluates (unlike assert() which is compiled out under NDEBUG).
#define CHECK(cond)                                                                           \
    do {                                                                                      \
        if (!(cond)) {                                                                        \
            std::cerr << "CHECK failed: " #cond << "\n"                                       \
                      << "  file: " << __FILE__ << "\n"                                       \
                      << "  line: " << __LINE__ << std::endl;                                 \
            std::exit(1);                                                                     \
        }                                                                                     \
    } while (0)


