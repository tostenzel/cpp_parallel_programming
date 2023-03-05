#include <stdio.h>
#include <stdlib.h>

void dmv(size_t rows, size_t cols, const double* A, const double* x, double* b);

int main() {
    const size_t rows = 2;
    const size_t cols = 3;
    double* A = malloc(sizeof(double) * rows * cols);
    double* x = malloc(sizeof(double) * cols);
    double* b = malloc(sizeof(double) * rows);

    //   A   *   x   =  b
    // 1 2 3   1 2 3   14
    // 4 5 6           32

    A[0] = 1.0;
    A[1] = 2.0;
    A[2] = 3.0;
    A[3] = 4.0;
    A[4] = 5.0;
    A[5] = 6.0;

    x[0] = 1.0;
    x[1] = 2.0;
    x[2] = 3.0;

    dmv(rows, cols, A, x, b);

    for (size_t row = 0; row < rows; ++row)
        printf("%zu: %f\n", row, b[row]);

    free(A);
    free(x);
    free(b);
}
