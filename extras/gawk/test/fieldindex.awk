{
	data[$1] = NR
}

END {
	for (i in data) {
		for (j in data) {
			if ("version" in PROCINFO) {	# gawk
				printf "<%s> typeof(i) = %s\n", i, typeof(i)
				printf "<%s> typeof(j) = %s\n", j, typeof(j)
			}
			printf("i = %s, j = %s\n", i, j)
			if (i != j) {
				printf("(i < j) = %d, (i < \"\" j) = %d\n",
				       (i < j), (i < "" j))
				exit 0
			}
		}
	}
}
