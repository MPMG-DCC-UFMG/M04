import pandas as pd
import json
import os
import sys

from DataCleaning import DataCleaning
from DataFiltering import DataFiltering

def main():
    base_dir = os.environ['M04_PATH'] + '/clean-filter-data'
    input_dir = base_dir + '/raw-input'
    config_file = sys.argv[1]

    with open(config_file) as read_file:
        configs = json.load(read_file)

        # Getting the configs that are common to all the files
        output_dir = configs['outputDirectory']
        files = configs['files']
        start_date = configs.get('startDate')
        end_date = configs.get('endDate')
        max_value = configs.get('maxValue')

        for file in files:
            # Getting the configs specific to each file
            input_filename = file['fileName']
            output_filename = file['outputFileName']
            columns = file['columns']
            extra_treatments = file["extraTreatments"]

            # Cleaning stage
            df = pd.read_csv(f'{input_dir}/{input_filename}', sep=';')
            cleaner = DataCleaning(df, output_filename, output_dir)

            # Cleaning treaments that should only happen to certain files, when specified
            if len(columns) > 0 :
                cleaner.select_columns(columns)
            if len(extra_treatments) > 0 :
                if 'validate1CNPJ' in extra_treatments: cleaner.remove_invalid_cnpj_bidding()
                if 'validate2CNPJs' in extra_treatments: cleaner.remove_invalid_cnpj_bond()

            # Cleaning treatments that should happen to every file
            cleaner.remove_na()

            # Filtering stage        
            filterer = DataFiltering(cleaner.df, output_filename, output_dir)

            if len(extra_treatments) > 0 :
                # Filtering by value
                if 'maxValue' in extra_treatments and bool(max_value):
                    filterer.remove_exceeding_value_biddings(max_value)
            
                # Filtering by period
                if 'startDate' in extra_treatments or 'endDate' in extra_treatments:
                    filterer.select_from_period(start_date, end_date)
            
            # Saving the resulting file
            filterer.save_file()
            print(f'Acabou o {input_filename}')