#pragma once

#include <cstdlib>
#include <iostream>

#define CMS2_CHECK(expr)                                                              \
    do {                                                                              \
        if (!(expr)) {                                                                \
            std::cerr << "CHECK FAILED: " #expr "\n  at " << __FILE__ << ":"        \
                      << __LINE__ << "\n";                                            \
            std::abort();                                                             \
        }                                                                             \
    } while (false)

#define CMS2_CHECK_MSG(expr, msg)                                                     \
    do {                                                                              \
        if (!(expr)) {                                                                \
            std::cerr << "CHECK FAILED: " << (msg) << "\n  expr: " #expr             \
                      << "\n  at " << __FILE__ << ":" << __LINE__ << "\n";           \
            std::abort();                                                             \
        }                                                                             \
    } while (false)
