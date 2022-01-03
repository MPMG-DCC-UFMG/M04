import pandas as pd
import os
from datetime import date

class DataFiltering(object):
    def __init__(self, df: pd.DataFrame, output_filename: str, output_dir: str) -> None:
        self.df = df
        self.out_file_path = self._get_out_file_path(output_filename, output_dir)

    def _get_out_file_path(self, filename: str, output_dir: str) -> str:  
        return os.path.join(output_dir, filename)

    def select_from_period(self, start_date: str, end_date: str) -> None:
        self.df = self.df[self.df.apply(lambda row: self._in_given_period(start_date, end_date, f'01/{row.mes_referencia}/{row.ano_referencia}'), axis=1)]

    def remove_exceeding_value_biddings(self, max_value: str) -> None:
        self.df.drop(self.df[self.df.vlr_licitacao > int(max_value)].index, inplace=True)
        pass

    # Helper functions that check whether a bidding has happened during a given
    # period of time
    def _get_date_obj(self, date_string) -> date:
        [day, month, year] = date_string.split('/')
        formatted_date = date(year=int(year), month=int(month), day=int(day))
        return formatted_date

    def _in_given_period(self, start_date: str, end_date: str, bidding_date: str) -> bool:
        bidding_date = self._get_date_obj(bidding_date)

        if(bool(start_date)):
            start_date = self._get_date_obj(start_date)
            if bidding_date < start_date : return False

        if(bool(end_date)):
            end_date = self._get_date_obj(end_date)
            if bidding_date > end_date : return False

        return True

    def save_file(self):
        self.df.to_csv(self.out_file_path, sep=';', index=False)