BEGIN{
    $0 = "AAAAAAAABBBBBBBBCCCCCCCC"
    x = $$$B++ ""*""+$$B++N
    print "expect 12, got:", x
}

# BEGIN{
#     $0 = "AAAAAAAABBBBBBBBCCCCCCCC"
# #    x = $$$B++ ""*""+$$B++N
# #    print "Leaked Data:", x
#     s1_2 = $B++ ; printf "s1_2 = <%s>\n", s1_2
#     s3 = $B ; printf "s3 = <%s>\n", s3
#     s4 = $B ; printf "s4 = <%s>\n", s4
# 
#     s5 = "" * "" ; printf "s5 = <%s>\n", s5
# 
#     s6_7 = $B++ ; printf "s6_7 = <%s>\n", s6_7
#     s8 = $B ; printf "s8 = <%s>\n", s8
# 
#     printf("$1 = <%s> $2 = <%s>\n", $1, $1)
# 
#     s9 = s5 + s8 ; printf "s9 = <%s>\n", s9
#     s10 = N; printf "s10 = <%s>\n", s10
# 
#     s11 = s4 s9 s10; printf "s11 = %s\n", s11
# }

# 	# BEGIN
# 
# [     1:0x570d70f12a68] Op_rule             : [in_rule = BEGIN] [source_file = leak.awk]
# [     2:0x570d70f10370] Op_push_i           : "AAAAAAAABBBBBBBBCCCCCCCC" [MALLOC|STRING|STRCUR]
# [     2:0x570d70f102f8] Op_push_i           : 0 [MALLOC|NUMCUR|NUMBER|NUMINT]
# [     2:0x570d70f102d0] Op_store_field      : 
# [     3:0x570d70f10460] Op_push             : B
# [     3:0x570d70f10438] Op_field_spec_lhs   : [target_assign = 0x570d70f104d8] [do_reference = true]
# [     3:0x570d70f10488] Op_postincrement    : 
# [      :0x570d70f104d8] Op_field_assign     : [invalidate_field0()]
# [     3:0x570d70f10410] Op_field_spec       : 
# [     3:0x570d70f103e8] Op_field_spec       : 
# [     3:0x570d70f10500] Op_push_i           : "" [MALLOC|STRING|STRCUR]
# [     3:0x570d70f10550] Op_times_i          : "" [MALLOC|STRING|STRCUR]
# [     3:0x570d70f105f0] Op_push             : B
# [     3:0x570d70f10578] Op_field_spec_lhs   : [target_assign = 0x570d70f10668] [do_reference = true]
# [     3:0x570d70f10618] Op_postincrement    : 
# [      :0x570d70f10668] Op_field_assign     : [invalidate_field0()]
# [     3:0x570d70f105a0] Op_field_spec       : 
# [     3:0x570d70f105c8] Op_plus             : 
# [      :0x570d70f10640] Op_no_op            : 
# [     3:0x570d70f10690] Op_push             : N
# [      :0x570d70f106b8] Op_concat           : [expr_count = 3] [concat_flag = 0]
# [     3:0x570d70f10348] Op_store_var        : x
# [     4:0x570d70f104b0] Op_push_i           : "Leaked Data:" [MALLOC|STRING|STRCUR]
# [     4:0x570d70f106e0] Op_push             : x
# [     4:0x570d70f10320] Op_K_print          : [expr_count = 2] [redir_type = ""]
# [      :0x570d70f10208] Op_no_op            : 
# [      :0x570d70f102a8] Op_atexit           : 
# [      :0x570d70f103c0] Op_stop             : 
# [      :0x570d70f10258] Op_no_op            : 
# [      :0x570d70f10280] Op_after_beginfile  : 
# [      :0x570d70f10230] Op_no_op            : 
# [      :0x570d70f10730] Op_after_endfile    : 
