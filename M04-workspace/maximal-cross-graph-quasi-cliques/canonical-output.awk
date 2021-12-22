#! /usr/bin/awk -f

function print_canonical_dimension(d) {
    n = split(d, a, ",")
    for (i = 1; i <= n; ++i) {
	split(a[i], b, "#")
	a[i] = b[1]
	noise[b[1]] = b[2] }
    asort(a)
    printf a[1] "#" noise[a[1]]
    for (i = 2; i <= n; ++i)
        printf "," a[i] "#" noise[a[i]] }

{
    print_canonical_dimension($2)
    printf ";"
    print_canonical_dimension($3)
    print "" }
