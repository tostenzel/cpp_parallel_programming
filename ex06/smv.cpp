#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <mpi.h>
#include <random>
#include <thread>
#include <time.h>
#include <vector>

using namespace std;

class SparseMatrix {
public:
  vector<size_t> rowOffsets;
  vector<size_t> columnIndices;
  vector<double> data;
  // vector<double> matrix;

  size_t n;
  int previousRowOffset = -1;

  void createMatrix(size_t n) {
    // Save n
    this->n = n;

    // Initialize vectors
    rowOffsets = vector<size_t>(1);
    // First element is always zero
    rowOffsets[0] = 0;
    columnIndices = vector<size_t>();
    data = vector<double>();
    // matrix = vector<double>(n * n);

    // Initialize random
    size_t seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine randomEngine(seed);
    uniform_real_distribution<double> zeroRandom(0, 1);
    uniform_real_distribution<double> elementRandom(-1, 1);
    // uniform_int_distribution<int> elementRandom(1, 4);

    for (size_t index = 0; index < n * n; index++) {
      double zeroRandomValue = zeroRandom(randomEngine);
      if (zeroRandomValue < (double)n / (double)(n * n)) {
        double data = (double)elementRandom(randomEngine);
        appendElement(index, data);
        /*matrix[index] = data;
      } else {
        matrix[index] = 0;*/
      }
    }

    // Finalize row offsets index
    updateRowOffsets(n * n - 1);
    rowOffsets.emplace_back(data.size());
  }

  void appendElement(size_t index, double dataEntry) {
    updateRowOffsets(index);

    // Add to column indices
    int column = index % n;
    columnIndices.emplace_back(column);

    // Add data to data
    data.emplace_back(dataEntry);
  }

  void updateRowOffsets(size_t index) {
    int row = index / n;
    // If the row has changed, add new entry to row offset.
    if (row != previousRowOffset) {
      // Special case for first row
      if (previousRowOffset == -1) {
        previousRowOffset = 0;
      }

      size_t difference = row - previousRowOffset;
      for (size_t index = 0; index < difference; index++) {
        rowOffsets.emplace_back(columnIndices.size());
      }
      previousRowOffset = row;
    }
  }

  vector<double> multiplySimple(vector<double> inputVector) {
    vector<double> result = vector<double>(n);

    for (size_t row = 0; row < rowOffsets.size() - 1; row++) {
      if (rowOffsets[row] != rowOffsets[row + 1]) {
        result[row] =
            multiplyRow(rowOffsets[row], rowOffsets[row + 1], inputVector);
      }
    }

    return result;
  }

  vector<double> multiply(vector<double> inputVector) {
    // Update global matrix details
    size_t globalRowOffset = rowOffsets[0];
    n = rowOffsets.size() - 1;

    vector<double> result = vector<double>(n);

    for (size_t row = 0; row < rowOffsets.size() - 1; row++) {
      size_t current_offset = rowOffsets[row] - globalRowOffset;
      size_t next_offset = rowOffsets[row + 1] - globalRowOffset;
      if (current_offset != next_offset) {
        result[row] = multiplyRow(current_offset, next_offset, inputVector);
      }
    }

    return result;
  }

  vector<double> multiplyParallel(vector<double> inputVector) {
    const auto processor = std::thread::hardware_concurrency();
    size_t numThreads = (processor != 0) ? processor * 2 : 4;
    size_t rows = rowOffsets.size() - 1;

    // Calculate global rowOffset
    size_t globalRowOffset = rowOffsets[0];
    vector<double> results = vector<double>(rows);

    auto block = [&](size_t firstRow, size_t lastRow) {
      for (size_t row = firstRow; row < lastRow; row++) {
        size_t current_offset = rowOffsets[row] - globalRowOffset;
        size_t next_offset = rowOffsets[row + 1] - globalRowOffset;
        if (current_offset != next_offset) {
          results[row] = multiplyRow(current_offset, next_offset, inputVector);
        }
      }
    };

    // Handle special case that there are less rows than threads
    if (rows <= numThreads) {
      std::vector<std::thread> threads;
      for (size_t row = 0; row < rows; row++)
        threads.emplace_back(block, row, row + 1);
      for (auto &thread : threads)
        thread.join();
    } else {
      // Normal case that there are enough rows for all threads
      size_t blockSize = rows / numThreads;
      size_t unEvenBlock = rows % numThreads;
      std::vector<std::thread> threads;
      for (size_t firstRow = 0; firstRow < rows - blockSize - unEvenBlock;
           firstRow += blockSize) {
        size_t lastRow = firstRow + blockSize;
        threads.emplace_back(block, firstRow, lastRow);
      }
      // Handle last thread
      size_t firstRow = rows - blockSize - unEvenBlock;
      size_t lastRow = rows;
      threads.emplace_back(block, firstRow, lastRow);
      for (auto &thread : threads)
        thread.join();
    }
    return results;
  }

