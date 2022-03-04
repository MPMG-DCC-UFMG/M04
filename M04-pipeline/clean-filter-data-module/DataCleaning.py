import os
import pandas as pd
import dask.dataframe as dd
from dask.multiprocessing import get

class DataCleaning(object):
    def __init__(self, df: pd.DataFrame, output_filename: str, output_dir: str) -> None:
        self.df = df
        self.out_file_path = self._get_out_file_path(output_filename, output_dir)

    def _get_out_file_path(self, filename: str, output_dir: str) -> str:  
        return os.path.join(output_dir, filename)

    def select_columns(self, cols: list) -> None:
        self.df = self.df.loc[:, cols]

    def remove_na(self) -> None:
        self.df = self.df.dropna()

    def remove_duplicates(self) -> None:
        self.df.drop_duplicates(inplace=True)
    
    def validate_cnpj(self, column: str) -> None:
        ddf = dd.from_pandas(self.df, npartitions=30)

        valid_cnpj = ddf[column].map(self._check_cnpj).compute()
        self.df['valid_cnpj'] = valid_cnpj
        self.df = self.df.loc[self.df['valid_cnpj'] == True]
        self.df.drop(columns=['valid_cnpj'], inplace=True)

        # self.df = self.df[self.df.apply(
        #     lambda row: self._check_cnpj(row[column]), axis=1
        # )]

    def _check_cnpj(self, cnpj: str) -> bool:
        cnpj = str(cnpj).zfill(14)
        if not str(cnpj).isdecimal() : return False

        # Até aqui descartamos CNPJs com problema de formatação. 
        # Agora verificaremos os dígitos verificadores
        # Definições iniciais
        cnpj_list = [int(char) for char in str(cnpj)]
        dig_ver_1 = cnpj_list[-2]
        dig_ver_2 = cnpj_list[-1]
        
        # Verificação do primeiro dígito
        order_weights = [5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2]
        sum = 0
        for i in range(12): sum = sum + cnpj_list[i] * order_weights[i]
        dig_1 = sum % 11

        if (dig_1 < 2) : dig_1 = 0
        else: dig_1 = 11 - dig_1
        
        if(dig_1 != dig_ver_1) : return False

        #Verificação do segundo dígito
        order_weights = [6, 5, 4, 3, 2, 9, 8, 7, 6, 5, 4, 3, 2]
        sum = 0
        for i in range(13): sum = sum + cnpj_list[i] * order_weights[i]
        dig_2 = sum % 11

        if (dig_2 < 2) : dig_2 = 0
        else: dig_2 = 11 - dig_2
        
        if(dig_2 != dig_ver_2) : return False

        return True

    def print_df(self):
        print(self.df)

    def save_file(self):
        self.df.to_csv(self.out_file_path, sep=';', index=False)