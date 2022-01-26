 
# Pattern mining
This module is responsible for the pattern mining part of the pipeline being developed by the M04 team of the PCA - MPMG program during the year of 2021.

It consists of an abstract Python class which dictates the main methods that need implementing for pattern mining (i.e. ingestion, mining and exportation).

As of now, there's one concrete implementation of the module, which finds the non-unitary-cliques for a given graph. We're also currently implementing cross-graph-quasi-clique mining.

## Requirements for running
* Python 3
  * Networkx

## Initial setup
When first installing the module, it is necessary to set its environment variables to the correct installation directories in the given machine. The following variables must be set:
```
M04_PATH: Path to the root of the 2021 M04 pipeline
```
## Usage
Once properly configured with the aforementioned environment variables, this module's behavior is as described initially, running the different pattern mining algorithms, with it's different inputs, and generating it's desired outputs. The input filepaths, the output filepaths and the algorithms to be run are specified in the config file, as described below.

The config file consists of an array of objects, each one with the following properties:
* `moduleName`: specifies the pattern mining algorithms to be run. Current available options are:
  * `biddingCliques`: finds the cliques between enterprises for a given bidding.
  * `crossGraphQuasiCliques`: under development.
* `inputFile`: filepath of the input file for the algorithm
* `outputFile`: filepath of the output file for the mined patterns
```
// config.json 
[
  {
    "moduleName": "biddingCliques",
    "inputFile": "fake/file/path/Pickle/pickledata.pkl",
    "outputFile": "fake/file/path/Patterns/cliques.json"
  },
  {
    "moduleName": "crossGraphQuasiCliques",
    "inputFile": "fake/file/path/Pickle/pickledata.pkl",
    "outputFile": "fake/file/path/Patterns/cliques.json"
  }
]
```

The module is used by calling the main shell script, `mine-patterns.sh`, passing as it's only argument the config file, e.g.:

`mine-patterns.sh config.json`
