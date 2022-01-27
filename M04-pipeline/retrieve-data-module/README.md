# Data fetching
This module is responsible for the data-fething part of the pipeline being developed by the M04 team of the PCA - MPMG program during the year of 2021.

It runs a SQL query against a SQL database and generates a CSV with the output. This CSV will be in a pre-configured Hadoop File System.

It is based on the Read-Database module of the [Fractal](https://github.com/MPMG-DCC-UFMG/M04/tree/master/fractal) program.

## Requirements for running
* OpenJDK 8
* Spark 2.x.x
* Hadoop File System
* Fractal

## Initial setup
When first installing the module, it is necessary to set its environment variables to the correct installation directories in the given machine. The following variables must be set:
```
JAVA_HOME: Java JDK directory
SPARK_HOME: Spark directory
SPARK_CONF_DIR: Spark config directory
HADOOP_HOME: Hadoop installation directory
HADOOP_CONF_DIR: Hadoop config directory
HDP_VERSION: Hadoop version
HADOOP_USER_NAME: Hadoop username
YARN_CONF_DIR: Yarn configuration directory
FRACTAL_PATH: Path to the directory in which the main script that summons Fractal, i.e. fractal-mpmg.sh lies in
M04_PATH: Path to the root of the 2021 M04 pipeline
```
## Usage
Once properly configured with the aforementioned environment variables, this module's behavior is to execute each one of the queries specified in the config files that will lie in the "queries" folder.
One such config file is exemplified below:
```
{
  "spark_fractal": [
    {
      "name": "spark.master",
      "value": "local[*]"
    },
    { 
      "name": "spark.driver.host",
      "value": "10.21.0.127"
    },
    { 
      "name": "spark.driver.memory",
      "value": "200g"
    },
    { 
      "name": "spark.executor.cores",
      "value": "5"
    },
    { 
      "name": "spark.executor.instances",
      "value": "4"
    },
    { 
      "name": "spark.executor.memory",
      "value": "40g"
    }
  ],
  "spark_database": [
    {
      "name": "spark.master",
      "value": "yarn"
    },
    {
      "name": "spark.driver.host",
      "value": "10.21.0.127"
    },
    {
      "name": "spark.executor.memory",
      "value": "6g"
    },
    {
      "name": "spark.hadoop.hive.llap.daemon.service.hosts",
      "value": "@llap0"
    },
    {
      "name": "spark.sql.hive.hiveserver2.jdbc.url",
      "value": "jdbc:hive2://hadoopmn-gsi-prod01.mpmg.mp.br:2181,hadoopmn-gsi-prod02.mpmg.mp.br:2181,hadoopmn-gsi-prod03.mpmg.mp.br:2181/dataset;serviceDiscoveryMode=zooKeeper;zooKeeperNamespace=hiveserver2;AuthMech=3;password=PASSWORD_HERE;user=USER_HERE"
    },
    {
      "name": "spark.datasource.hive.warehouse.exec.results.max",
      "value": "2147483647"
    }
  ],
  "app": {
    "name" : "read_database",
    "query": "SELECT id_licitacao, num_cpf_cnpj_show FROM dataset_v2.sicom_20210109_licitacao_habilitado h WHERE h.cod_tipo_pessoa = 'J'",
    "output_path": "hdfs://hadoopgsiha/dados-fractal/read-database-cnpjs-por-licitacao.csv",
    "delimiter" : ";",
    "show_header": "true"	
  }
}
```
The most relevant part and the one that will vary is the "app" values:
- `"name"` should never change for this module
- `"query"` is the SQL query to be run against the database
- `"output_path"` is the hdfs destination path
- `"delimiter"` is the delimiter to be used in the CSV output file
- `"show_header"` determines whether the output CSV should contain the header of each column or not.