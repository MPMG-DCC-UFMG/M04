import sys
sys.path.insert(0, '../..')

import pandas as pd
# Pacotes implementados
from util import carregamento_dados as cd, ferramentas_grafos as fg
dump_path = '../../pickles/licitacoes/'

df = pd.read_pickle(dump_path + 'grafos_licitacoes')

