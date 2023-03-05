#ifndef PP_UTIL_H
#define PP_UTIL_H

#include <chrono>

inline auto start_timer() { return std::chrono::system_clock::now(); }

inline auto stop_timer(std::chrono::time_point<std::chrono::system_clock> t1, const char* s) {
    auto t2 = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    std::cout << "# elapsed time ("<< s <<"): " << ms << "ms" << std::endl;
}

template<class T>
auto ceil_div(T x, T y) {
    return (x + y - T(1)) / y;
}

#endif
