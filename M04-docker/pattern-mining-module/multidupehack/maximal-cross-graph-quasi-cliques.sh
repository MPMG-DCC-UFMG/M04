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
epsilon_2-clique faltando por licitação (ou seja, cada arresta tem
peso superior a 1 - epsilon_2-clique) e com pelo menos 3 CNPJs e no
máximo epsilon_3+-clique faltando por licitação.  Os epsilons padrão
são respetivamente $EPSILON_2_CLIQUE e $EPSILON_3_CLIQUE_OR_MORE.

Cada linha do arquivo \"vínculos-ponderados\" contém quatro campos
separados por ';': dois CNPJs vinculados, uma licitação da qual
participaram e um peso para este vínculo na hora da licitação.  Cada
tripla deve aparecer no máximo uma vez.  Não pode haver nenhum espaço,
nenhuma tabulação e nenhuma barra (\"/\").
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

min_weight_for_2_cliques=$(printf "1 - $EPSILON_2_CLIQUE\n" | bc -l)
min_weight_for_3_cliques_or_more=$(printf "1 - $EPSILON_3_CLIQUE_OR_MORE / 3\n" | bc -l)

epsilon=$(printf "2 * $EPSILON_3_CLIQUE_OR_MORE\n" | bc -l)
IFS=\;

TMP=`mktemp -dt maximal-cross-graph-quasi-cliques.sh.XXXXXX`
trap "rm -r $TMP 2>/dev/null" 0

# Pre-computing all neighorhoods.
mkdir $TMP/neighborhoods
awk -F \; -v out_dir=$TMP/neighborhoods/ '
{
    print ";" $1 ";" >> out_dir $2
    print ";" $2 ";" >> out_dir $1 }' "$1"
for neighborhood in $TMP/neighborhoods/*
do
    sort -u "$neighborhood" -o "$neighborhood"
done

maximal_cross_graph_3_quasi_cliques_or_more() {
    # $cnpj1 and $cnpj2 must be in the maximal cross-graph quasi-cliques listed at this iteration.
    printf "0;$cnpj1,$cnpj2
" > $TMP/edge
    # Select the distinct vertices that can be associated with cnpj1 and cnpj2.
    sort -mu "$TMP/neighborhoods/$cnpj1" "$TMP/neighborhoods/$cnpj2" > $TMP/vertex_sel
    # # Alternative selection not requiring pre-computed neirboorhoods (a little slower on my system).
    # grep -e "^$cnpj1;" -e "^$cnpj2;" -e ";$cnpj1;" -e ";$cnpj2;" "$1" |
    # 	cut -d \; -f -2 |
    # 	tr \; \\n |
    # 	sort -u |
    # 	awk '{ print ";" $0 ";" }' > $TMP/vertex_sel
    # Give to multidupehack the directed edges between the selected vertices (until awk) so that it lists the maximal cross-graph quasi-cliques (with at most $EPSILON_3_CLIQUE_OR_MORE edge missing in each procurement; $epsilon in the directed graph) involving $cnpj1 and $cnpj2 (option -g) and at least another CNPJ (option -s 3).
    sed 's/;/^/' $TMP/vertex_sel |
	grep -f - "$1" |
	grep -f $TMP/vertex_sel |
	awk -F \; '{ print; print $2 ";" $1 ";" $3 ";" $4 }' |
	/home/pattern-mining-module/multidupehack/multidupehack --ids \; --ods \; -e "2147483648 2147483648 $epsilon" -c 0\ 1 -s 3 -g $TMP/edge --pn ;
}

maximal_cross_graph_2_quasi_clique() {
    noise_per_cnpj=$(awk -v RS='[,#]' '!(NR % 2) { sum += $0 } END { print sum / 2 }' $TMP/procurements)
    printf "$cnpj1#$noise_per_cnpj,$cnpj2#$noise_per_cnpj;"
    cat $TMP/procurements
}

# Computing the pairs of CNPJs connected at least once with a weight exceeding both $min_weight_for_2_cliques and $min_weight_for_3_cliques_or_more (written on the standard output) and those only exceeding at least once the smaller threshold (written in $TMP/pairs_for_2-cliques or in $TMP/pairs_for_3_cliques_or_more, depending what is the smaller threshold).
awk -F \; -v threshold_for_2_cliques=$min_weight_for_2_cliques -v threshold_for_3_cliques_or_more=$min_weight_for_3_cliques_or_more -v out_dir=$TMP/ '
BEGIN {
    if (threshold_for_2_cliques < threshold_for_3_cliques_or_more) {
        smaller_threshold = threshold_for_2_cliques
        greater_threshold = threshold_for_3_cliques_or_more
        out = out_dir "pairs_for_2_cliques" }
    else {
        smaller_threshold = threshold_for_3_cliques_or_more
        greater_threshold = threshold_for_2_cliques
        out = out_dir "pairs_for_3_cliques_or_more" } }

{
    if ($4 >= greater_threshold)
        for_all[$1 ";" $2]
    else
        if ($4 >= smaller_threshold)
            for_some[$1 ";" $2] }

END {
    for (pair in for_all)
        print pair
    for (pair in for_some)
        if (!(pair in for_all))
            print pair >> out }' "$1" | while read cnpj1 cnpj2
do
    # Maximal cross-graph 3+-quasi-cliques and 2-quasi-cliques with at least one weight exceeding both $min_weight_for_2_cliques and $min_weight_for_3_cliques_or_more.
    # $TMP/procurements is the support of the 2-quasi-clique, output only if no maximal cross-graph quasi-clique in mulltidupehack's last execution involves a superset of procurements (grep).
    grep -e "^$cnpj1;$cnpj2" -e "^$cnpj2;$cnpj1" "$1" |
	awk -F \; -v threshold=$min_weight_for_2_cliques '$4 >= threshold { print $3 "#" 2 * (1 - $4) }' |
	sort -n |
	tr \\n , |
	sed 's/,$/\n/' |
	tee $TMP/procurements |
	sed -e 's/#[^,]*//g' -e 's/,/\\b.*,.*\\b/g' -e 's/^/;.*\\b/' -e 's/$/\\b/' > $TMP/superset_of_procurements
    maximal_cross_graph_3_quasi_cliques_or_more "$1" |
	tee -a $TMP/maximal-cross-graph-3+-quasi-cliques |
	sed 's/#[^,;]*//g' |
	grep -qf $TMP/superset_of_procurements || maximal_cross_graph_2_quasi_clique
done
# Every maximal cross-graph 3+-quasi-clique was found as many times as there are in it pairs of CNPJs connected at least once with weight $min_weight_for_3_cliques_or_more or more: below, sort -u removes repetitions.

if [ -e $TMP/pairs_for_2_cliques ]
then
    # $min_weight_for_2_cliques is the smaller weight and there may be missing maximal cross-graph 2-quasi-cliques.
    sort -u $TMP/maximal-cross-graph-3+-quasi-cliques -o $TMP/maximal-cross-graph-3+-quasi-cliques
    # Maximal cross-graph 2-quasi-cliques with no weight exceeding $min_weight_for_3_cliques_or_more.
    while read cnpj1 cnpj2
    do
	# $TMP/procurements is the support of the 2-quasi-clique, output only if no maximal cross-graph quasi-clique, among all those with at least three CNPJs, involves a superset of procurements (grep).
	grep -e "^$cnpj1;$cnpj2" -e "^$cnpj2;$cnpj1" "$1" |
	    awk -F \; -v threshold=$min_weight_for_2_cliques '$4 >= threshold { print $3 "#" 2 * (1 - $4) }' |
	    sort -n |
	    tr \\n , |
	    sed 's/,$/\n/' |
	    tee $TMP/procurements |
	    sed -e 's/#[^,]*//g' -e 's/,/\\b.*,.*\\b/g' |
	    awk -v cnpj1="$cnpj1" -v cnpj2="$cnpj2" '{ print "\\b" cnpj1 "\\b.*," cnpj2 "\\b.*;.*\\b" $0 "\\b\n\\b" cnpj2 "\\b.*," cnpj1 "\\b.*;.*\\b" $0 "\\b" }' |
	    grep -qf - $TMP/maximal-cross-graph-3+-quasi-cliques || maximal_cross_graph_2_quasi_clique
    done < $TMP/pairs_for_2_cliques
    cat $TMP/maximal-cross-graph-3+-quasi-cliques
    exit
fi

if [ -e $TMP/pairs_for_3_cliques_or_more ]
then
    # $min_weight_for_3_cliques_or_more is the smaller weight and there may be missing maximal cross-graph 3-quasi-cliques.
    while read cnpj1 cnpj2
    do
	maximal_cross_graph_3_quasi_cliques_or_more "$1"
    done < $TMP/pairs_for_3_cliques_or_more >> $TMP/maximal-cross-graph-3+-quasi-cliques
fi
sort -u $TMP/maximal-cross-graph-3+-quasi-cliques
