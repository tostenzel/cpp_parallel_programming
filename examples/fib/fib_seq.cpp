#include <iostream>
#include <thread>
#include <vector>

int fib(int n) {
    int a_0 = 0;
    int a_1 = 1;
    for (int i = 0; i < n; ++i) {
        const int tmp = a_0;
        a_0 = a_1;
        a_1 += tmp;
    }

    return a_0;
}

int main() {
    for (int i = 0; i < 32; ++i)
        std::cout << fib(i) << std::endl;
}
