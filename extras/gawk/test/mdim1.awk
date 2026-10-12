BEGIN {
	print "typeof(y) is", typeof(y)
	z = y
	print "typeof(y) is", typeof(y)
	print "typeof(z) is", typeof(z)

	print "typeof(x[0]) is", typeof(x[0])
	x[1] = x[0]
	print "typeof(x[0]) is", typeof(x[0])
	print "typeof(x[1]) is", typeof(x[1])
	x[2][1] = "xx"
	print "typeof(x[2]) is", typeof(x[2])
	print "typeof(x[2][1]) is", typeof(x[2][1])

	x[1][0] = "xx"	# this will fatal
	print x[1][0]
}
