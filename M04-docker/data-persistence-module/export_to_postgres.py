import json
import pandas as pd
import psycopg2
import pyhive
from pandas import DataFrame
from psycopg2 import extras
from pyhive import hive

class ExportData:

    def __init__(self, config_file_name: str, size: int):
        self.__config = self.__read_config_file(config_file_name)
        self.__size_temp = size

    def __read_config_file(self, file_name: str) -> dict:
        with open(file_name) as json_file:
            return json.load(json_file)

    def __get_postgres_conn(self, schema='public') -> psycopg2:
        options: str = f'''-c search_path=dbo,{schema}'''
        postgres_conn = psycopg2.connect(
            user=self.__config["postgresCredentials"]["user"],
            password=self.__config["postgresCredentials"]["password"],
            host=self.__config["postgresCredentials"]["host"],
            port=self.__config["postgresCredentials"]["port"],
            options=options
        )

        postgres_conn.autocommit = True

        return postgres_conn

    def __get_hive_conn(self) -> hive.Connection:
        database = self.__config["hiveCredentials"]["database"]
        host = self.__config["hiveCredentials"]["host"]
        port = self.__config["hiveCredentials"]["port"]
        user = self.__config["hiveCredentials"]["user"]
        password = self.__config["hiveCredentials"]["password"]

        conn = hive.Connection(host=host, port=port, username=user, password=password, database=database,
                               auth='CUSTOM')
        return conn


    def __create_postgres_database(self) -> None:
        postgress_connection = self.__get_postgres_conn()
        cur = postgress_connection.cursor()
        try:
            sql_create_db = f'''CREATE schema {self.__config['postgresCredentials']['schema']}'''
            cur.execute(sql_create_db)
            print("Database created successfully!")
        except psycopg2.errors.DuplicateDatabase:
            print(f"Database {self.__config['postgresCredentials']['schema']} already exists!")
        finally:
            cur.close()
            postgress_connection.close()

        postgress_connection = self.__get_postgres_conn(self.__config['postgresCredentials']['schema'])
        cur = postgress_connection.cursor()
        try:
            self.__create_tables_and_views(cur)
            print("Tables and views created successfully!")
        except psycopg2.errors.DuplicateDatabase:
            print(f"Database {self.__config['postgresCredentials']['schema']} already exists!")
        finally:
            cur.close()
            postgress_connection.close()

    def __create_tables_and_views(self, cursor) -> None:
        file_tables = self.__config['path_script_create_tables']
        file_views = self.__config['path_script_create_views']

        with open(file_tables, 'r') as f_tables, open(file_views, 'r') as f_views:
            sql_file_tables = f_tables.read()
            sql_file_views = f_views.read()

        # all SQL commands (split on ';')
        sql_commands_tables = sql_file_tables.split(';')
        sql_commands_views = sql_file_views.split(';')

        # Execute every command from the input file
        for command in sql_commands_tables:
            cursor.execute(command)

        # Execute every command from the input file
        for command in sql_commands_views:
            cursor.execute(command)


    def __retrieve_data_from_hive(self, hive_conn: pyhive.hive.Connection, command: str) -> DataFrame:
        dataframe: DataFrame = pd.read_sql(command, hive_conn)
        print(dataframe.head(10))
        return dataframe


    def __execute_values(self, df: DataFrame, table: str) -> None:
        print("Data Types of The Columns in Data Frame")
        print(df.dtypes)
        print('--------------------------------')
        print(f'Rows number: {df.shape}')
        print('--------------------------------')

        tuples = [tuple(x) for x in df.to_numpy()]

        cols = ','.join(list(df.columns))
        cols = cols.replace(''.join([table, '.']), '')

        # SQL query to execute
        query = "INSERT INTO %s(%s) VALUES %%s" % (table, cols)
        conn = self.__get_postgres_conn(self.__config['postgresCredentials']['schema'])
        cursor = conn.cursor()

        try:
            extras.execute_values(cursor, query, tuples)
            conn.commit()
            print(f'The dataframe {table} is inserted')
        except (Exception, psycopg2.DatabaseError) as error:
            print("Error: %s" % error)
            conn.rollback()
        finally:
            cursor.close()

    def __simple_export_data(self, table: str) -> None:
        print(f'Loading data from {table}.........')
        command = f'SELECT * FROM {table}'
        self.__export(table, command)
        print(f'Export: {table}')

    def __export(self, table, command: str) -> None:
        hive_conn: hive.Connection = self.__get_hive_conn()
        dataframe: DataFrame = self.__retrieve_data_from_hive(hive_conn, command)
        self.__execute_values(dataframe, table)

    def __get_rows_number_table(self, table: str) -> int:
        command = f'SELECT COUNT(*) AS SIZE FROM {table}'
        hive_conn: hive.Connection = self.__get_hive_conn()
        with hive_conn.cursor() as cur:
            cur.execute(command)
            table_count = cur.fetchone()
        return table_count[0]

    def __batch_export_data(self, table: str) -> None:
        batch_size = self.__config["max_batch_size"]
        rows_number_table: int = self.__get_rows_number_table(table)
        iteration: int = 0
        print(f'EXPORT DATA FROM {table}......')
        while iteration < rows_number_table:
            print(f'ROWS [{iteration},{(iteration+batch_size)}] of {rows_number_table}')
            command: str = f'''SELECT * FROM {table} limit {batch_size} offset {iteration}'''
            self.__export(table, command)
            iteration = iteration + batch_size

    def __file_export_data(self, file_path: str, file_name: str, table_name: str) -> None:
        full_path = ''.join([file_path, file_name])
        dataframe: DataFrame = pd.read_csv(full_path, sep=';', low_memory=False, nrows=self.__size_temp)
        print(f'EXPORT DATA FROM {table_name}......')
        self.__execute_values(dataframe, table_name)


    def run(self) -> None:
        if self.__config["execute_create_database"]:
            self.__create_postgres_database()

        tables_simple_export = self.__config["simple_export"]
        for table in tables_simple_export:
            if table['export']:
                table_name = table['table_name']
                self.__simple_export_data(table_name)

        tables_batch_export = self.__config["batch_export"]
        for table in tables_batch_export:
            if table['export']:
                table_name = table['table_name']
                self.__batch_export_data(table_name)

        data_export = self.__config["data_from_file"]
        for data in data_export:
            if data['export']:
                file_path = data['file_path']
                file_name = data['file_name']
                table_name = data['table_name']
                self.__file_export_data(file_path, file_name, table_name)


def execute(config_file: str, size=100000000):
    print("VERSION: 1.0")
    exp = ExportData(config_file, size)
    exp.run()

if __name__ == '__main__':
    import sys
    config_file = sys.argv[1]
    execute(config_file)







