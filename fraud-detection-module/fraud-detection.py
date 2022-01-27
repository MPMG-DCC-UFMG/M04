import sys
import json
import pandas as pd

from RankCnpj import RankCnpj

def rankCnpj(input_paths: dict, output_path: str) -> None:
    # Read data
    links_info_path = input_paths['linksInfo']
    bidding_info_path = input_paths['biddingInfo']

    link_data_df = pd.read_csv(links_info_path, sep=';')
    bidding_info_df = pd.read_csv(bidding_info_path, sep=';')

    # Run the rankCnpj pipeline
    RankCnpj(link_data_df, bidding_info_df, output_path)

def main():
    configs_path = sys.argv[1]
    configs = json.load(open(configs_path))

    for method in configs:
        fraud_detection_method = method['fraudDetectionMethod']
        input_paths = method['inputFilepaths']
        output_path = method['outputFilepath']

        print(f'Starting the execution of the \'{fraud_detection_method}\' method')

        if fraud_detection_method == 'rankCnpj':
            rankCnpj(input_paths, output_path)
            
        print(f'Finished the execution of the \'{fraud_detection_method}\' method')

main()