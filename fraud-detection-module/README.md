 
# Fraud detection
This module is responsible for the fraud detection part of the pipeline being developed by the M04 team of the PCA - MPMG program during the years of 2021 and 2022. It tries to detect frauds by generating multiple rankings, each one highlighting biddings and CNPJs that deserve further looking into, because they show unexpected behaviors.

It consists of a script that instantiates different classes, each one corresponding to one of the ranking systems. Each class should be able to handle its data ingestion, its data treatment and its data exporting procedures.

As of now, there are two implemented classes. the first one ranks CNPJs based on their participations on biddings and gives them an alarm number, which is based on the biddings it took part in and on the value of those biddings. The second one ranks the different biddings by one of the metrics determined in the previous module. They will both be explained further down the document.

## Requirements for running
* Python 3
  * Pandas

## Usage
Once properly downloaded, this module's behavior is as described initially, running the different ranking algorithms, with it's different inputs, and generating it's desired outputs. The input filepaths, the output filepath and the algorithms to be run are specified in the config file, as described below.

The config file consists of an array of objects, each one with at least the following properties:
* `fraudDetectionMethod`: specifies the fraud detection algorithms to be run. Current available options are:
  * `rankCnpj`: assigns an alert value to each CNPJ based on the values of the biddings it took part on.
  * `rankByMetric`: returns all the calculated metrics for a given bidding, ordering by a chosen metric either in ascending or descending order.
* `inputFilepaths`: this is an object which takes as its keys the type of info of the input and as the values the filepaths of said files
* `outputFilepath`: filepath of the output file for the ranked entities
```
// config.json 
[
  {
    "fraudDetectionMethod": "rankCnpj",
    "inputFilepaths": {
      "biddingInfo": "/fake/file/path/biddingInfo.csv",
      "linksInfo": "/fake/file/path/linksInfo.csv"
    },
    "outputFilepath": "/fake/file/path/output.csv"
  },
  {
    "fraudDetectionMethod": "rankByMetric",
    "metric": "competition",
    "ascending": false,
    "inputFilepaths": {
      "metricsInfo": "/fake/file/path/metrics.csv"
    },
    "outputFilepath": "/fake/file/path/output2.csv"
  }
]
```
The module is used by calling the main shell script, `fraud-detection.sh`, passing as it's only argument the config file, e.g.:

`fraud-detection.sh config.json`
## Available classes
### RankCnpj
To execute the pipeline of this class, it is necessary to pass it two CSV files. The first one regards the general information about the biddings, and the second one the information about the links CNPJs that partake on the biddings, with each link having a weight.
They should be as follows:

* `biddingInfo`: should contain one column named `id_licitacao`, which is the id of the bidding, and another column named `vlr_licitacao`, which is the value of said bidding. It could have other columns, but they will be ignored. The file should be in CSV format.
* `linksInfo`: should contain one column named `id_licitacao`, which is the id of the bidding, one column namesd `cnpj1` and one named `cnpj2`, which represent both enterprises which have a link, and a column called `peso`, which represents the weight of the bond. The file should be in CSV format.

This algorithm assigns an alert value to each CNPJ based on the values of the biddings it took part on and ranks them according to this alert value.

The output is a CSV with the following columns:
* `cnpj`: CNPJ that is under analysis
* `participations`: list of biddings it took part on
* `alert`: alert value

### RankByMetric
To execute the pipeline of this class, it is necessary to pass it one CSV file, with all of the metrics calculated for each bidding.
Besides, the configuration of this class also takes the following parameters:
* `metric`: the metric to be used to rank the biddings. Current available options are:
  * `density`
  * `competition`
  * `nbOfNodes`
  * `nbOfEdges`
  * `nbOfCliques`
  * `sizeOfLargestClique`
* `ascending`: if true, the biddings will be ranked in ascending order according to the chosen metric, otherwise, descending order.

The only input the module requires is the filepath of the metrics file, which should be in CSV format:
* `metricsInfo`: path to the metrics file

