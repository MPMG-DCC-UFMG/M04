import json
from time import process_time
import gc
from pyhive import hive
import pandas as pd
import os

class DataFromHive:

    def __init__(self, config_file: str):
        self.config = self.__readConfigFile(config_file)

    def __readConfigFile(self, config_file: str) -> dict:
        with open(config_file) as json_file:
           return json.load(json_file)

    def execute(self):
        database = self.config["hiveCredentials"]["database"]
        host = self.config["hiveCredentials"]["host"]
        port = self.config["hiveCredentials"]["port"]
        user = self.config["hiveCredentials"]["user"]
        password = self.config["hiveCredentials"]["password"]

        hive_conn = self.__get_hive_conn(host, port, user, password, database)

        for file in self.config["simpleCommand"]:
            sql_command = self.__make_command(file)
            self.__make_file(hive_conn, sql_command, file['outputFileName'])

        for complex_command in self.config["complexCommand"]:
            sql_command = complex_command['sql_command']
            self.__make_file(hive_conn, sql_command, complex_command['outputFileName'])

        for batch_command in self.config["batchCommand"]:
            sql_command = self.__make_command(batch_command)
            rowCounts = self.__rowCountTable(hive_conn, batch_command["databaseName"], batch_command["tableName"])
            self.__make_file_step_by_step(hive_conn, sql_command, batch_command['outputFileName'],
                                           batch_command['stepSize'], rowCounts)

    def __rowCountTable(self, conn: hive.Connection, database: str, table: str) -> int:
        sourceData = '.'.join([database, table])
        command = ' '.join(['SELECT COUNT(*) AS size FROM', sourceData])
        print(command)
        df = pd.read_sql(command, conn)
        print(df)
        return df['size'][0]

    def __make_command(self, dict_file: dict) -> str:
        collumns = list(map(str, dict_file["columns"]))
        collumns = ', '.join(collumns)
        command = ' '.join(['SELECT', collumns, 'FROM', dict_file["databaseName"]])
        command = '.'.join([command, dict_file ["tableName"]])
        return command

    def __get_hive_conn(self, host: str, port: int, user: str, password: str, database: str) -> hive.Connection:
        conn = hive.Connection(host=host, port=port, username=user, password=password, database=database,
                               auth='CUSTOM')
        return conn

    def __make_file(self, conn: hive.Connection, sql_command: str, file_name: str):
        start_time = process_time()
        df = pd.read_sql(sql_command, conn)
        print(df.head(10))
        df.to_csv(self.config['outputDirectory'] + file_name, sep=';', index=False)

        del df
        gc.collect()

        end_time = process_time()
        print ( '\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<' )
        print ( 'TOTAL TIME TO MAKE FILE {}: {}'.format (file_name.upper(),  end_time - start_time ) )
        print ( '>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<' )

    def __make_file_step_by_step(self, conn: hive.Connection, sql_command: str,
                               file_name: str, rows_per_file: int, total_rows: int):
        current_value = 0
        file_id = 0
        while current_value < (total_rows + rows_per_file):
            file_id += 1
            current_statement = sql_command + ' limit ' + str(rows_per_file) + ' offset ' + str(current_value)
            current_value += rows_per_file
            start_time = process_time()
            df = pd.read_sql(current_statement, conn)
            print(df.head(10))
            df.to_csv(self.config['outputDirectory'] + file_name + '_' + str(file_id) + '.csv', sep=';', index=False)

            del df
            gc.collect()

            end_time = process_time()
            print('\n>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<')
            print('TOTAL TIME TO MAKE FILE {}: {}'.format(file_name.upper() + '_' + str(file_id) + '.csv',  end_time - start_time))
            print('>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><<<<<<<<<>>>>><<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<')

        self.__tuning_data_files(file_name, file_id)

    def __tuning_data_files(self, file_name: str, number_of_files: int):
        os.chdir(self.config['outputDirectory'])
        #os.system("cp "+file_name+"_1.csv " + file_name)
        #os.system("cp participacoes_1.csv participacoes-completo.csv")
        for i in range(2, number_of_files):
            os.system("sed -i '1d' " + file_name + "_" + str(i) + ".csv")

        cat_command = "cat "
        for i in range(1, number_of_files):
            cat_command += file_name + "_" + str(i) + ".csv "
        cat_command = cat_command + " > " + file_name
        os.system(cat_command)

def main(config_file: str):
    data_from_hive = DataFromHive(config_file)
    data_from_hive.execute()

if __name__ == '__main__':
    import sys
    config_file = sys.argv[1]
    main(config_file)