#!/usr/bin/awk -f
# Distributed under the terms of the GNU General Public License v3, or (at your option) any later version.
# AUTHOR: Loïc Cerf
# e-mail: lcerf@dcc.ufmg.br

$4 > weight[$1 ";" $2 ";" $3] {
    weight[$1 ";" $2 ";" $3] = $4 }

END {
    for (k in weight)
	print k ";" weight[k] }
