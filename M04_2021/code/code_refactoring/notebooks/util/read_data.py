import pandas as pd
import os

working_dir = os.getcwd()
slice = working_dir.find('code', 13)
data_path = working_dir[:slice] + "data/input/"


def salvar_relacoes_entre_cnpjs():
    return pd.read_csv(
        data_path + 'relacao_cnpjs.csv',
        header=None,
        dtype=str,
        sep=' '
    )


def salvar_informacoes_licitacoes():
    return pd.read_csv(
        data_path + 'infos_licitacoes.csv',
        dtype=str,
        sep=';'
    )


def salvar_cnpjs_por_licitacao():
    return pd.read_csv(
        data_path + 'licitacoes_cnpjs_licitantes.csv',
        dtype=str,
        sep=';'
    )
