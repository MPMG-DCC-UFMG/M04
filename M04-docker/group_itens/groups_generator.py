import json
from time import process_time
import pandas as pd
from pyhive import hive

class ItemHashCode:

    def __init__(self, bidding_id: str):
        self._bidding_id = bidding_id
        self._hash_code = ''.join([bidding_id, '_'])
        self._bidder = set()

    def add_bidder(self, bidder: int):
        self._bidder.add(bidder)
        self._hash_code = ''.join([self._hash_code, str(bidder)])

    @property
    def bidding_id(self) -> str:
        return self._bidding_id

    @property
    def hash_code(self) -> str:
        return self._hash_code

class Group:

    def __init__(self, group_id: int, item_hash_code: ItemHashCode):
        self._group_id = group_id
        self._item_hash_code = item_hash_code
        self._itens_list = set()

    def add_item(self, item: int):
        self._itens_list.add(item)

    @property
    def group_id(self) -> int:
        return self._group_id

    @property
    def itens(self) -> set:
        return self._itens_list

    @property
    def hash_code(self) -> ItemHashCode:
        return self._item_hash_code

    def __str__(self):
        group_str = ''.join(['GROUP ID: ', str(self._group_id), ' ==> ITEMS: '])
        group_str += str(self._itens_list)
        return group_str

class GroupItens:

    def __init__(self, config_file: str):
        self.config = self.__readConfigFile(config_file)
        self.__group_id = 1

    def __readConfigFile(self, config_file: str) -> dict:
        with open(config_file) as json_file:
           return json.load(json_file)

    def execute(self):
        hive_conn = self.__get_hive_conn()
        years = self.config["years"]
        for year in years:
            dict_group = self.__make_group(hive_conn, year, 1)
            self.__group_to_file(dict_group, year, 1)

            dict_group = self.__make_group(hive_conn, year, 2)
            self.__group_to_file(dict_group, year, 2)

    def __get_hive_conn(self) -> hive.Connection:
        database = self.config["hiveCredentials"]["database"]
        host = self.config["hiveCredentials"]["host"]
        port = self.config["hiveCredentials"]["port"]
        user = self.config["hiveCredentials"]["user"]
        password = self.config["hiveCredentials"]["password"]

        conn = hive.Connection(host=host, port=port, username=user, password=password, database=database,
                               auth='CUSTOM')
        return conn

    def __make_group(self, conn: hive.Connection, year: int, half_year: int) -> dict:
        df = self.__get_data_frame_to_process(conn, year, half_year)
        #df = pd.read_csv('E:\\__MPMG\\M04\\data\\bidding_data.csv', sep=';')
        if df.size == 0:
            return df

        dict_group = dict()
        current_item = df.iloc[0]['id_item_licitacao']
        current_bidding = df.iloc[0]['id_licitacao']
        end_row = {'id_licitacao': '0', 'id_item_licitacao': '0', 'num_cpf_cnpj_licitante': '0'}
        df = df.append(end_row, ignore_index=True)

        current_item_hash_code = ItemHashCode(current_bidding)

        for index, bidding_id, item_id, num_cpf_cnpj_licitante in df.itertuples():
            if item_id != current_item:
                group = dict_group.get(current_item_hash_code.hash_code)
                if group == None:
                    new_group = Group(self.__group_id, current_item_hash_code)
                    new_group.add_item(current_item)
                    dict_group[current_item_hash_code.hash_code] = new_group
                    self.__group_id += 1
                else:
                    group.add_item(current_item)

                current_item_hash_code = ItemHashCode(bidding_id)
                current_item = item_id

            current_item_hash_code.add_bidder(num_cpf_cnpj_licitante)

        return dict_group


    def __group_to_file(self, dict_group: dict, year: int, half_year: int):
        if len(dict_group) == 0:
            return

        output_file_name = ''.join([self.config['outputDirectory'], str(year), str(half_year), '_group.csv'])
        with open(output_file_name, 'w') as output_file:
            print("KEY;GROUP;BIDDING_ID;ITEM", file=output_file)
            for key, value in dict_group.items():
                for item in value.itens:
                    print("{};{};{};{}".format(key, value.group_id,
                                                value.hash_code.bidding_id, item), file=output_file)

    def __get_data_frame_to_process(self, conn: hive.Connection, year: int, half_year: int) -> pd.DataFrame:
        start_time = process_time()
        print('LOADING DATA BIDDING: {}/{}'.format(year, half_year))

        start_month = 1
        end_month = 6
        if half_year == 2:
            start_month = 7
            end_month = 12

        command = 'SELECT id_licitacao, id_item_licitacao, num_cpf_cnpj_licitante FROM M04_ITENS_LOTE'
        command = ' '.join([command, 'WHERE ano_exercicio = '])
        command = ' '.join([command, str(year)])
        command = ' '.join([command, ' AND mes_licitacao >= '])
        command = ' '.join([command, str(start_month)])
        command = ' '.join([command, ' AND mes_licitacao <= '])
        command = ' '.join([command, str(end_month)])
        command = ' '.join([command, 'ORDER BY id_licitacao, id_item_licitacao, num_cpf_cnpj_licitante'])

        df = pd.read_sql(command, conn)

        end_time = process_time()
        print('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<')
        print('TOTAL TIME : {}'.format(end_time - start_time))
        print('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n')

        return df

def main(config_file: str):
    group_itens = GroupItens(config_file)
    group_itens.execute()

if __name__ == '__main__':
    import sys
    config_file = sys.argv[1]
    main(config_file)
    #main('config.json')