  double multiplyRowSIMD(size_t firstRowOffset, size_t lastRowOffset,
                         vector<double> inputVector) {
    double accumulator = 0;
    double vectorMem[4];
    __m256d partialSum = _mm256_setzero_pd();
    // Handle all four segments
    for (size_t index = 0; index < lastRowOffset - firstRowOffset; index += 4) {
      // Gather vector values
      for (size_t vectorIndex = 0; vectorIndex < +4; vectorIndex++) {
        size_t column = columnIndices[firstRowOffset + index + vectorIndex];
        double vectorValue = inputVector[column];
        vectorMem[vectorIndex] = vectorValue;
      }

      // Load gathered vector values to register
      __m256d vectorReg = _mm256_loadu_pd(vectorMem);

      // Load matrix values
      __m256d matrixReg = _mm256_loadu_pd(&data[firstRowOffset + index]);

      // Calculate values
      __m256d multiplicationResults = _mm256_mul_pd(vectorReg, matrixReg);
      __m256d partialSum = _mm256_add_pd(multiplicationResults, partialSum);
    }

    // Accumulate partial sum
    double partialSumMem[4];
    _mm256_storeu_pd(partialSumMem, partialSum);
    for (int part = 0; part < 4; part++) {
      accumulator += partialSumMem[part];
    }

    // Handle uneven
    size_t modulo = (lastRowOffset - firstRowOffset) % 4;
    accumulator +=
        multiplyRow(lastRowOffset - modulo, lastRowOffset, inputVector);

    return accumulator;
  }

  double multiplyRow(size_t firstRowOffset, size_t lastRowOffset,
                     vector<double> inputVector) {
    double accumulator = 0;
    for (size_t index = 0; index < lastRowOffset - firstRowOffset; index++) {
      size_t column = columnIndices[firstRowOffset + index];
      double columnValue = data[firstRowOffset + index];
      double vectorValue = inputVector[column];
      accumulator += columnValue * vectorValue;
    }

    return accumulator;
  }

  void print() {
    std::cout << "rowOffsets:" << std::endl;
    for (size_t index = 0; index < rowOffsets.size(); index++) {
      std::cout << rowOffsets[index] << std::endl;
    }

    std::cout << "columnIndices:" << std::endl;
    for (size_t index = 0; index < columnIndices.size(); index++) {
      std::cout << columnIndices[index] << std::endl;
    }

    std::cout << "data:" << std::endl;
    for (size_t index = 0; index < data.size(); index++) {
      std::cout << data[index] << std::endl;
    }

    /*
    if (MPI::COMM_WORLD.Get_rank() == 0) {
      std::cout << "matrix:" << std::endl;
      for (size_t index = 0; index < matrix.size(); index++) {
        std::cout << matrix[index] << std::endl;
      }
    }
    */
  }
};

class DenseVector {
public:
  vector<double> data;

  void createVector(size_t n) {
    // Initialize vector
    data = vector<double>(n);

    // Initialize random
    size_t seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine randomEngine(seed);
    uniform_real_distribution<double> elementRandom(-1, 1);
    // uniform_int_distribution<int> elementRandom(1, 4);

    for (size_t index = 0; index < n; index++) {
      data[index] = (double)elementRandom(randomEngine);
    }
  }

  void print() {
    std::cout << "vector:" << std::endl;
    for (size_t index = 0; index < data.size(); index++) {
      std::cout << data[index] << std::endl;
    }
  }
};

class DistributedSparseMatrix {
public:
  vector<double> createAndCompute(int matrixDimensions) {
    // Get basic information from the matrix
    int parallelComputers = MPI::COMM_WORLD.Get_size();
    int workers = parallelComputers - 1;
    int rank = MPI::COMM_WORLD.Get_rank();

    // Create matrix and vector on rank 0
    if (rank == 0) {
      return rank0(matrixDimensions, workers);
    } else {
      rankOther();
      return vector<double>();
    }
  }

