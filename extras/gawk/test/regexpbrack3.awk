BEGIN {
	print("12.4alpha9" ~ /^.*[^0-9]/ ? "match" : "nomatch")
}
