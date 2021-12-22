#! /usr/bin/awk -f

function print_canonical_dimension(d) {
    split(d, a, ",")
    n = asort(a)
    printf a[1]
    for (i = 2; i <= n; ++i)
        printf "," a[i] }

{
    print_canonical_dimension($2)
    printf ";"
    print_canonical_dimension($3)
    print "" }
