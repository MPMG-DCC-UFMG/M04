import pandas as pd

class RankByMetric(object):
    def __init__(self, metrics_info: pd.DataFrame, metric: str, asc: bool, output_path: str) -> None:
        self.ordered_biddings = self.order_by_metric(metrics_info, metric, asc)

        self.save_result(output_path)


    def order_by_metric(self, info: pd.DataFrame, metric: str, asc: bool) -> pd.DataFrame:
        ordered_biddings = info.sort_values(by=[metric], ascending=asc)
        return ordered_biddings
        
    def save_result(self, output_path: str) -> None:
        self.ordered_biddings.to_csv(output_path, index=False, sep=';')
        
