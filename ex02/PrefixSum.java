import java.util.Arrays;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

class PrefixSum {
    public static void main(String[] args) {
        // Load the CSV file.
        CsvLoader fileLoader = new CsvLoader();
        double[] doubleList = fileLoader.loadCsv(args[0]);

        double[] finalArray = computePrefixSum(doubleList);
        System.out.println(Arrays.stream(finalArray).mapToObj(String::valueOf).collect(Collectors.joining(", ")));
    }

    public static double[] computePrefixSum(double[] input) {
        double[] step1List = step1(input);
        double[] step2List = step2(step1List);
        double[] step3List = step3(step2List);
        double[] output = addRemainingElements(step3List);
        return output;
    }

    private static double[] step1(double[] input) {
        int executorWorkload = input.length / Constants.executorAmount;
        double[] output = input;

        IntStream.range(0, Constants.executorAmount).parallel().forEach(executorIndex -> {
            int executorPosition = executorIndex * executorWorkload;

            for (int index = executorPosition + 1; index < executorPosition + executorWorkload; index++) {
                output[index] = output[index - 1] + input[index];
            }
        });
        return output;
    }

    private static double[] step2(double[] input) {
        int executorWorkload = input.length / Constants.executorAmount;
        double[] output = input;

        for (int i = executorWorkload * 2 - 1; i < input.length; i += executorWorkload) {
            output[i] += output[i - executorWorkload];
        }
        return output;
    }

    private static double[] step3(double[] input) {
        int executorWorkload = input.length / Constants.executorAmount;
        double[] output =  input;

        IntStream.range(1, Constants.executorAmount).parallel().forEach(executorIndex -> {
            int executorPosition = executorIndex * executorWorkload;

            for (int index = executorPosition; index < executorPosition + executorWorkload - 1; index++) {
                output[index] = input[index] + input[executorPosition - 1];

            }
        });
        return output;
    }

    // in case len(array) not divisible by number of processors
    private static double[] addRemainingElements(double[] input) {
        double[] output =  input;
        int remaining = input.length % Constants.executorAmount;

        for (int i = input.length - remaining; i < input.length; i++) {
            output[i] = input[i - 1] + input[i];
        }

        return output;
    }
}