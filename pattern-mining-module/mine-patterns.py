import json
import sys

from CliquesMiner import CliquesMiner

def main():
    configs_path = sys.argv[1]
    configs = json.load(open(configs_path))

    for entry in configs:
        module_name = entry['moduleName']
        input_path = entry['inputFile']
        output_path = entry['outputFile']

        print(f'Starting the execution of the \'{module_name}\' module')

        if module_name == 'biddingCliques':
            cliquesMiner = CliquesMiner(input_path, output_path)
            cliquesMiner.pipeline()
            print(f'Finished the execution of the \'{module_name}\' module')    
        elif module_name == 'crossGraphQuasiCliques':
            #TODO
            print(f'Finished the execution of the \'{module_name}\' module')
            pass
        else:
            print(f'There\'s no \'{module_name}\' module')
main()