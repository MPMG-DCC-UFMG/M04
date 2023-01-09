import pandas as pd

class RankCnpj(object):
    def __init__(self, link_data_df: pd.DataFrame, bidding_info_df: pd.DataFrame, output_path: str,configs) -> None:
        self.graphId=  configs["columnNames"]["graphId"]
        self.graphValues = configs["columnNames"]["graphValue"]
        link_data_dic = self._get_link_data(link_data_df,configs)
        biddings_dic = self._get_bidding_values(bidding_info_df,configs)
        rank_cnpj = self.rank_cnpj(link_data_dic, biddings_dic, 1,configs)
    
        # Sort by alarm value
        sort_rank_cnpj =  {k: v for k, v in sorted(
            rank_cnpj.items(), key = lambda x: x[1]['alarm'], reverse=True)}
        
        # Save result
        with open(output_path, 'w') as f:
            f.write('cnpj;participations;alarm\n')
            for cnpj in sort_rank_cnpj:
                f.write('{};{};{}\n'.format(
                    cnpj,
                    #sorts itens based on alarm , then returns list of just itens
                    [x for (x,y) in (
                    sorted(
                        [(x,sort_rank_cnpj[cnpj]['participations'][x]) for x in sort_rank_cnpj[cnpj]['participations'] ], 
                        key = lambda x: x[1]))],
                    sort_rank_cnpj[cnpj]['alarm']))

        print('end')
    
    #parse information about bonds
    def _get_link_data(self, link_data: pd.DataFrame,configs) -> dict:
        link_data_dic = {}
        for index, row in link_data.iterrows():
            link_data_dic[index] = {
                'cnpj1': row['cnpj1'],
                'cnpj2': row['cnpj2'],
                'bidding': row['id_licitacao'],
                'weight': row['peso']
            }
        
        return link_data_dic
    #parse information about nodes values
    def _get_bidding_values(self, biddings_info: pd.DataFrame,configs) -> dict:
        bidding_dic = {}
        
        for index, row in biddings_info.iterrows():
            bidding_dic[row[self.graphId]] = float(row[self.graphValues]) or 0
        
        return bidding_dic

    #read all bonds and for every pair of cnpj1 cnpj2 
    def rank_cnpj(self, links: dict, bidding_values: dict, weight: float,configs) -> dict:
        # CNPJs ranked by alarm level,
      
        rank_cnpj_dic = {}
        new_links = {k: v for k, v in links.items() }
        for reg_num in new_links:
            reg = links[reg_num]

            if reg['bidding'] in bidding_values:
                alarm = bidding_values[reg['bidding']] * reg['weight']
            else:
                # print('Bidding {} not in bidding values table'
                #       .format(reg['bidding']))
                alarm = 0
            self.update_cnpj(reg['cnpj1'], alarm, reg['bidding'], rank_cnpj_dic)
            self.update_cnpj(reg['cnpj2'], alarm, reg['bidding'], rank_cnpj_dic)
            
        return rank_cnpj_dic    
    
    def update_cnpj(self, cnpj: str, alarm: float, bidding: str, rank_cnpj_dic: dict) -> None:
        
        if not cnpj in rank_cnpj_dic:
            rank_cnpj_dic[cnpj] = {
               'alarm': alarm, 'participations': {bidding: alarm}}
        else:
            #only adds the alarm of a bidding once 
            if bidding not in rank_cnpj_dic[cnpj][ 'participations']:
                rank_cnpj_dic[cnpj]['alarm'] += alarm
                rank_cnpj_dic[cnpj]['participations'][bidding] = alarm