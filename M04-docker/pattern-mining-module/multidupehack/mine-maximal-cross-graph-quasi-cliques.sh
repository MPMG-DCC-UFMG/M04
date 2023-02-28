#!/bin/sh

TMP=$1
min_weight_for_2_cliques=$(printf "1 - $2\n" | bc -l)
min_weight_for_3_cliques_or_more=$(printf "1 - $3 / 3\n" | bc -l)

epsilon=$(printf "2 * $3\n" | bc -l)
IFS=\;

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
    # Give to /home/pattern-mining-module/multidupehack the directed edges between the selected vertices (until awk) so that it lists the maximal cross-graph quasi-cliques (with at most $EPSILON_3_CLIQUE_OR_MORE edge missing in each procurement; $epsilon in the directed graph) involving $cnpj1 and $cnpj2 (option -g) and at least another CNPJ (option -s 3).
    sed 's/;/^/' $TMP/vertex_sel |
	grep -f - "$1" |
	grep -f $TMP/vertex_sel |
	awk -F \; '{ print; print $2 ";" $1 ";" $3 ";" $4 }' |
	/home/pattern-mining-module/multidupehack/multidupehack --ids \; --ods \; -e "2147483648 2147483648 $epsilon" -c 0\ 1 -s 3 -g $TMP/edge --pn |
	/home/pattern-mining-module/multidupehack/canonical-output.awk -F \;
}

maximal_cross_graph_2_quasi_clique() {
    noise_per_cnpj=$(awk -v RS='[,#]' '!(NR % 2) { sum += $0 } END { print sum / 2 }' $TMP/procurements)
    printf "$cnpj1#$noise_per_cnpj,$cnpj2#$noise_per_cnpj;"
    cat $TMP/procurements
}

# Computing the pairs of CNPJs connected at least once with a weight exceeding both $min_weight_for_2_cliques and $min_weight_for_3_cliques_or_more (written on the standard output) and those only exceeding at least once the smaller threshold (written in $1/pairs_for_2-cliques or in $1/pairs_for_3_cliques_or_more, depending what is the smaller threshold).
awk -F \; -v threshold_for_2_cliques=$min_weight_for_2_cliques -v threshold_for_3_cliques_or_more=$min_weight_for_3_cliques_or_more -v out_dir=$1/ '
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
            print pair >> out }' $1/reduced_data | while read cnpj1 cnpj2
do
    # Maximal cross-graph 3+-quasi-cliques and 2-quasi-cliques with at least one weight exceeding both $min_weight_for_2_cliques and $min_weight_for_3_cliques_or_more.
    # $1/procurements is the support of the 2-quasi-clique, output only if no maximal cross-graph quasi-clique in /home/pattern-mining-module/multidupehack's last execution involves a superset of procurements (grep).
    grep -e "^$cnpj1;$cnpj2" -e "^$cnpj2;$cnpj1" $1/reduced_data |
	awk -F \; -v threshold=$min_weight_for_2_cliques '$4 >= threshold { print $3 "#" 2 * (1 - $4) }' |
	sort -n |
	tr \\n , |
	sed 's/,$/\n/' |
	tee $1/procurements |
	sed -e 's/#[^,]*//g' -e 's/,/\\b.*,.*\\b/g' -e 's/^/;.*\\b/' -e 's/$/\\b/' > $1/superset_of_procurements
    maximal_cross_graph_3_quasi_cliques_or_more $1/reduced_data |
	tee -a $1/maximal-cross-graph-3+-quasi-cliques |
	sed 's/#[^,;]*//g' |
	grep -qf $1/superset_of_procurements || maximal_cross_graph_2_quasi_clique
done
# Every maximal cross-graph 3+-quasi-clique was found as many times as there are in it pairs of CNPJs connected at least once with weight $min_weight_for_3_cliques_or_more or more: below, sort -u removes repetitions.

if [ -e $1/pairs_for_2_cliques ]
then
    # $min_weight_for_2_cliques is the smaller weight and there may be missing maximal cross-graph 2-quasi-cliques.
    sort -u $1/maximal-cross-graph-3+-quasi-cliques -o $1/maximal-cross-graph-3+-quasi-cliques
    # Maximal cross-graph 2-quasi-cliques with no weight exceeding $min_weight_for_3_cliques_or_more.
    while read cnpj1 cnpj2
    do
	# $1/procurements is the support of the 2-quasi-clique, output only if no maximal cross-graph quasi-clique with at least three CNPJs is a superpattern (grep).
	grep -e "^$cnpj1;$cnpj2" -e "^$cnpj2;$cnpj1" $1/reduced_data |
	    awk -F \; -v threshold=$min_weight_for_2_cliques '$4 >= threshold { print $3 "#" 2 * (1 - $4) }' |
	    sort -n |
	    tr \\n , |
	    sed 's/,$/\n/' |
	    tee $1/procurements |
	    sed -e 's/#[^,]*//g' -e 's/,/\\b.*,.*\\b/g' |
	    awk -v cnpj1="$cnpj1" -v cnpj2="$cnpj2" '{ print "\\b" cnpj1 "\\b.*," cnpj2 "\\b.*;.*\\b" $0 "\\b\n\\b" cnpj2 "\\b.*," cnpj1 "\\b.*;.*\\b" $0 "\\b" }' |
	    grep -qf - $1/maximal-cross-graph-3+-quasi-cliques || maximal_cross_graph_2_quasi_clique
    done < $1/pairs_for_2_cliques
    cat $1/maximal-cross-graph-3+-quasi-cliques
    exit
fi

if [ -e $1/pairs_for_3_cliques_or_more ]
then
    # $min_weight_for_3_cliques_or_more is the smaller weight and there may be missing maximal cross-graph 3-quasi-cliques.
    while read cnpj1 cnpj2
    do
	maximal_cross_graph_3_quasi_cliques_or_more $1/reduced_data
    done < $1/pairs_for_3_cliques_or_more >> $1/maximal-cross-graph-3+-quasi-cliques
fi
sort -u $1/maximal-cross-graph-3+-quasi-cliques
