#!/bin/sh
# Distributed under the terms of the GNU General Public License v3, or (at your option) any later version.
# AUTHOR: Loïc Cerf
# e-mail: lcerf@dcc.ufmg.br

if [ -z "$2" ]
then
    printf "Uso: $0 padrões valores

Cada linha do arquivo \"padrões\" contém dois campos separados por
';': CNPJs e licitações, ambos separados por ','.  Cada CNPJ e
licitação pode ser seguido de '#' e do ruído coberto, como
multidupehack informa com a opção --pn.  Sem a informação, se
considere que o ruído é 0.

Cada linha do arquivo \"valores\" contém dois campos separados por ';'
ou ' ': uma licitação e seu valor.
"
    # Esse formato pode ser diferente, modificando o script abaixo.
    exit
fi

# gawk -F \; '
# FILENAME == ARGV[1] {
#     value[$1] = $2 }

# FILENAME == ARGV[2] {
#     printf "%s\t", $1
#     nb_of_vertices = split($1, a, ",")
#     max_nb_of_edges = nb_of_vertices * (nb_of_vertices - 1)
#     nb_of_procurements = split($2, a, ",")
#     split(a[nb_of_procurements], b, "#")
#     grade = value[b[1]] * (max_nb_of_edges - b[2])^2 / max_nb_of_edges / 2
#     cmd = "LC_ALL=C sort -t \"" FS "\" -k 2rn"
#     print a[nb_of_procurements] FS grade |& cmd
#     for (i = 1; i <= nb_of_procurements; ++i) {
#         split(a[i], b, "#")
#         contrib = value[b[1]] * (max_nb_of_edges - b[2])^2 / max_nb_of_edges / 2
#         grade += contrib
#         print a[nb_of_procurements] FS contrib |& cmd }
#     close(cmd, "to")
#     cmd |& getline
#     printf "%s", $1
#     while (cmd |& getline)
#         printf ",%s", $1
#     printf "\t%d\t%d\t%d\n", grade, nb_of_vertices, nb_of_procurements
#     close(cmd) }' "$2" "$1" | LC_ALL=C sort -k 3,3nr -k 4,4nr -k 5,5nr -k 1

# Somente o nível de alarme
gawk -F \; '
FILENAME == ARGV[1] {
    value[$1] = $2 }

FILENAME == ARGV[2] {
    printf "%s\t", $1
    nb_of_vertices = split($1, a, ",")
    max_nb_of_edges = nb_of_vertices * (nb_of_vertices - 1)
    nb_of_procurements = split($2, a, ",")
    split(a[nb_of_procurements], b, "#")
    grade = value[b[1]] * (max_nb_of_edges - b[2])^2 / max_nb_of_edges / 2
    cmd = "LC_ALL=C sort -t \"" FS "\" -k 2rn"
    print a[nb_of_procurements] FS grade |& cmd
    while (--nb_of_procurements) {
        split(a[nb_of_procurements], b, "#")
        contrib = value[b[1]] * (max_nb_of_edges - b[2])^2 / max_nb_of_edges / 2
        grade += contrib
        print a[nb_of_procurements] FS contrib |& cmd }
    close(cmd, "to")
    cmd |& getline
    printf "%s", $1
    while (cmd |& getline)
        printf ",%s", $1
    printf "\t%d\n", grade
    close(cmd) }' "$2" "$1" | LC_ALL=C sort -k 3nr -k 1
