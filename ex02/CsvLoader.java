import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

public class CsvLoader {
    public double[] loadCsv(String fileName) {
        File file = new File(fileName);
        Scanner scanner = null;
        try {
            scanner = new Scanner(file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }
        scanner.useDelimiter(", ");

        ArrayList<Double> doubleList = new ArrayList<Double>();
        while(scanner.hasNextDouble()) {
            doubleList.add(scanner.nextDouble());
        }

        double[] doubleArray = doubleList.stream().mapToDouble(Double::doubleValue).toArray();
        return doubleArray;
    }
}
