# Test the weird greek locale

BEGIN {
	tf[0] = "false"
	tf[1] = "true"

	sigma[1] = "Σ"
	sigma[2] = "σ"
	sigma[3] = "ς"

	print "IGNORECASE = 0"
	for (i = 1; i <= 3; i++) {
		for (j = 1; j <= 3; j++) {
			pat = sprintf("[=%s=]", sigma[j])

			printf("%s ~ /%s/ = %s\n", sigma[i], pat, tf[sigma[i] ~ pat])
		}
	}
	print ""

	IGNORECASE = 1
	print "IGNORECASE = 1"
	for (i = 1; i <= 3; i++) {
		for (j = 1; j <= 3; j++) {
			pat = sprintf("[=%s=]", sigma[j])

			printf("%s ~ /%s/ = %s\n", sigma[i], pat, tf[sigma[i] ~ pat])
		}
	}

}
