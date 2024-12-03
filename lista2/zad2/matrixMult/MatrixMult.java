import java.io.IOException;
import java.util.HashMap;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;

public class MatrixMult {

    // Mapper Class
    public static class MatrixMapper extends Mapper<Object, Text, Text, Text> {
        private Text outputKey = new Text();
        private Text outputValue = new Text();

        @Override
        public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
            String line = value.toString();
            String[] parts = line.split(",");
            String matrixName = parts[0];
            int i = Integer.parseInt(parts[1]);
            int j = Integer.parseInt(parts[2]);
            int valueMatrix = Integer.parseInt(parts[3]);
            String fileName = ((FileSplit) context.getInputSplit()).getPath().getName();
            String[] splittedName = fileName.split("_");
            int numOfRows = Integer.parseInt(splittedName[1]);
            int numOfCols = Integer.parseInt(splittedName[2].substring(0, splittedName[2].length() - 4));

            if (matrixName.equals("A")) {
                // Emit keys for A(i, k) where k is the column index in the output
                for (int k = 0; k < numOfCols; k++) { // assume P = 100 (cols in B)
                    outputKey.set(i + "," + k);
                    outputValue.set("A," + j + "," + valueMatrix);
                    context.write(outputKey, outputValue);
                }
            } else if (matrixName.equals("B")) {
                // Emit keys for B(k, j) where k is the row index in A
                for (int k = 0; k < numOfRows; k++) { // assume M = 100 (rows in A)
                    outputKey.set(k + "," + j);
                    outputValue.set("B," + i + "," + valueMatrix);
                    context.write(outputKey, outputValue);
                }
            }
        }
    }

    // Reducer Class
    public static class MatrixReducer extends Reducer<Text, Text, Text, IntWritable> {
        @Override
        public void reduce(Text key, Iterable<Text> values, Context context) throws IOException, InterruptedException {
            HashMap<Integer, Integer> map = new HashMap();
            HashMap<Integer, Integer> modifiedMap = new HashMap();

            for (Text value : values) {
                String[] parts = value.toString().split(",");
                if (parts[0].equals("A")) {
                    int index = Integer.parseInt(parts[1]);
                    int temp = map.getOrDefault(index, 1);
                    map.put(index, temp * Integer.parseInt(parts[2]));
                    int modified = modifiedMap.getOrDefault(index, 0);
                    modifiedMap.put(index, modified+1);
                } else if (parts[0].equals("B")) {
                    int index = Integer.parseInt(parts[1]);
                    int temp = map.getOrDefault(index, 1);
                    map.put(index, temp * Integer.parseInt(parts[2]));
                    int modified = modifiedMap.getOrDefault(index, 0);
                    modifiedMap.put(index, modified+1);
                }
            }

            int sum = 0;
            for (int k : map.keySet()) {
                if (modifiedMap.getOrDefault(k, 0) < 2) {
                  continue;
                }
                int val = map.get(k);
                sum += val;
            }
            if (sum != 0) {
              context.write(key, new IntWritable(sum));
            }
        }
    }

    public static void main(String[] args) throws Exception {
        Configuration conf = new Configuration();
        Job job = Job.getInstance(conf, "matrix multiplication");

        job.setJarByClass(MatrixMult.class);
        job.setMapperClass(MatrixMapper.class);
        job.setReducerClass(MatrixReducer.class);
        job.setMapOutputKeyClass(Text.class);
        job.setMapOutputValueClass(Text.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(IntWritable.class);

        FileInputFormat.addInputPath(job, new Path(args[0]));
        FileOutputFormat.setOutputPath(job, new Path(args[1]));

        System.exit(job.waitForCompletion(true) ? 0 : 1);
    }
}
