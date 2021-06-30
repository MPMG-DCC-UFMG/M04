import pandas as pd


def salvar_relacoes_entre_cnpjs():
    return pd.read_csv(
        '/home/pedrogabrielbhz/code/M04/M04_2021/data/input/relacao_cnpjs.csv',
        header=None,
        dtype=str,
        sep=' '
    )


def salvar_informacoes_licitacoes():
    return pd.read_csv(
        '/home/pedrogabrielbhz/code/M04/M04_2021/data/input/infos_licitacoes.csv',
        dtype=str,
        sep=';'
    )


def salvar_cnpjs_por_licitacao():
    return pd.read_csv(
        '/home/pedrogabrielbhz/code/M04/M04_2021/data/input/licitacoes_cnpjs_licitantes.csv',

        dtype=str,
        sep=';'
    )
