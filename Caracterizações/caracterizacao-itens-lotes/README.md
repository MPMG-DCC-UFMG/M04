# Caracterização dos dados relacionados a itens e lotes.

Esse módulo contém um notebook com análises mais detalhadas e precisas sobre o arquivo. Aqui está um resumo do que foi analisado.

## Instruções de acesso ao notebook

Para acessar o notebook, é necessário fazer um acesso remoto à VM do MP, uma vez que os dados não devem sair de lá.
De dentro dela, deve-se rodar o comando

`jupyter notebook --no-browser --port=8080`

Da MICA, deve-se linkar a porta 8080 da MICA à porta 8080 da VM, com o comando:

`ssh -N -L 8080:localhost:8080 usuariompmg@177.190.215.24 -p 27834`, substituindo o usuário pelo usuário correto

Na máquina local, deve-se linkar a porta 8080 da máquina à porta 8080 da MICA, com o comando:

`ssh -N -L 8080:localhost:8080 usuariodcc@login.dcc.ufmg.br`, substituindo o usuário pelo usuário correto

Feito isso, pode-se acessar o notebook no browser local usando a URL disponibilizada no console do Jupyter

## Análise básica de utilidade dos campos

Muitos dos campos têm valores incoerentes ou nulos, que são de pouca serventia. Analisando os dados, nota-se que para todos os casos, temos sempre um OU `vlr_global` OU `vlr_unitario` e `num_itens`, mas nunca os dois. Assim, para cada caso o valor efetivo do item deve ser extrapolado a partir desses outros fatores.

No caso em que temos `vlr_global`, usamos ele. Quando não temos, usamos `vlr_unitario` * `num_itens` * (1-`vlr_perc_desconto`/100)

Por isso, calculamos um `vlr_global_efetivo`, que observa qual dos dois está disponível e o considera como o valor real.

Ao analisar os itens que excedem o valor de 1 bilhão, todas elas têm o valor `vlr_global` nulo e, portanto, têm como problema um grande número de itens associado a itens com valor muito alto. Foram encontradas 189 entradas

Ao agrupar os itens bilionários por `id_item_licitacao` e verificar quantos grupos temos 159, o que sugere que há mais de uma linha para alguns desses itens. Assim, há mais de um CNPJ licitante e, portanto, há problemas na base de dados de interesse. Mais precisamente, 20 das 159 licitações bilionárias são de nosso interesse.

## Análise dos itens com mais de um participante

Nos interessamos agora apenas pelos itens que tiveram mais do que um participante, uma vez que o trabalho do grupo é baseado em vínculos entre licitantes. Utilizamos o número de linhas em que um item aparece para determinar quantos CNPJs participaram daquele item. 

Contudo, isso nnos dá, de cara, algumas anomalias. Percebe-se que mesmo eliminando duplicatas (só precisamos de uma linha por item, já com as participações contadas), alguns itens têm mais do que uma linha, apresentando valores de `vlr_unitario` distintos, mesmo tendo mesmo `id_item_licitacao` e `num_item`.
 
![image](https://user-images.githubusercontent.com/48407152/154516441-2904ec25-7e10-43af-9b41-9b1cdbd777d6.png)
 
![image](https://user-images.githubusercontent.com/48407152/154516477-ea731b06-563a-4806-a641-cf7188eb507c.png)
 
Isso leva a crer que o valor do item cadastrado para diferentes CNPJs licitantes é, assim, distinto, como em um leilão. Contudo, isso inviabiliza uma análise real dos valores das licitações, a não ser que resolvamos tratar o valor efetivo como a média dos valores que aparecem nas diferentes linhas.

Ao filtrar os itens bilionários, encontramos, dessa vez, 73 linhas. Contudo, há, novamente, itens com mais de uma linha. Há, portanto, CNPJs que fizeram proposições milionárias.

Em seguida fazemos a análise com a média do valor de cada item, abstraindo as ofertas dos diferentes CNPJs.

Criamos assim um dataframe small_df, que contém, para cada item, o seu valor médio e as informações básicas sobre ele. Usando isso, constata-se que apenas 30 itens dos 5 milhões de itens individuais são bilionários. Provavelmente, eles deverão ser filtrados ou ter seu valor alterado, mas não parecem comprometer a base de dados como um todo, desde que tomemos consciência deles.


## Caracterização por vlr_global

* Há 75595 linhas com valor global, sendo que para um mesmo item há valores globais diferentes. Dessa forma, para cada item computamos o valor mínimo. Pra as linhas em que o item é -1, mantivemos o valor global como valor mínimo, interpretando que não há itens diversos na licitação, mas sim um item único
* Há 44151 linhas correspondentes a itens com mais de um CNPJ licitante, correspondendo a 13722 itens distintos (em realidade, são 13832 itens, pois a consulta também agrupou os itens com id_item_licitacao igual a -1. Vai ser melhor tratar de um id composto licitação-item, para considerar os casos em que o id do item é -1, caso em que o id da licitação deve valer.
* Também foi gerado um dataframe que tem somente os itens com múltiplos cnpjs associados, tomando o cuidado de preservar os itens com id -1 como entradas separadas e com valor próprio.
* Há uma seção de exportações para gerar esses dados

Com o dataset reduzido, foi possível utilizar o pandas-profiling. A grandeza de interesse era, principalmente, o valor global mínimo por item. Abaixo coloco algumas métricas encontradas sobre esse valor.
 
![image](https://user-images.githubusercontent.com/48407152/154508921-d08cffea-c116-46f6-8a4f-c0b2bd0eb834.png)
 
![image](https://user-images.githubusercontent.com/48407152/154509044-e27b4bfb-326d-4807-89ac-4e3bb63a6e2e.png)
 
![image](https://user-images.githubusercontent.com/48407152/154509146-994f1321-1f61-46a1-b98e-2add1aba94de.png)
 
Verificamos, assim, que o valor máximo é da ordem de 19 milhões. Apesar de ser um valor alto, é coerente com a ordem de grandeza que licitações podem assumir. Para garantir a razoabilidade, verificamos também quais seriam esses itens milionários e constatamos que são itens que de fato tendem a ter valores altos:
 
![image](https://user-images.githubusercontent.com/48407152/154510282-dd74c71f-d4ab-44a5-850e-29d2f6b156b3.png)
 
Além disso, foi plotado um histograma (em escala logarítmica, pois em linear ele confina praticamente todos os itens juntos) para determinar a distribuição de valores.
 
![image](https://user-images.githubusercontent.com/48407152/154509844-62087ce3-e025-4a95-822e-eda640d8d259.png)
 
Pode-se notar que, como esperado, a maior parte dos itens não passa da ordem das dezenas de milhares de reais, o que sugere que a base de dados pode ser trabalhada de forma coerente
