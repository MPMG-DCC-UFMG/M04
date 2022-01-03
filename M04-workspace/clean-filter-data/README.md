# Data cleaning and filtering
This module is responsible for the data cleaning and filtering part of the pipeline being developed by the M04 team of the PCA - MPMG program during the year of 2021.

It executes the following tasks:

1. Fetches the raw data from the configured Hadoop filesystem
2. Cleans the data, excluding null values, repeated values and invalid values
3. Filters the data, be it by date or by maximum bidding value (other filter may be added in the future)
4. Outputs the treated files to the defined output directory.

## Requirements for running
* Hadoop File System
* Python 3
  * Pandas
  * Numpy

## Initial setup
When first installing the module, it is necessary to set its environment variables to the correct installation directories in the given machine. The following variables must be set:
```
HADOOP_HOME: Hadoop installation directory
HADOOP_CONF_DIR: Hadoop config directory
HDP_VERSION: Hadoop version
HADOOP_USER_NAME: Hadoop username
YARN_CONF_DIR: Yarn configuration directory
M04_PATH: Path to the root of the 2021 M04 pipeline
```
## Usage
Once properly configured with the aforementioned environment variables, this module's behavior is as described initially, fetching the data from Hadoop, processing it and outputting it. The base hadoop URL, the input and output filenames and the output directory are to be specified in the config.json file, as well as the desired filters. If a given filter is not to be applied, it suffices to leave it as an empty string.
The module is used by calling the main shell script, `clean-filter-data.sh` 
```
// config.json 
{
  "baseHadoopURL": "hdfs:hadoop-base-url-here",
  "outputDirectory": "path-to-output-directory-here",
  "startDate": "20/09/2020", // if not necessary, leave as an empty string. The format should be DD/MM/YYYY
  "endDate": "20/09/2021", // if not necessary, leave as an empty string. The format should be DD/MM/YYYY
  "maxValue": "10000000", // if not necessary, leave as an empty string
  "files": [ 
  // array that lists all of the files to be fetched and processed
  // each of the entries should have the following format
    {
      "fileName": "read-database-licitacao.csv", // Filename as is in the Hadoop Filesystem
      "outputFileName": "read-database-licitacao.csv", // Desired output filename, once treated
      "columns": ["id_licitacao", "nome_orgao_show", "sigla_uf", "nome_modalidade_show", "num_modalidade", "ano_referencia", "mes_referencia", "dsc_objeto_licitacao_show", "vlr_licitacao"], 
      // Array of columns to be kept. If all columns are to be kept, leave as an empty array
      "extraTreatments": ["maxValue", "startDate", "endDate"] 
      // Array of extra treatments that can be applied to the data. If no treatment is to be applied, leave it as an empty array
    }
  ]
}
```
Currently, the `"extraTreatments"` available are:
* `"maxValue"`: drops any bidding which exceeds the stipulated maximum value
* `"startDate"`: drops any bidding which happens before the stipulated start date.
* `"endDate"`: drops any bidding which happens after the stipulated end date.
* `"validate1CNPJ"`: drops any bidding participation from a CNPJ that is deemed invalid
* `"validate2CNPJs"`: drops any bond between CNPJs in which at least one is deemed invalid