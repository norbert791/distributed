import java.io.IOException;
import java.util.StringTokenizer;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.DoubleWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

public class Weather {

  public static class TokenizerMapper
       extends Mapper<Object, Text, Text, DoubleWritable>{

    private Text word = new Text();

    public void map(Object key, Text value, Context context
                    ) throws IOException, InterruptedException {
      StringTokenizer itr = new StringTokenizer(value.toString(), ",");

      String month = "";
      DoubleWritable outVal = new DoubleWritable();
      int counter = 0;
      while (itr.hasMoreTokens()) {
        String token = itr.nextToken();
        if (counter == 12) {
          counter = 0;
          continue;
        }
        if (counter == 0) {
          // System.out.println(token);
          month = token.substring(5,7);
        }
        if (counter == 3) {
          double val = Double.parseDouble(token);
          word.set(month);
          outVal.set(val);
          // System.out.println(word + "::");
          context.write(word, outVal);
        }
        counter++;
      }
    }
  }

  public static class IntSumReducer
       extends Reducer<Text,DoubleWritable,Text,Text> {
    private Text result = new Text();

    public void reduce(Text key, Iterable<DoubleWritable> values,
                       Context context
                       ) throws IOException, InterruptedException {
      double sum = 0;
      int num = 0;
      double max = 0;
      double min = Double.MAX_VALUE;;
      for (DoubleWritable val : values) {
        sum += val.get();
        max = Math.max(max, val.get());
        min = Math.min(min, val.get());
        num++;
      }
      result.set(String.valueOf(max)+"," + String.valueOf(min)+","+String.valueOf(sum/(double)num));
      context.write(key, result);
    }
  }

  public static void main(String[] args) throws Exception {
    Configuration conf = new Configuration();
    Job job = Job.getInstance(conf, "word count");
    job.setJarByClass(Weather.class);
    job.setMapperClass(TokenizerMapper.class);
    // job.setCombinerClass(IntSumReducer.class);
    job.setReducerClass(IntSumReducer.class);
    
    job.setMapOutputKeyClass(Text.class);
    job.setMapOutputValueClass(DoubleWritable.class);
    job.setOutputKeyClass(Text.class);
    job.setOutputValueClass(Text.class);
    FileInputFormat.addInputPath(job, new Path(args[0]));
    FileOutputFormat.setOutputPath(job, new Path(args[1]));
    System.exit(job.waitForCompletion(true) ? 0 : 1);
  }
}