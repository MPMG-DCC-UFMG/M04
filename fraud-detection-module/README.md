 
# Pattern mining
This module is responsible for the fraud detection part of the pipeline being developed by the M04 team of the PCA - MPMG program during the years of 2021 and 2022. It tries to detect frauds by generating multiple rankings, each one highlighting biddings and CNPJs that deserve further looking into, because they show unexpected behaviors.

It consists of a script that instantiates different classes, each one corresponding to one of the ranking systems. Each class should be able to handle its data ingestion, its data treatment and its data exporting procedures.

As of now, there's one implemented class, which ranks CNPJs based on their participations on biddings and gives them an alarm number, which is based on the biddings it took part in and on the value of those biddings. This class will be explained further down the document.

## Requirements for running
* Python 3
  * Pandas

## Usage
Once properly downloaded, this module's behavior is as described initially, running the different ranking algorithms, with it's different inputs, and generating it's desired outputs. The input filepaths, the output filepath and the algorithms to be run are specified in the config file, as described below.

The config file consists of an array of objects, each one with the following properties:
* `fraudDetectionMethod`: specifies the fraud detection algorithms to be run. Current available options are:
  * `rankCnpj`: assigns an alert value to each CNPJ based on the values of the biddings it took part on.
  * `rankCrossGraphQuasiCliques`: under development.
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
    "fraudDetectionMethod": "rankCrossGraphQuasiCliques",
    "inputFilepaths": {
      "biddingInfo": "/fake/file/path/biddingInfo.csv",
      "linksInfo": "/fake/file/path/linksInfo.csv"
    },
    "outputFilepath": "/fake/file/path/output2.csv"
  }
]
```
The module is used by calling the main shell script, `fraud-detection.sh`, passing as it's only argument the config file, e.g.:

`fraud-detection.sh config.json`
