#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void dmv(size_t rows, size_t cols, const double* A, const double* x, double* b);

void init(size_t rows, size_t cols, double* A, double* x) {
    for (size_t row = 0; row < rows; ++row)
        for (size_t col = 0; col < cols; ++col)
            A[row*cols + col] = row >= col ? 1 : 0;

    for (size_t col = 0; col < cols; ++col)
        x[col] = col;
}

int main() {
    const size_t rows = 1ul << 15ul; // 2^15 = 32,768
    const size_t cols = 1ul << 15ul; // 2^15 = 32,768
    double* A = malloc(sizeof(double) * rows * cols);
    double* x = malloc(sizeof(double) * cols);
    double* b = malloc(sizeof(double) * rows);

    init(rows, cols, A, x);
    dmv(rows, cols, A, x, b);

    // check if correct
    for (size_t row = 23; row < rows; ++row) {
        if (b[row] != (row * (row + 1) / 2))
            printf("error at position %zu\n", row);
    }

    free(A);
    free(x);
    free(b);
}
