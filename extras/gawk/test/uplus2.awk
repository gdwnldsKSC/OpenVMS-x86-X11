function f(x) { return x ? 1 : 0 }
BEGIN {
	for(i = 0; i != 3; ++i) {
		print +a[y], +b[y], f(a[y])
	}
}
