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

public class MatrixVector {

    // Mapper Class
    public static class MatrixMapper extends Mapper<Object, Text, Text, Text> {
        private Text outputKey = new Text();
        private Text outputValue = new Text();

        @Override
        public void map(Object key, Text value, Context context) throws IOException, InterruptedException {
            String line = value.toString();
            String[] parts = line.split(",");
            String fileName = ((FileSplit) context.getInputSplit()).getPath().getName();
            String[] splittedName = fileName.split("_");
            int numOfRows = Integer.parseInt(splittedName[1]);
            int numOfCols = Integer.parseInt(splittedName[2].substring(0, splittedName[2].length() - 4));

            if (parts.length == 2) {
                int i = Integer.parseInt(parts[0]);
                int valueMatrix = Integer.parseInt(parts[1]);
              for (int j = 0; j < numOfRows; j++) {
                outputKey.set(String.valueOf(j));
                outputValue.set(i + "," + valueMatrix);
                context.write(outputKey, outputValue);
              }
              return;
            } 

            int row = Integer.parseInt(parts[0]);
            int col = Integer.parseInt(parts[1]);
            int v = Integer.parseInt(parts[2]);
            outputKey.set(String.valueOf(row));
            outputValue.set(col + "," + v);

            context.write(outputKey, outputValue);
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
                int index = Integer.parseInt(parts[0]);
                int temp = map.getOrDefault(index, 1);
                map.put(index, temp * Integer.parseInt(parts[1]));
                int modified = modifiedMap.getOrDefault(index, 0);
                modifiedMap.put(index, modified+1);
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

        job.setJarByClass(MatrixVector.class);
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
