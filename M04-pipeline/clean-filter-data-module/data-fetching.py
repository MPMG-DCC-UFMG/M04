import json
import os
import sys

def main():
    base_dir = os.environ['BASE_DIR']
    temp_dir = base_dir + '/temp-input'
    output_dir = base_dir + '/raw-input'
    hadoop_home = os.environ['HADOOP_HOME']
    config_file = sys.argv[1]

    with open(config_file) as read_file:
        configs = json.load(read_file)
        
        baseHadoopURL = configs['baseHadoopURL']
        files = configs['files']

        for file in files:
            filename = file['fileName']
            complete_remote_filename = f'{baseHadoopURL}/{filename}'
            complete_temp_filename = f'{temp_dir}/{filename}'
            complete_output_filename = f'{output_dir}/{filename}'

            # Fetches the data from the HDFS
            fetch_command = f'{hadoop_home}/bin/hdfs dfs -getmerge {complete_remote_filename} {complete_temp_filename}'
            os.system(fetch_command)

            # Verifies whether the fetched file is non-empty
            # If it is empty, keeps the old one
            if os.path.getsize(complete_temp_filename) > 0:
                os.replace(complete_temp_filename, complete_output_filename)

main()