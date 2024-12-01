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
            String keyStr = String.valueOf(currRow) + "," + String.valueOf(i);
            String valStr = String.valueOf(val) + "," + String.valueOf(currRow);
            System.out.println(keyStr + "::" + valStr);
            outVal.set(valStr);
            outKey.set(keyStr);
            context.write(outKey, outVal);
          }
        } else {
          for (int i = 0; i < rows; i++) {
            String keyStr = String.valueOf(i) + "," + String.valueOf(currCol);
            String valStr = String.valueOf(val) + "," + String.valueOf(currCol);
            System.out.println(keyStr + "::" + valStr);
            outVal.set(valStr);
            outKey.set(keyStr);
            context.write(outKey, outVal);
          }
        }
      }
    }
  }

  public static class IntSumReducer
       extends Reducer<Text,Text,Text,Text> {
    private Text result = new Text();
    private Text resVal = new Text();

    public void reduce(Text key, Iterable<Text> values,
                       Context context
                       ) throws IOException, InterruptedException {
      HashMap<Integer, Integer> map = new HashMap();
      for (Text v : values) {
        System.out.println(v.toString());
        String[] arr = v.toString().split(",");
        int v2 = Integer.parseInt(arr[0]);
        int index = Integer.parseInt(arr[1]);
        int temp = map.getOrDefault(index, new Integer(1)).intValue();
        temp *= v2;
        map.put(index, temp);
      }
      
      int sum = 0;
      for (Integer v : map.values()) {
        sum += v;
      }

      resVal.set(String.valueOf(sum));
      result.set(key);
      System.out.println("resVal: " + String.valueOf(sum));
      context.write(result, resVal);
    }
  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    Job job = Job.getInstance(conf, "word count");
    job.setJarByClass(MatrixMult.class);
    job.setMapperClass(TokenizerMapper.class);
    // job.setCombinerClass(IntSumReducer.class);
    job.setReducerClass(IntSumReducer.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}