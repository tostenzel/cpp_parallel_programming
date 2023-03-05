#include <iostream>
#include <memory>

template<class V>
void init(size_t rows, size_t cols, V* A, V* x) {
    for (size_t row = 0; row < rows; ++row)
        for (size_t col = 0; col < cols; ++col)
            A[row*cols + col] = row >= col ? 1 : 0;

    for (size_t col = 0; col < cols; ++col)
        x[col] = col;
}

template<class V>
void dmv(size_t rows, size_t cols, const V* A, const V* x, V* b) {
    for (size_t row = 0; row < rows; ++row) {
        V acc = 0;
        for (size_t col = 0; col < cols; ++col)
            acc += A[row*cols + col] * x[col];
        b[row] = acc;
    }
}

int main() {
    const size_t rows = 1ul << 15; // 2^15 = 32,768
    const size_t cols = 1ul << 15; // 2^15 = 32,768
    auto A = std::make_unique<double[]>(rows * cols);
    auto x = std::make_unique<double[]>(cols);
    auto b = std::make_unique<double[]>(rows);

    init(rows, cols, A.get(), x.get());
    dmv(rows, cols, A.get(), x.get(), b.get());

    // check if correct
    for (size_t row = 0; row < rows; ++row)
        if (b[row] != row*(row+1)/2) std::cout << "error at position " << row << std::endl;
    return 0;
}
