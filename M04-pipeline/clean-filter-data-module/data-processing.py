import pandas as pd
import json
import os
import sys
import time
from DataCleaning import DataCleaning
from DataFiltering import DataFiltering

def main():
    start = time.time()

    base_dir =  os.environ['BASE_DIR']
    input_dir = base_dir + '/raw-input'
    config_file = sys.argv[1]

    with open(config_file) as read_file:
        configs = json.load(read_file)
        total_len=0
        # Getting the configs that are common to all the files
        output_dir = configs['outputDirectory']
        files = configs['files']

        for file in files:
            # Getting the configs specific to each file
            input_filename = file['fileName']
            output_filename = file['outputFileName']
            columns = file['columns']
            extra_treatments = file["extraTreatments"]
            column_renames = file["renameColumns"]
            
            # Cleaning stage
            df = pd.read_csv(f'{input_dir}/{input_filename}', sep=';', low_memory=False)
            total_len+=len(df)
            cleaner = DataCleaning(df, output_filename, output_dir)

            # Cleaning treaments that should only happen to certain files, when specified
            if len(columns) > 0 :
                cleaner.select_columns(columns)

            for treatment in extra_treatments:
                if treatment['treatmentName'] == 'validateCNPJ':
                    cleaner.validate_cnpj(treatment['column'])
                elif treatment['treatmentName'] == 'removeNA':
                    cleaner.remove_na()
                elif treatment['treatmentName'] == 'removeDuplicates':
                    cleaner.remove_duplicates()

            # Filtering stage        
            filterer = DataFiltering(cleaner.df, output_filename, output_dir)

            for treatment in extra_treatments:
                if treatment['treatmentName'] == 'maxValue':
                    filterer.max_value(treatment['column'], treatment['value'])
                elif treatment['treatmentName'] == 'minValue':
                    filterer.min_value(treatment['column'], treatment['value'])
                elif treatment['treatmentName'] == 'startDate':
                    filterer.after_date(treatment['columns'], treatment['date'])
                elif treatment['treatmentName'] == 'endDate':
                    filterer.before_date(treatment['columns'], treatment['date'])

            # Renaming columns
            for old_name, new_name in column_renames.items():
                filterer.rename_column(old_name, new_name)
            
            # Saving the resulting file
            filterer.save_file()
    end = time.time()
    print("Execution time: " ,end - start)
    print("Numbers of rows processed: " ,total_len)

main()