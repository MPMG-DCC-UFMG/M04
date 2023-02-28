create or replace view m04_item_vencedor as (
SELECT
	id_licitacao,
	id_item_licitacao as id_item,
	ano_referencia,
	mes_exercicio as mes_referencia,
	qtde_item_show as quantidade_vencedor,
	vlr_unitario_show as valor_unitario_vencedor,
	(qtde_item_show*vlr_unitario_show) as valor_total_vencedor,
	num_cpf_cnpj_vencedor_show as cnpj_vencedor,
	nome_vencedor_show as nome_cnpj_vencedor,
	dsc_item_show as descricao_item,
	dsc_unid_medida_show as unidade_medida,
	grupo
FROM m04_vencedores
ORDER BY id_item
) ;

create or replace view m04_analise_itens_licitacoes as (
SELECT
  id_licitacao,
  count(DISTINCT num_cpf_cnpj_vencedor_show) as quantidade_participantes,
  sum(qtde_item_show) as quantidade_itens_licitacao
FROM m04_vencedores
GROUP BY id_licitacao
ORDER BY quantidade_participantes DESC, quantidade_itens_licitacao DESC
);

create or replace view m04_analise_licitacoes_municipio as (
SELECT
	cod_ibge_municipio as id_municipio,
	num_cpf_cnpj_municipio as cnpj_municipio,
	nome_cidade as nome_municipio,
	count(DISTINCT id_licitacao) as quantidade_licitacoes,
	count(DISTINCT num_cpf_cnpj_licitante) as quantidade_concorrentes
FROM m04_itens_lote
GROUP BY cod_ibge_municipio,
	num_cpf_cnpj_municipio,
	nome_cidade
ORDER BY nome_municipio
);


create or replace view m04_analise_licitante_participacoes as (
SELECT
	num_cpf_cnpj_vencedor_show as cnpj,
	count(DISTINCT num_cpf_cnpj_vencedor_show) as quantidade_itens_vencidos,
	sum(qtde_item_show*vlr_unitario_show) as valor_total_itens_vencidos,
	min(qtde_item_show*vlr_unitario_show) as valor_minimo_itens_vencidos,
	max(qtde_item_show*vlr_unitario_show) as valor_maximo_itens_vencidos,
	avg(qtde_item_show*vlr_unitario_show) as valor_medio_itens_vencidos
FROM m04_vencedores
GROUP BY num_cpf_cnpj_vencedor_show
ORDER BY valor_total_itens_vencidos DESC, num_cpf_cnpj_vencedor_show
);

create or replace view m04_analise_ranking_itens as (
SELECT
	id_licitacao,
	id_item_licitacao as id_item,
	grupo,
	cod_ibge_municipio,
	count(DISTINCT num_cpf_cnpj_licitante) as quantidade_participantes,
	min(num_quant_item*vlr_unitario) as valor_minimo,
	max(num_quant_item*vlr_unitario) as valor_maximo,
	avg(num_quant_item*vlr_unitario) as valor_medio
FROM m04_itens_lote
GROUP BY id_licitacao, id_item_licitacao, grupo, cod_ibge_municipio
ORDER BY id_item
);

create or replace view m04_detalhe_item as (
SELECT
	id_orgao,
	nome_orgao,
	id_licitacao,
	id_item_licitacao as id_item,
	nome_item as descricao_item,
	cod_item as codigo_item,
	nome_cidade as nome_municipio,
	num_cpf_cnpj_municipio as cnpj_municipio,
	ano_exercicio as ano_referencia,
	mes_licitacao as mes_referencia,
	grupo,
	max(num_quant_item) as quantidade,
	max(num_quant_item*vlr_unitario) as valor_maximo_licitado,
	min(num_quant_item*vlr_unitario) as valor_minimo_licitado,
	avg(num_quant_item*vlr_unitario) as valor_medio_licitado,
	count(distinct num_cpf_cnpj_licitante) as quantidade_licitantes
FROM m04_itens_lote
GROUP BY
	id_orgao,
	nome_orgao,
	id_licitacao,
	id_item_licitacao,
	nome_item,
	cod_item,
	nome_cidade,
	num_cpf_cnpj_municipio,
	ano_exercicio,
	mes_licitacao,
	grupo
);

