#!/usr/bin/awk -f

BEGIN {
    n = split(procurements, p, /,/)
    last = p[n] }


{
    m = split($2, q, /,/) }

last <= q[m] {
    i = 1
    elem = p[1]
    for (j = 1; q[j] < elem; ++j);
    while (q[j] == elem && ++i <= n)
	for (elem = p[i]; q[++j] < elem; );
    if (i > n)
	exit 1 }