  vector<double> rank0(int matrixDimensions, int workers) {
    // Create matrix
    SparseMatrix sparseMatrix = SparseMatrix();
    sparseMatrix.createMatrix(matrixDimensions);

    // Create vector
    DenseVector denseVector = DenseVector();
    denseVector.createVector(matrixDimensions);

    // Calculate how many rows every machine should handle
    int shift = (sparseMatrix.rowOffsets.size() - 1) / workers;

    // Distribute data amongst workers
    vector<MPI::Request> sendRequests =
        splitAndSendData(sparseMatrix, denseVector, workers, shift);

    // Calculate rows that couldn't be distributed evenly.
    vector<double> nonEvenResults =
        handleNonEvenRows(sparseMatrix, denseVector, matrixDimensions, workers);

    // Wait that all sending is done
    for (auto sendRequest : sendRequests) {
      sendRequest.Wait();
    }

    // Receive all the work
    vector<double> results = vector<double>();
    for (int workerIndex = 1; workerIndex <= workers; workerIndex++) {
      vector<double> result = vector<double>(shift);
      MPI::COMM_WORLD.Recv(&result[0], shift, MPI::DOUBLE, workerIndex, 8);
      results.insert(results.end(), result.begin(), result.end());
    }
    // And finally add non even rows to results
    results.insert(results.end(), nonEvenResults.begin(), nonEvenResults.end());

    checkResults(sparseMatrix, denseVector, results);
    return results;
  }

  vector<MPI::Request> splitAndSendData(SparseMatrix sparseMatrix,
                                        DenseVector denseVector, int workers,
                                        int shift) {
    // Print matrix and vector
    // sparseMatrix.print();
    // denseVector.print();

    // Prepare array for anonymous sends
    vector<MPI::Request> sendRequests(workers * 7);

    int startRow = 0;
    for (int workerIndex = 1; workerIndex <= workers; workerIndex++) {
      int endRow = shift * workerIndex - 1;
      int requestIndex = (workerIndex - 1) * 7;

      // Splitting of the matrix
      unsigned long dataStartIndex = sparseMatrix.rowOffsets[startRow];
      unsigned long dataEndIndex = sparseMatrix.rowOffsets[endRow + 1];
      unsigned long dataLength = dataEndIndex - dataStartIndex;

      // Send sizes
      unsigned long rowOffsetsSize = shift;
      sendRequests[requestIndex] = MPI::COMM_WORLD.Isend(
          &rowOffsetsSize, 1, MPI::UNSIGNED_LONG, workerIndex, 1);
      sendRequests[requestIndex + 1] = MPI::COMM_WORLD.Isend(
          &dataLength, 1, MPI::UNSIGNED_LONG, workerIndex, 2);
      unsigned long denseVectorSize = denseVector.data.size();
      sendRequests[requestIndex + 2] = MPI::COMM_WORLD.Isend(
          &denseVectorSize, 1, MPI::UNSIGNED_LONG, workerIndex, 3);

      // Send vectors
      sendRequests[requestIndex + 3] =
          MPI::COMM_WORLD.Isend(&sparseMatrix.rowOffsets[startRow], shift,
                                MPI::UNSIGNED_LONG, workerIndex, 4);
      sendRequests[requestIndex + 4] =
          MPI::COMM_WORLD.Isend(&sparseMatrix.columnIndices[dataStartIndex],
                                dataLength, MPI::UNSIGNED_LONG, workerIndex, 5);
      sendRequests[requestIndex + 5] =
          MPI::COMM_WORLD.Isend(&sparseMatrix.data[dataStartIndex], dataLength,
                                MPI::DOUBLE, workerIndex, 6);
      sendRequests[requestIndex + 6] =
          MPI::COMM_WORLD.Isend(&denseVector.data[0], denseVector.data.size(),
                                MPI::DOUBLE, workerIndex, 7);

      startRow = endRow + 1;
    }
    return sendRequests;
  }

