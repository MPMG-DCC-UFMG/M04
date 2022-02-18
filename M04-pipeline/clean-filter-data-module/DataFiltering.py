import pandas as pd
import os
from datetime import date

class DataFiltering(object):
    def __init__(self, df: pd.DataFrame, output_filename: str, output_dir: str) -> None:
        self.df = df
        self.out_file_path = self._get_out_file_path(output_filename, output_dir)

    def _get_out_file_path(self, filename: str, output_dir: str) -> str:  
        return os.path.join(output_dir, filename)

    def max_value(self, column: str, value: str):
        self.df.drop(self.df[self.df[column] > int(value)].index, inplace=True)

    def min_value(self, column: str, value: str):
        self.df.drop(self.df[self.df[column] < int(value)].index, inplace=True)

    def select_from_period(self, date_columns: dict, start_date: str, end_date: str) -> None:
        def within_period(row):

            day = row[date_columns['day']] if 'day' in date_columns else '01'
            month = row[date_columns['month']] if 'month' in date_columns else '01'
            year = row[date_columns['year']] if 'year' in date_columns else '1900'

            ref_date = f'{day}/{month}/{year}'
            return self._in_given_period(start_date, end_date, ref_date)

        self.df = self.df[self.df.apply(
            lambda row: within_period(row),
            axis=1
        )]

    def after_date(self, date_columns: dict, start_date: str) -> None:
        self.select_from_period(date_columns, start_date, None)

    def before_date(self, date_columns: dict, end_date: str) -> None:
        self.select_from_period(date_columns, None, end_date)

    def rename_column(self, old_name: str, new_name: str):
        self.df.rename(columns={old_name: new_name}, inplace=True)

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