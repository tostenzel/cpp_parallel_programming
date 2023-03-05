import java.util.Arrays;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

public class SparseArrayCompaction {
    public static void main(String[] args) {
        // Load the CSV file.
        CsvLoader fileLoader = new CsvLoader();
        double[] doubleList = fileLoader.loadCsv(args[0]);

        System.out.println(Arrays.stream(doubleList.clone()).mapToObj(String::valueOf).collect(Collectors.joining(", ")));
        double[] binaryTmp = createTmp(doubleList);
        System.out.println(Arrays.stream(binaryTmp).mapToObj(String::valueOf).collect(Collectors.joining(", ")));
        double[] prefixedTmp = PrefixSum.computePrefixSum(binaryTmp);
        System.out.println(Arrays.stream(prefixedTmp).mapToObj(String::valueOf).collect(Collectors.joining(", ")));
        double[] finalArray = createCompactArray(doubleList, prefixedTmp);

        System.out.println(Arrays.stream(finalArray).mapToObj(String::valueOf).collect(Collectors.joining(", ")));
    }

    private static double[] createTmp(double[] input) {
        int executorWorkload = input.length / Constants.executorAmount;
        double[] output = input;

        IntStream.range(0, Constants.executorAmount).parallel().forEach(executorIndex -> {
            int executorPosition = executorIndex * executorWorkload;

            for (int index = executorPosition + 1; index < executorPosition + executorWorkload; index++) {
                output[index] = (input[index] != 0) ? 1 : 0;
            }
        });

        // Take care of the remaining elements sequentially.
        int remaining = input.length % Constants.executorAmount;

        for (int i = input.length - remaining; i < input.length; i++) {
            output[i] = (input[i] != 0) ? 1 : 0;
        }

        return output;
    }

    private static double[] createCompactArray(double[] A, double[] tmp) {
        int executorWorkload = A.length / Constants.executorAmount;
        double[] V = new double[(int)tmp[tmp.length - 1]];

        IntStream.range(0, Constants.executorAmount).parallel().forEach(executorIndex -> {
            int executorPosition = executorIndex * executorWorkload;

            for (int i = executorPosition + 1; i < executorPosition + executorWorkload; i++) {
                if (A[i] != 0) {
                    V[(int)tmp[i] - 1] = A[i];
                }
            }
        });

        // Take care of the remaining elements sequentially.
        int remaining = A.length % Constants.executorAmount;

        for (int i = A.length - remaining; i < A.length; i++) {
            if (A[i] != 0) {
                V[(int)tmp[i] - 1] = A[i];
            }
        }

        return V;
    }
}