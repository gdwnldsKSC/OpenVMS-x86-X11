# Test the weird greek locale

BEGIN {
	tf[0] = "false"
	tf[1] = "true"

	uc_sigma  = "\323"	# lower case is \363, not \362
	lc_sigma1 = "\363"	# upper case is \323
	lc_sigma2 = "\362"	# upper case is \323

	uc_sigma_i  = 0323	# lower case is \363, not \362
	lc_sigma1_i = 0363	# upper case is \323
	lc_sigma2_i = 0362	# upper case is \323

	data1 = "b" uc_sigma
	data2 = "b" lc_sigma1
	data3 = "b" lc_sigma2

	IGNORECASE = 1

	format = "\"b\\%o\" ~ /\\%o/: expect %s, got %s\n"

	printf(format, uc_sigma_i, uc_sigma_i,  "true", tf[data1 ~ uc_sigma])
	printf(format, lc_sigma1_i, uc_sigma_i,  "true",  tf[data2 ~ uc_sigma])
	printf(format, lc_sigma2_i, uc_sigma_i,  "false",  tf[data3 ~ uc_sigma])
	print ""

	printf(format, uc_sigma_i, lc_sigma1_i, "true", tf[data1 ~ lc_sigma1])
	printf(format, lc_sigma1_i, lc_sigma1_i, "true",  tf[data2 ~ lc_sigma1])
	printf(format, lc_sigma2_i, lc_sigma1_i, "false", tf[data3 ~ lc_sigma1])
	print ""

	printf(format, uc_sigma_i, lc_sigma2_i, "true", tf[data1 ~ lc_sigma2])
	printf(format, lc_sigma1_i, lc_sigma2_i, "false", tf[data2 ~ lc_sigma2])
	printf(format, lc_sigma2_i, lc_sigma2_i, "true",  tf[data3 ~ lc_sigma2])
}
