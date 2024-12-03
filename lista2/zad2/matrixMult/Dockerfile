# Use an appropriate base image
FROM openjdk:8-jdk

# Set environment variables for Hadoop
ENV HADOOP_VERSION 3.4.1
ENV HADOOP_HOME /usr/local/hadoop
ENV PATH $HADOOP_HOME/bin:$PATH

# Install Hadoop
RUN apt-get update && \
    apt-get install -y curl && \
    curl -O https://downloads.apache.org/hadoop/common/hadoop-$HADOOP_VERSION/hadoop-$HADOOP_VERSION.tar.gz && \
    tar -xvzf hadoop-$HADOOP_VERSION.tar.gz && \
    mv hadoop-$HADOOP_VERSION $HADOOP_HOME && \
    rm hadoop-$HADOOP_VERSION.tar.gz

ENV HADOOP_CLASSPATH=${JAVA_HOME}/lib/tools.jar

RUN mkdir input
RUN cp $HADOOP_HOME/etc/hadoop/*.xml input
RUN $HADOOP_HOME/bin/hadoop jar $HADOOP_HOME/share/hadoop/mapreduce/hadoop-mapreduce-examples-3.4.1.jar grep input output 'dfs[a-z.]+'
RUN cat output/*

COPY MatrixMult.java MatrixMult.java
COPY input/A_2_2.txt /user/norbert/input/A_2_2.txt
COPY input/B_2_2.txt /user/norbert/input/B_2_2.txt


RUN $HADOOP_HOME/bin/hadoop com.sun.tools.javac.Main MatrixMult.java
RUN jar cf wc.jar MatrixMult*.class

CMD ${HADOOP_HOME}/bin/hadoop jar wc.jar MatrixMult /user/norbert/input /user/norbert/output && cat /user/norbert/output/*

# Start Hadoop and expose necessary ports
# EXPOSE 9870 8088
