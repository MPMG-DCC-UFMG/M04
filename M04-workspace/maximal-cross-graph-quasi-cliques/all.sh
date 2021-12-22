#!/bin/sh
# Distributed under the terms of the GNU General Public License v3, or (at your option) any later version.
# AUTHOR: Loïc Cerf
# e-mail: lcerf@dcc.ufmg.br

# # Manter somente uma orientação para cada arresta ponderada; na presença de várias vezes a mesma arresta, aquela de maior peso.
# awk -F , '
# {
#     if ($1 ";" $2 ";" $3 in edges) {
#         if ($4 > edges[$1 ";" $2 ";" $3])
#             edges[$1 ";" $2 ";" $3] = $4 }
#     else {
#         if ($2 ";" $1 ";" $3 in edges) {
#             if ($4 > edges[$2 ";" $1 ";" $3])
#                 edges[$2 ";" $1 ";" $3] = $4 }
#         else
#             edges[$1 ";" $2 ";" $3] = $4 } }

# END {
#     for (e in edges)
#         print e ";" edges[e] }' vinculos-societarios-validos.csv telefone-em-comum > vínculos-ponderados

TMP=`mktemp -t all.sh.XXXXXX`
trap "rm $TMP 2>/dev/null" 0

# Para cada tripla (CNPJ1, CNPJ_2, licitação) com CNPJ1 e CNPJ2 ordenados de forma consistente, mantém o vínculo de maior peso
./greatest-weight.awk -F \; vínculos-ponderados > $TMP

# CNPJs ranqueados por nível de alarme, considerando somente os vínculos com peso 1.
awk -F \; '$4 == 1' $TMP | ./rank-CNPJs.sh - valores_lic.csv > CNPJs-ranqueados.tsv

# Cross-graph quasi-cliques ranqueadas por nível de alarme.
./maximal-cross-graph-quasi-cliques.sh $TMP | ./rank-cross-graph-quasi-cliques.sh - valores_lic.csv > 2-cliques-and-1-noise-3+-cliques.tsv


# Figuras para o relatório
mkdir 'Figuras no relatório M04.3'

# Número de cross-graph quasi-cliques máximas por número de CNPJs envolvidos
cut -f 1 2-cliques-and-1-noise-3+-cliques.tsv | awk -F , '{ print NF }' | sort -n | uniq -c | awk '{ while (++i != $2) print 0, i; print $1, i }' > $TMP
printf "set encoding iso_8859_1
set style data histogram
set boxwidth 2 relative
set style histogram
set style fill solid border -1
set terminal pngcairo enhanced color solid lw 3
set xlabel 'número de CNPJs envolvidos'
set xrange [.001:]
set ylabel 'número de cross-graph quasi-cliques máximas'
set logscale y
set yrange [.1:]
set output 'Figuras no relatório M04.3/nb of patterns per nb of CNPJs.png'
plot '$TMP' using 1:xtic(2) notitle" #| gnuplot

# Maior número de CNPJs
awk '$4 > max { max = $4; argmax = $0 } END { print argmax }' 2-cliques-and-1-noise-3+-cliques.tsv #| ./png-table.sh - 'Figuras no relatório M04.3/most-CNPJs' 3

# Top-3
head -3 2-cliques-and-1-noise-3+-cliques.tsv #| ./png-table.sh - 'Figuras no relatório M04.3/top-3'
