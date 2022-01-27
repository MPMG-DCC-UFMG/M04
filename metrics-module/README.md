# Metrics generation
This module is responsible for the metrics generation part of the pipeline being developed by the M04 team of the PCA - MPMG program during the years of 2021 and 2022.

It consists of a script which takes multiple inputs (raw data about the biddings, modeled graphs and mined cliques, at the moment) and outputs a series of quality metrics related to the given bidding.

## Requirements for running
* Python 3
  * Networkx
  * Pandas

## Usage
Once properly downloaded , this module's behavior is as described initially, generating the different desired metrics. The input filepaths, the output filepath and the metrics to be generated are specified in the config file, as described below.

The config file consists of an object with the following keys:
* `inputFiles`: this is itself an object, which contains the filepaths for the different input files. Currently, there are 3 possible attributes:
  * `generalData`: path to the csv file which contains general information about the bidding
  * `graphData`: path to the graph data file, which contains one graph for each bidding
  * `cliquesData`: path to the cliques data json file, which contains one entry for each bidding which has cliques. If so, the value is a list of lists, each list corresponding to a clique
* `outputFile`: filepath of the output file for the 
generated metrics
* `desiredMetrics`: this is a list of objects, each one corresponding to one desired metric. Each object has the following attributes:
  * `metric`: corresponds to the name of the desired metric
  * `requiredInputs`: corresponds to an array of the inputs necessary to generate the given metric. The names must exactly match those in the `inputFiles` keys
```
// config.json 
{
  "inputFiles": {
    "generalData": "path/to/file.csv",
    "graphData": "path/to/file.pkl",
    "cliquesData": "path/to/file.json"
  },
  "outputFile": "path/to/output_file.csv",
  "desiredMetrics": [
    {
      "metric": "density",
      "requiredInputs": [
        "graphData"
      ]
    },
    {
      "metric": "competition",
      "requiredInputs": [
        "graphData"
      ]
    },
    {
      "metric": "nbOfNodes",
      "requiredInputs": [
        "graphData"
      ]
    },
    {
      "metric": "nbOfEdges",
      "requiredInputs": [
        "graphData"
      ]
    },
    {
      "metric": "nbOfCliques",
      "requiredInputs": [
        "cliquesData"
      ]
    },
    {
      "metric": "sizeOfLargestClique",
      "requiredInputs": [
        "cliquesData"
      ]
    }
  ]
}
```
Currently, the available metrics, along with its input dependencies, are:
* `density`: graphData
* `competition`: graphData
* `nbOfNodes`: graphData
* `nbOfEdges`: graphData
* `nbOfCliques`: cliquesData
* `sizeOfLargestClique`: cliquesData

The module is used by calling the main shell script, `get-metrics.sh`, passing as it's only argument the config file, e.g.:

`get-metrics.sh config.json`
