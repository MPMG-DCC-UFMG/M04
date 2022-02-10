
FROM ubuntu:20.04

# # Setup JAVA_HOME, this is useful for docker commandlin

RUN apt-get update && apt-get install -y --no-install-recommends  openjdk-8-jdk
WORKDIR /home
COPY /M06   /home/M06 
COPY /queries /home/queries
ADD /retrieve-data.sh /home

EXPOSE 5000/tcp
EXPOSE 5001/tcp
EXPOSE 5002/tcp
EXPOSE 5003/tcp
EXPOSE 5004/tcp
EXPOSE 5005/tcp
EXPOSE 5006/tcp
EXPOSE 5007/tcp
EXPOSE 5008/tcp
EXPOSE 5009/tcp
EXPOSE 5010/tcp

RUN apt-get update
RUN apt-get install dos2unix
RUN find . -type f -print0 | xargs -0 dos2unix

ENV M06_HOME /home/M06

ENV HADOOP_HOME /home/M06/hadoop-3.1.1

ENV HADOOP_CONF_DIR /home/M06/hadoop-3.1.1/etc/hadoop/

ENV YARN_CONF_DIR /home/M06/hadoop-3.1.1/etc/hadoop/

ENV HDP_VERSION 3.1.5.0-152

ENV JAVA_HOME /usr/lib/jvm/java-8-openjdk-amd64

ENV SPARK_HOME /home/M06/spark-hw

ENV FRACTAL_HOME /home/M06/fractal

#Linhas responsaveis por compilar os programas

RUN cd ${FRACTAL_HOME} && ./gradlew assemble
RUN cd ${M06_HOME} && ./gradlew assemble