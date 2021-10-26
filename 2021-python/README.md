# **Trabalho M04 2021 - Detecção de fraudes em licitações**

Projeto desenvolvido em Python e pensado para integração com os demais programas do Programa de Capacidades Analíticas, feito pelo DCC/UFMG em parceria com o MPMG.

## Preparação de ambiente
Com o projeto baixado, é necessário para poder rodá-lo executar os seguintes passos:

1. Criar um ambiente virtual do Python para o projeto, de modo a isolar o projeto dos recursos do sistema. Isso pode ser feito com o comando:

    ``python3 -m venv venv/``
    
    e ativá-lo, usando o comando:

    ``source venv/bin/activate``

    Será necessário ativá-lo a cada vez que for trabalhar, mas a criação do ambiente é única.

2. Instalar as dependências do projeto, usando o comando:

    ``pip install -r requirements.txt``

3. Adicionar os arquivos de input (em formato csv) à pasta 2021-python/data/input. Caso o diretório não exista, deve ser criado.

**Observação**

A cada vez que instalar um novo módulo no projeto, atualize as dependências do projeto, rodando o comando:

``pip freeze > requirements.txt``
