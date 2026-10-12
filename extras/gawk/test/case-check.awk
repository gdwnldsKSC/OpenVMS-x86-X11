BEGIN {
	tf[0] = "false"
	tf[1] = "true"

	IGNORECASE = 1

	printf("case (i):\n")
	printf("\tΣ ~ σ: expect true,  got %s\n", tf["Σ" ~ "σ"])
	printf("\tσ ~ Σ: expect true,  got %s\n", tf["σ" ~ "Σ"])
	printf("\tΣ ~ ς: expect true,  got %s\n", tf["Σ" ~ "ς"])
	printf("\tς ~ Σ: expect false, got %s\n", tf["ς" ~ "Σ"])
	printf("\tσ ~ ς: expect false, got %s\n", tf["σ" ~ "ς"])
	printf("\tς ~ σ: expect false, got %s\n", tf["ς" ~ "σ"])
	printf("case (ii)\n")
	printf("\tΩ ~ ω: expect true,  got %s\n", tf["Ω" ~ "ω"])
	printf("\tω ~ Ω: expect true,  got %s\n", tf["ω" ~ "Ω"])
	printf("\tΩ ~ ω: expect false, got %s\n", tf["Ω" ~ "ω"])
	printf("\tω ~ Ω: expect true,  got %s\n", tf["ω" ~ "Ω"])
	printf("\tΩ ~ Ω: expect false, got %s\n", tf["Ω" ~ "Ω"])
	printf("\tΩ ~ Ω: expect false, got %s\n", tf["Ω" ~ "Ω"])
	printf("case (iii)\n")
	printf("\tẞ ~ ß: expect false, got %s\n", tf["ẞ" ~ "ß"])
	printf("\tß ~ ẞ: expect true,  got %s\n", tf["ß" ~ "ẞ"])
	printf("case (iv)\n")
	printf("\tǄ ~ ǆ: expect true,  got %s\n", tf["Ǆ " ~ "ǆ"])
	printf("\tǆ ~ Ǆ: expect true,  got %s\n", tf["ǆ " ~ "Ǆ"])
	printf("\tǄ ~ ǅ: expect true,  got %s\n", tf["Ǆ " ~ "ǅ"])
	printf("\tǅ ~ Ǆ: expect false, got %s\n", tf["ǅ " ~ "Ǆ"])
	printf("\tǆ ~ ǅ: expect true,  got %s\n", tf["ǆ " ~ "ǅ"])
	printf("\tǅ ~ ǆ: expect false, got %s\n", tf["ǅ " ~ "ǆ"])
}
