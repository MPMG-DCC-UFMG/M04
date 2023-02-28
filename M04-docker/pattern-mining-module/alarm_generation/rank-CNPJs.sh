#!/bin/sh
# Distributed under the terms of the GNU General Public License v3, or (at your option) any later version.
# AUTHOR: Loïc Cerf
# e-mail: lcerf@dcc.ufmg.br

if [ -z "$2" ]
then
    printf "Uso: $0 vínculos-ponderados valores

Cada linha do arquivo \"vínculos-poderados\" contém quatro campos:
dois CNPJs, uma licitação da qual participaram e um peso para esse
vínculo na hora da licitação.

Cada linha do arquivo \"valores\" contém dois campos: uma licitação e
seu valor.

Em ambos arquivos, ';' separa os campos.
"
    # Esse formato pode ser diferente, modificando o script abaixo.
    exit
fi

# # Licitações ordenadas por ID crescente e com números de vínculos por licitação
# awk -F \; 'FILENAME == ARGV[1] {
#     value[$1] = $2 }

# FILENAME == ARGV[2] {
#     term = value[$3] * $4
#     alarm[$1] += term
#     alarm[$2] += term
#     links[$1] = links[$1] FS $3
#     links[$2] = links[$2] FS $3 }

# END {
#     for (cnpj in links) {
#     	split(substr(links[cnpj], 2), a)
# 	n = asort(a)
#         old = a[1]
#         printf cnpj "\t" old
# 	for (i = 2; i <= n; ++i) {
# 	    if (a[i] == old)
# 	        ++count
# 	    else {
# 	    	if (count) {
# 		    printf "x" ++count
# 		    count = 0 }
#                 old = a[i]
# 	        printf "," old } }
#         if (count) {
#             printf "x" ++count
#             count = 0 }
#         printf "\t%d\n", alarm[cnpj] } }' "$2" "$1" | sort -k 3nr -k 1

# Licitações ordenadas por contribução decrescente ao nível de alarme e sem número de vínculos por licitação
gawk -F \; 'FILENAME == ARGV[1] {
    value[$1] = $2 }

FILENAME == ARGV[2] {
    term = value[$3] * $4
    links[$1] = links[$1] FS $3 "," term
    links[$2] = links[$2] FS $3 "," term }

END {
    cmd = "LC_ALL=C sort -t \"" FS "\" -k 2rn"
    for (cnpj in links) {
        printf "%s\t", cnpj
    	nb_of_links = split(substr(links[cnpj], 2), a)
        do {
            split(a[nb_of_links], b, ",")
            contrib[b[1]] += b[2] } while (--nb_of_links)
        grade = 0
        for (procurement in contrib) {
            grade += contrib[procurement]
            print procurement FS contrib[procurement] |& cmd }
        close(cmd, "to")
        delete(contrib)
        cmd |& getline
        printf "%s", $1
        while (cmd |& getline)
            printf ",%s", $1
        printf "\t%d\n", grade
        close(cmd) } }' "$2" "$1" | LC_ALL=C sort -k 3nr -k 1
