#!/bin/sh
# Distributed under the terms of the GNU General Public License v3, or (at your option) any later version.
# AUTHOR: Loïc Cerf
# e-mail: lcerf@dcc.ufmg.br

# multidupehack must be installed.

EPSILON_2_CLIQUE=0.999999999
EPSILON_3_CLIQUE_OR_MORE=1

if [ -z "$1" -o "$1" = "-h" -o "$1" = "--help" ]
then
    printf "Uso: $0 vínculos-ponderados [epsilon_2-clique] [epsilon_3+-clique]

Lista as maximal cross-graph quasi-cliques com 2 CNPJs e no máximo
epsilon_2-clique vínculos faltando por licitação (ou seja, cada
arresta tem peso superior a 1 - epsilon_2-clique) e com pelo menos 3
CNPJs e no máximo epsilon_3+-clique vínculos faltando por licitação.
Os epsilons padrão são respetivamente $EPSILON_2_CLIQUE e $EPSILON_3_CLIQUE_OR_MORE.

Cada linha do arquivo \"vínculos-ponderados\" contém quatro campos
separados por ';': dois CNPJs vinculados, uma licitação da qual
participaram e um peso para este vínculo na hora da licitação.  Cada
tripla deve aparecer no máximo uma vez.  Não pode haver nenhum espaço,
nenhuma tabulação, nenhuma vírgula e nenhuma barra (\"/\").
"
    # That format can be different, modifying the script below.
    exit
fi

# Usage error for noise tolerance thresholds that would turn the computation incorrect.
if [ -n "$2" ]
then
    if [ $(printf "$2 < 0 || $2 >= 1\n" | bc -l) -eq 1 ]
    then
	printf 'epsilon_2-clique deve ser pelo menos 0 e menor que 1!
' 1>&2
	exit 64
    fi
    EPSILON_2_CLIQUE=$2
fi
if [ -n "$3" ]
then
    if [ $(printf "$3 < 0 || $3 >= 2\n" | bc -l) -eq 1 ]
    then
	printf 'epsilon_3+-clique deve ser pelo menos 0 e menor que 2!
' 1>&2
	exit 64
    fi
    EPSILON_3_CLIQUE_OR_MORE=$3
fi

TMP=`mktemp -dt maximal-cross-graph-quasi-cliques.sh.XXXXXX`
trap "rm -r $TMP 2>/dev/null" 0

# Merge procurements with equal graphs, identifying every group with its line number in $TMP/procurement_groups
sort -t \; -k 1,2 "$1" |
    gawk -F \; '
{
    edge[$3][++size[$3]] = $1 ";" $2 ";" $4 }

END {
    for (procurement in size)
        procurements_grouped_by_size[size[procurement]][procurement]
    for (s in procurements_grouped_by_size) {
        s = s + 0
        for (procurement in procurements_grouped_by_size[s]) {
            if (procurement in procurements_grouped_by_size[s]) {
                printf "%s", procurement
                delete procurements_grouped_by_size[s][procurement]
                    for (other_procurement in procurements_grouped_by_size[s]) {
                        for (e = 1; e <= s && edge[other_procurement][e] == edge[procurement][e]; ++e);
                        if (e > s) {
                            printf ";%s", other_procurement
                            delete procurements_grouped_by_size[s][other_procurement] } }
                print "" } } } }'  |
    tee $TMP/procurement_groups |
    awk -F \; '
{
    if (FILENAME == ARGV[1]) {
        ++group_id
        for (i = 1; i <= NF; ++i)
            group[$i] = group_id }
    else
        tuple[$1 ";" $2 ";" group[$3] ";" $4] }

END {
    for (t in tuple)
        print t }' - "$1" > $TMP/reduced_data

# Pre-compute all neighborhoods
mkdir $TMP/neighborhoods
awk -F \; -v neighborhoods=$TMP/neighborhoods/ '
{
    print ";" $1 ";" > neighborhoods $2
    print ";" $2 ";" > neighborhoods $1 }' $TMP/reduced_data
for neighborhood in $TMP/neighborhoods/*
do
    sort -u "$neighborhood" -o "$neighborhood"
done

/home/pattern-mining-module/mine-maximal-cross-graph-quasi-cliques.sh $TMP $EPSILON_2_CLIQUE $EPSILON_3_CLIQUE_OR_MORE | awk -F \; '
{
    if (FILENAME == ARGV[1])
        group[NR] = $0
    else {
        printf "%s;", $1
        nb_of_groups = split($2, a, ",")
        split(a[nb_of_groups], b, "#")
        procurements = group[b[1]]
        gsub(/;/, "#" b[2] ",", procurements)
        printf "%s", procurements "#" b[2]
        while (--nb_of_groups) {
            split(a[nb_of_groups], b, "#")
            procurements = group[b[1]]
            gsub(/;/, "#" b[2] ",", procurements)
            printf ",%s", procurements "#" b[2] }
        print "" } }' $TMP/procurement_groups -
