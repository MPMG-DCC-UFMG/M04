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

awk -F \; 'FILENAME == ARGV[1] {
    value[$1] = $2 }

FILENAME == ARGV[2] {
    alarm[$1] += value[$3] * $4
    alarm[$2] += value[$3] * $4
    participations[$1] = participations[$1] " " $3
    participations[$2] = participations[$2] " " $3 }

END {
    for (cnpj in participations) {
    	split(substr(participations[cnpj], 2), a)
	n = asort(a)
        old = a[1]
        printf cnpj "\t" old
	for (i = 2; i <= n; ++i) {
	    if (a[i] == old)
	        ++count
	    else {
	    	if (count) {
		    printf "x" ++count
		    count = 0 }
                old = a[i]
	        printf "," old } }
        if (count) {
            printf "x" ++count
            count = 0 }
        printf "\t%d\n", alarm[cnpj] } }' "$2" "$1" | sort -k 3nr -k 1
