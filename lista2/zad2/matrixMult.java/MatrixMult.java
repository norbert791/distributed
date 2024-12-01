import java.io.IOException;
import java.util.StringTokenizer;
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

  public static class TokenizerMapper
       extends Mapper<Object, Text, Text, Text>{

    private Text outVal = new Text();
    private Text outKey = new Text();

    public void map(Object key, Text value, Context context
                    ) throws IOException, InterruptedException {
      String name = ((FileSplit) context.getInputSplit()).getPath().getName();
      StringTokenizer itr = new StringTokenizer(value.toString(), " ");
    
      boolean left = false;
      if (name.startsWith("left")) {
        left = true;
      }

      name = name.substring(0, name.length() - 4);
      String[] names = name.split("_");
      int rows = Integer.parseInt(names[1]);
      int cols = Integer.parseInt(names[2]);
      int currRow = 0;
      int currCol = 0;
      while (itr.hasMoreTokens()) {
        String tok = itr.nextToken();
        if (tok == "\n") {
          currRow++;
          continue;
        }

        int val = Integer.parseInt(tok);

        if (left) {
          for (int i = 0; i < cols; i++) {
            String keyStr = String.valueOf(currRow) + " " + String.valueOf(i);
            outVal.set(val + " left " + String.valueOf(currRow));
            outKey.set(keyStr);
            context.write(outKey, outVal);
          }
        } else {
          for (int i = 0; i < rows; i++) {
            String keyStr = String.valueOf(i) + " " + String.valueOf(currCol);
            outVal.set(val + " right " + String.valueOf(currCol));
            outKey.set(keyStr);
            context.write(outKey, outVal);
          }
        }
      }
    }
  }

  public static class IntSumReducer
       extends Reducer<Text,Text,Text,IntWritable> {
    private Text result = new Text();
    private IntWritable resVal = new IntWritable();
    private HashMap<Integer, Integer> map = new HashMap();

    public void reduce(Text key, Iterable<Text> values,
                       Context context
                       ) throws IOException, InterruptedException {
      
      for (Text v : values) {
        String[] arr = v.toString().split(" ");
        int v2 = Integer.parseInt(arr[0]);
        int index = Integer.parseInt(arr[2]);
        int temp = map.getOrDefault(new Integer(index), new Integer(1)).intValue();
        temp *= v2;
        map.put(new Integer(index), temp);
      }
      
      int sum = 0;
      for (Integer v : map.values()) {
        sum += v;
      }

      resVal.set(sum);
      result.set(key);
      context.write(result, resVal);
    }
  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    Job job = Job.getInstance(conf, "word count");
    job.setJarByClass(MatrixMult.class);
    job.setMapperClass(TokenizerMapper.class);
    job.setCombinerClass(IntSumReducer.class);
    job.setReducerClass(IntSumReducer.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(IntWritable.class);
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}