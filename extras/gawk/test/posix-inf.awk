function init_inf_nan_constants()
{
	return CONST_NEG_NAN = -(CONST_POS_NAN = 0 * \
		(CONST_NEG_INF =  (CONST_POS_INF = 0^-1) * -1))
}
BEGIN {
	init_inf_nan_constants()
	printf("%g | %g | %g | %g \n",
		CONST_POS_INF, CONST_NEG_INF,
		CONST_POS_NAN, CONST_NEG_NAN)
}
