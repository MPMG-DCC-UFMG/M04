import sys
import pandas as pd
input_path = sys.argv[1]
output_path = sys.argv[2]

df_alarm = pd.read_csv(input_path, sep='\t', names=["cnpjs", "grupos", "alarme"])
df_alarm.to_csv(output_path, sep=';', index=False)