create or replace view m04_detalhe_item_vencido as (
SELECT
	v.id_licitacao,
	v.id_item_licitacao AS id_item,
	v.id_orgao as id_orgao,
	v.ano_referencia as ano_referencia,
	v.mes_exercicio as mes_referencia,
	qtde_item_show as quantidade_item,
	vlr_unitario_show as valor_unitario_item,
	(qtde_item_show * vlr_unitario_show) as valor_total_item,
	num_cpf_cnpj_vencedor_show as cnpj_licitante,
	nome_vencedor_show as nome_licitante,
	dsc_item_show as descricao_item,
	dsc_unid_medida_show as descricao_unidade_medida,
	grupo
FROM m04_vencedores v
ORDER BY id_item
);

create or replace view m04_detalhe_licitacao as (
SELECT
	id_licitacao,
	id_orgao,
	nome_orgao,
	nome_cidade as nome_municipio,
	grupo,
	ano_exercicio as ano_referencia,
	mes_licitacao as mes_referencia,
	count(distinct num_cpf_cnpj_licitante) as quantidade_participantes_licitacao,
	min(vlr_unitario*num_quant_item) as valor_total
FROM m04_itens_lote
GROUP BY id_licitacao,
	id_orgao,
	nome_orgao,
	nome_cidade,
	ano_exercicio,
	mes_licitacao,
	grupo
ORDER BY id_licitacao DESC
);

create or replace view m04_detalhe_licitacoes as (
SELECT
  id_licitacao,
  id_orgao as id_orgao_criador,
  nome_orgao,
  num_cpf_cnpj_municipio as cnpj_orgao,
  ano_exercicio as ano_licitacao,
  mes_licitacao,
  id_item_licitacao as id_item,
  nome_item as descricao_item,
  vlr_unitario as valor_unitario_lance,
  num_quant_item as quantidade,
  (num_quant_item * vlr_unitario) as valor_total_lance,
  num_cpf_cnpj_licitante as cnpj_licitante,
  nome_licitante,
  grupo
FROM m04_itens_lote
ORDER BY id_item
);


create or replace view m04_detalhe_licitante as (
SELECT
	num_cpf_cnpj_licitante as cnpj,
	nome_licitante as nome,
	cod_tipo_pessoa_licitante as tipo_pessoa,
	grupo
FROM m04_itens_lote
);

create or replace view m04_detalhe_orgao as (
SELECT
  DISTINCT
  CAST(id_orgao AS text) AS id,
  nome_orgao AS nome,
  nome_tipo_orgao AS tipo,
  num_cpf_cnpj_municipio AS cnpj,
  cod_ibge_municipio AS codigo_ibge,
  nome_cidade AS cidade,
  sigla_uf AS uf,
  grupo
FROM m04_itens_lote
);

create or replace view m04_item_participantes as (
SELECT
	id_licitacao,
	id_item_licitacao as id_item,
	num_cpf_cnpj_licitante as cnpj_licitante,
	nome_licitante,
	grupo,
	num_quant_item as quantidade_lance,
	vlr_unitario as valor_unitario_lance,
	(num_quant_item*vlr_unitario) as valor_total_lance
FROM m04_itens_lote
ORDER BY id_item_licitacao
);

create or replace view m04_item_vencedor as (
SELECT
	id_licitacao,
	id_item_licitacao as id_item,
	ano_referencia,
	mes_exercicio as mes_referencia,
	qtde_item_show as quantidade_vencedor,
	vlr_unitario_show as valor_unitario_vencedor,
	(qtde_item_show*vlr_unitario_show) as valor_total_vencedor,
	num_cpf_cnpj_vencedor_show as cnpj_vencedor,
	nome_vencedor_show as nome_cnpj_vencedor,
	dsc_item_show as descricao_item,
	dsc_unid_medida_show as unidade_medida,
	grupo
FROM m04_vencedores
ORDER BY id_item
);

create or replace view m04_itens_participantes as (
SELECT
	id_licitacao,
	id_item_licitacao as id_item,
	grupo,
	count(distinct num_cpf_cnpj_licitante) as quantidade_participantes_item
FROM m04_itens_lote
GROUP BY id_licitacao,
	id_item_licitacao,
	grupo
ORDER BY id_item
);

