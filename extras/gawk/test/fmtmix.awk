# input is
# 5 3
{
	CONVFMT="%.3f"
	OFMT="%03e"
	print $1	# expect: 5
	$0 = $1/$2
	print		# expect: 1.666667e+00
	print "" $1	# expect: 1.667
	CONVFMT="%.0f"
	OFMT="%.0e"
	print $1	# expect 2 - printf "%.0f\n"  1.666667 ---> 2
	# The above is a little surprising. The flow is that $1 gets
	# the numeric value 1.666667 from $0, and is converted via CONVFMT
	# to get a string value, and that's what's printed.  This is
	# definitely dark corner territory.
	print		# expect: 2e+00
}
