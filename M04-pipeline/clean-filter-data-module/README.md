# Data cleaning and filtering
This module is responsible for the data cleaning and filtering part of the pipeline being developed by the M04 team of the PCA - MPMG program during the year of 2021.

It executes the following tasks:

1. Fetches the raw data from the configured Hadoop filesystem
2. Cleans the data, excluding null values, repeated values and invalid values, if wanted
3. Filters the data, be it by date or by value (other filter may be added in the future)
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
The module is used by calling the main shell script, `clean-filter-data.sh`, passing as it's only argument the config file. 
```
// config.json 
{
  "baseHadoopURL": "hdfs:hadoop-base-url-here",
  "outputDirectory": "path-to-output-directory-here",
  "files": [ 
  // array that lists all of the files to be fetched and processed
  // each of the entries should have the following format
    {
      "fileName": "itens-vlr-global.csv", // Filename as is in the Hadoop Filesystem
      "outputFileName": "itens-vlr-global.csv", // Desired output filename, once treated
      "columns": [
        "id_licitacao",
        "id_item_licitacao",
        "ano_exercicio",
        "mes_licitacao",
        "sigla_uf",
        "num_lote",
        "nome_lote",
        "num_item",
        "nome_item",
        "nome_cidade",
        "vlr_global_minimo",
        "nb_of_participants"
      ], 
      // Array of columns to be kept. If all columns are to be kept, leave as an empty array
      "extraTreatments": [
        {
          "treatmentName": "removeNA"
        },
        {
          "treatmentName": "removeDuplicates"
        },
        {
          "treatmentName": "maxValue",
          "value": 1000000,
          "column": "vlr_global_minimo"
        },
        {
          "treatmentName": "minValue",
          "value": 1,
          "column": "vlr_global_minimo"
        },
        {
          "treatmentName": "startDate",
          "date": "02/02/2020",
          "columns": {
            "year": "ano_exercicio",
            "month": "mes_licitacao"
          }
        },
        {
          "treatmentName": "endDate",
          "date": "30/03/2021",
          "columns": {
            "year": "ano_exercicio",
            "month": "mes_licitacao"
          }
        }
      ] 
      // Array of extra treatments that can be applied to the data. If no treatment is to be applied, leave it as an empty array. Each treatment is an object which has the property "treatmentName" and the other properties that might be needed for that specific treatment.
    }
  ],
  "renameColumns": {
    "vlr_global_minimo": "vlr_total",
    "nb_of_participants": "num_licitantes"
  }
  // Object that maps the old column names to the new ones. If no renaming is to be applied, leave it as an empty object. Take note that the renaming of columns is done after the extra treatments.
}
```
Currently, the `"extraTreatments"` available are:
* `"removeNA"`: Removes all rows with null values. Doesn't take any parameters.
```
{
  "treatmentName": "removeNA"
}
```
* `"removeDuplicates"`: Removes all rows with repeated values. Doesn't take any parameters.
```
{
  "treatmentName": "removeDuplicates"
}
```
* `"maxValue"`: Removes all rows with values greater than the given value. Takes the value and the column as parameters.
```
{
  "treatmentName": "maxValue",
  "value": 1000000,
  "column": "vlr_global_minimo"
}
```
* `"minValue"`: Removes all rows with values less than the given value. Takes the value and the column as parameters.
```
{
  "treatmentName": "minValue",
  "value": 1,
  "column": "vlr_global_minimo"
}
```
* `"startDate"`: Removes all rows with dates before the given date. Takes the date and the columns as parameters. The date must be in the format `"DD/MM/YYYY"`. The columns are to be given in an object which should contain one key for each column. You can pass only a year, a year and a month or a year, a month and a day. If month or day are not provided, they default to 01.
```
{
  "treatmentName": "startDate",
  "date": "02/02/2020",
  "columns": {
    "year": "ano_exercicio",
    "month": "mes_licitacao",
    "day": "dia_licitacao"
  }
}
```
* `"endDate"`: Removes all rows with dates after the given date. Takes the date and the columns as parameters. The date must be in the format `"DD/MM/YYYY"`. The columns are to be given in an object which should contain one key for each column. You can pass only a year, a year and a month or a year, a month and a day. If month or day are not provided, they default to 01.
```
{
  "treatmentName": "endDate",
  "date": "30/03/2021",
  "columns": {
    "year": "ano_exercicio",
    "month": "mes_licitacao",
    "day": "dia_licitacao"
  }
}
```
* `"validateCNPJ"`: Validates the CNPJ of the given column and drops invalid values. Takes the column as parameter.
```
{
  "treatmentName": "validateCNPJ",
  "column": "cnpj_fornecedor"
}
```