#include <iostream>
#include <time.h>

#include "matrix.h"
#include "util.h"

#define debug 0

template <class T> void fillMatrixSequential(Matrix<T> *matrix) {
  srand(time(NULL));
  for (size_t y = 0; y < matrix->getDimY(); y++) {
    for (size_t x = 0; x < matrix->getDimX(); x++) {
      (*matrix)[x][y] = rand() % 101;
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 4)
    return EXIT_FAILURE;

  
  size_t matrixDimensions = std::stoi(argv[1]);
  size_t numThreads = std::stoi(argv[2]);
  size_t parallelizationMode = std::stoi(argv[3]);

  // Verify that parallelization mode exists
  if (parallelizationMode > 2) {
    std::cout << "Incorrect distribution strategy" << std::endl;
    return EXIT_FAILURE;
  }

  // Create input matrices
  Matrix<double> matrix1 = Matrix<double>(matrixDimensions, matrixDimensions);
  // Needed only for matrix1, because it executes the operations
  matrix1.setNumThreads(numThreads);
  matrix1.setParallelizationMode(parallelizationMode);
  matrix1.setBlockSize(8);

  Matrix<double> matrix2 = Matrix<double>(matrixDimensions, matrixDimensions);
  matrix2.setNumThreads(numThreads);

  std::cout << "Input parameters: " << matrixDimensions << " " <<  numThreads
            << " " << parallelizationMode << std::endl;

  // Fill matrix
  //fillMatrixSequential(&matrix1);
  //fillMatrixSequential(&matrix2);
  matrix1.randomizeMatrix();
  matrix2.randomizeMatrix();

  if (debug != 0) {
    std::cout << "Matrix 1:" << std::endl;
    std::cout << matrix1 << std::endl;
    std::cout << "Matrix 2:" << std::endl;
    std::cout << matrix2 << std::endl;
  }

  // Sum matrices
  Matrix<double> output = matrix1 + matrix2;

  if (debug != 0) {
    std::cout << "Output matrix:" << std::endl;
    std::cout << output << std::endl;
  }

  // Print last summation time
  std::cout << "# elapsed time: " << matrix1.getLastExecutionTime() << "ms"
            << std::endl;
}
