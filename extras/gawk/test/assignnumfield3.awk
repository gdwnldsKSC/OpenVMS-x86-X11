BEGIN {
	$0 = 10.1
	CONVFMT = "%.6e"
	FS = "e"
	print "" $0, $1, $2, "NF =", NF
	CONVFMT = "%.6g"
	print "" $0, $1, $2, "NF =", NF
}
