{
	n = split($0, a, /./)
	for (i = 1; i <= n;i++) {
		print i, a[i]
	}
}