  vector<double> handleNonEvenRows(SparseMatrix sparseMatrix,
                                   DenseVector denseVector,
                                   int matrixDimensions, int workers) {
    // Calculate non even rows on rank 0
    int notEvenRows = matrixDimensions % workers;
    vector<double> nonEvenResults = vector<double>();
    if (notEvenRows != 0) {
      // Initialize second sparse matrix for non even rows
      SparseMatrix sparseMatrixNonEven = SparseMatrix();

      // Slice ending of the row offsets, column indices and data to
      // non even matrix
      sparseMatrixNonEven.rowOffsets = vector<size_t>(
          sparseMatrix.rowOffsets.begin() + (matrixDimensions - notEvenRows),
          sparseMatrix.rowOffsets.end());
      unsigned long dataStartIndex =
          sparseMatrix.rowOffsets[matrixDimensions - notEvenRows];
      sparseMatrixNonEven.columnIndices =
          vector<size_t>(sparseMatrix.columnIndices.begin() + dataStartIndex,
                         sparseMatrix.columnIndices.end());
      sparseMatrixNonEven.data = vector<double>(
          sparseMatrix.data.begin() + dataStartIndex, sparseMatrix.data.end());

      // Multiply non even parts of the rows
      nonEvenResults = sparseMatrixNonEven.multiplyParallel(denseVector.data);
    }
    return nonEvenResults;
  }

  void rankOther() {
    // Initialize sparse matrix and dense vector
    SparseMatrix sparseMatrix = SparseMatrix();
    DenseVector denseVector = DenseVector();

    // Receive vector lengths
    size_t rowOffsetsSize;
    MPI::COMM_WORLD.Recv(&rowOffsetsSize, 1, MPI::UNSIGNED_LONG, 0, 1);
    size_t dataSize;
    MPI::COMM_WORLD.Recv(&dataSize, 1, MPI::UNSIGNED_LONG, 0, 2);
    size_t denseVectorSize;
    MPI::COMM_WORLD.Recv(&denseVectorSize, 1, MPI::UNSIGNED_LONG, 0, 3);

    // Initialize vectors
    sparseMatrix.rowOffsets = vector<size_t>(rowOffsetsSize);
    sparseMatrix.columnIndices = vector<size_t>(dataSize);
    sparseMatrix.data = vector<double>(dataSize);
    denseVector.data = vector<double>(denseVectorSize);

    // Receive data
    MPI::COMM_WORLD.Recv(&sparseMatrix.rowOffsets[0], rowOffsetsSize,
                         MPI::UNSIGNED_LONG, 0, 4);
    MPI::COMM_WORLD.Recv(&sparseMatrix.columnIndices[0], dataSize,
                         MPI::UNSIGNED_LONG, 0, 5);
    MPI::COMM_WORLD.Recv(&sparseMatrix.data[0], dataSize, MPI::DOUBLE, 0, 6);
    MPI::COMM_WORLD.Recv(&denseVector.data[0], denseVectorSize, MPI::DOUBLE, 0,
                         7);

    // Fix rowOffsets
    sparseMatrix.rowOffsets.emplace_back(sparseMatrix.rowOffsets[0] +
                                         sparseMatrix.columnIndices.size());

    // Do calculations
    vector<double> results = sparseMatrix.multiplyParallel(denseVector.data);

    // Send results
    MPI::COMM_WORLD.Send(&results[0], results.size(), MPI::DOUBLE, 0, 8);
  }

  void checkResults(SparseMatrix sparseMatrix, DenseVector denseVector,
                    vector<double> results) {
    // Calculate results on a single thread with naive functionality
    vector<double> correctResults =
        sparseMatrix.multiplySimple(denseVector.data);

    // Check results
    for (size_t index = 0; index < correctResults.size(); index++) {
      if (results[index] != correctResults[index]) {
        std::cout << "THERE WERE ERROR IN RESULTS" << std::endl;
        return;
      }
    }
    std::cout << "RESULTS WERE CORRECT" << std::endl;
  }
};

int main(int argc, char **argv) {
  if (argc != 2)
    return EXIT_FAILURE;
  size_t n = std::stoi(argv[1]);

  // Initialize OpenMPI
  MPI::Init(argc, argv);

  // SparseMatrix sparseMatrix = SparseMatrix();
  // sparseMatrix.createMatrix(n);

  // DenseVector denseVector = DenseVector();
  // denseVector.createVector(n);

  // sparseMatrix.print();
  // denseVector.print();

  // vector<double> results = sparseMatrix.multiplyNaive(denseVector.data);

  DistributedSparseMatrix distributedSparseMatrix = DistributedSparseMatrix();
  vector<double> results = distributedSparseMatrix.createAndCompute(n);

  /*
  if (results.size() != 0) {
    std::cout << "result vector:" << std::endl;
    for (size_t index = 0; index < results.size(); index++) {
      std::cout << results[index] << std::endl;
    }
  }
  */

  MPI::Finalize();

  return EXIT_SUCCESS;
}