create or replace view m04_licitacoes_municipio as (
SELECT
	cod_ibge_municipio as id_municipio,
	nome_cidade as nome_municipio,
	sigla_UF,
	id_licitacao,
	grupo,
	id_item_licitacao as id_item,
	nome_item as descricao_item,
	ano_exercicio as ano_referencia,
	mes_licitacao as mes_referencia,
	count(distinct num_cpf_cnpj_licitante) as quantidade_licitantes
FROM m04_itens_lote
GROUP BY cod_ibge_municipio,
	nome_cidade,
	sigla_UF,
	id_licitacao,
	id_item_licitacao,
	grupo,
	nome_item,
	ano_exercicio,
	mes_licitacao
ORDER BY id_item_licitacao
);

create or replace view m04_vinculo_endereco as (
SELECT
	id_licitacao,
	id_item_licitacao as id_item_licitado,
	num_lote,
	num_cpf_cnpj1 as cnpj1,
	num_cpf_cnpj_matriz1 as matriz_cnpj1,
	nome_matriz1,
	num_cpf_cnpj2 as cnpj2,
	num_cpf_cnpj_matriz2 as matriz_cnpj2,
	nome_matriz2,
	data_referencia,
	data_inicio1 as data_entrada_vinculo_cnpj1,
	data_fim1 as data_saida_vinculo_cnpj1,
	data_inicio2 as data_entrada_vinculo_cnpj2,
	data_fim2 as data_saida_vinculo_cnpj2,
	descricao_tipo_logradouro,
	logradouro,
	numero,
	complemento1,
	complemento2,
	flag_complemento_igual,
	bairro,
	cep,
	uf,
	nome_municipio,
	cod_municipio_siafi,
	cod_municipio_ibge,
	grupo
FROM m04_vinculo_endereco_licitacoes
ORDER BY id_item_licitado
);


create or replace view m04_vinculo_telefonico as (
SELECT
	num_cpf_cnpj1 as cnpj1,
	num_cpf_cnpj_matriz1 as matriz_cnpj1,
	nome_matriz1,
	num_cpf_cnpj2 as cnpj2,
	num_cpf_cnpj_matriz2 as matriz_cnpj2,
	nome_matriz2,
    id_licitacao,
    data_referencia,
    num_lote,
    id_item_licitacao as id_item_licitado,
    ddd_telefone,
	data_inicio1 as data_entrada_vinculo_cnpj1,
	data_fim1 as data_saida_vinculo_cnpj1,
	data_inicio2 as data_entrada_vinculo_cnpj2,
	data_fim2 as data_saida_vinculo_cnpj2,
    grupo
FROM m04_vinculo_telefone_licitacoes
ORDER BY id_item_licitacao
);

create or replace view m04_rank_cnpjs as (
SELECT
  CAST(r.id AS int),
  cnpj,
  participacoes,
  alarme,
  m.nome_corporativo,
  m.codigo_fiscal_cnae,
  m.cnpj_completo,
  m.cnpj_prefixo
FROM m04_saida_rank_cnpjs_matrizes r inner join m04_saida_cnpj_metricas m
	on r.cnpj = m.cnpj_completo
);

create or replace view m04_cross_graph_quasi_cliques as (
SELECT
  CAST(id AS text),
  cnpjs,
  grupos,
  alarme,
  pesos_grupos,
  'FILIAL' as tipo
FROM m04_saida_rank_cross_graph_quasi_cliques_filiais
union
SELECT
  CAST(id AS text),
  cnpjs,
  grupos,
  alarme,
  pesos_grupos,
  'MATRIZ' as tipo
FROM m04_saida_rank_cross_graph_quasi_cliques_matrizes
);

--create or replace view m04_rank_metricas as (
--SELECT
--  CAST(id AS text),
--  iditem,
--  densidade,
--  competicao,
--  quantidadenos,
--  quantidadearestas,
--  quantidadecliques,
--  tamanhomaiorclique,
--  vlritem
--FROM m04_saida_rank_metricas
--);


create or replace view m04_dados_grupo as (
SELECT
  CAST(g.id AS text),
  g.chave,
  g.grupo,
  g.licitacao_id,
  CAST(g.item AS text),
  g.ano,
  g.mes,
  v.id_item_licitacao,
  v.nome_vencedor_show,
  v.num_cpf_cnpj_vencedor_show,
  v.dsc_item_show,
  v.dsc_unid_medida_show,
  v.qtde_item_show,
  v.vlr_unitario_show
FROM m04_dados_grupo g inner join m04_vencedores v
	on g.item = v.id_item_licitacao
);