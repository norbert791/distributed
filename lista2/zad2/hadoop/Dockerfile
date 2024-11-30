FROM apache/hadoop:3.4.1


ENV HADOOP_CLASSPATH=${JAVA_HOME}/lib/tools.jar

RUN mkdir input
RUN cp etc/hadoop/*.xml input
RUN bin/hadoop jar share/hadoop/mapreduce/hadoop-mapreduce-examples-3.4.1.jar grep input output 'dfs[a-z.]+'
RUN cat output/*

COPY Main.java Main.java
COPY input/file1.txt /user/norbert/input/file1.txt
COPY input/file2.txt /user/norbert/input/file2.txt

RUN bin/hadoop com.sun.tools.javac.Main WordCount.java
RUN jar cf wc.jar WordCount*.class

CMD ["bin/hadoop jar wc.jar WordCount /user/norbert/input /user/norbert/output"]