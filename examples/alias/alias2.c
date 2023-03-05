#include <stddef.h>

void add(size_t n, float* a, float* restrict p) {
    for (size_t i = 0; i != n; ++i)
        a[i] += *p;
}
