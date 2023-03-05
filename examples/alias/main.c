#include <stddef.h>
#include <stdio.h>

void add(size_t n, float*, int*);

int main () {
    float a[] = {100.f, 100.f, 100.f, 100.f};

    add(4, a, (int*) a+2); // 100.f -> 1120403456

    printf("%f, %f, %f, %f\n", a[0], a[1], a[2], a[3]);
    return 0;
}
