BEGIN {
	print a[1]+0
	a[1][1] = "0"
	print typeof(a[1])
}
