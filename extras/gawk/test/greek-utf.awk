# Test the weird greek locale

BEGIN {
	tf[0] = "false"
	tf[1] = "true"

	uc_sigma  = "Σ"
	lc_sigma1 = "σ"
	lc_sigma2 = "ς"

	data1 = "b" uc_sigma
	data2 = "b" lc_sigma1
	data3 = "b" lc_sigma2

	IGNORECASE = 1

	format = "\"%s\" ~ /%s/: expect %s, got %s\n"

	printf(format, data1, uc_sigma,  "true", tf[data1 ~ uc_sigma])
	printf(format, data2, uc_sigma,  "true",  tf[data2 ~ uc_sigma])
	printf(format, data3, uc_sigma,  "false",  tf[data3 ~ uc_sigma])
	print ""

	printf(format, data1, lc_sigma1, "true", tf[data1 ~ lc_sigma1])
	printf(format, data2, lc_sigma1, "true",  tf[data2 ~ lc_sigma1])
	printf(format, data3, lc_sigma1, "false", tf[data3 ~ lc_sigma1])
	print ""

	printf(format, data1, lc_sigma2, "true", tf[data1 ~ lc_sigma2])
	printf(format, data2, lc_sigma2, "false", tf[data2 ~ lc_sigma2])
	printf(format, data3, lc_sigma2, "true",  tf[data3 ~ lc_sigma2])
}
