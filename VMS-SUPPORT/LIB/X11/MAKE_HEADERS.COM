$! Native equivalents of include/Imakefile and lib/X11/Imakefile header rules.
$! This static profile has neither XTHREADS nor XUSE_MTSAFE_API enabled.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ input_open = 0
$ output_open = 0
$ mode = F$EDIT(P1,"UPCASE,TRIM")
$ IF (mode .NES. "XPOLL") .AND. (mode .NES. "XLIBCONF") THEN GOTO done
$ IF (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ IF (P4 .NES. "") .OR. (P5 .NES. "") .OR. (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF F$SEARCH(P2) .EQS. "" THEN GOTO done
$ temporary = F$PARSE(P3,,,"DEVICE") + F$PARSE(P3,,,"DIRECTORY") + -
    F$PARSE(P3,,,"NAME") + "_" + F$GETJPI("","PID") + ".TMP"
$ OPEN/WRITE generated 'temporary'
$ output_open = 1
$ IF mode .EQS. "XLIBCONF" THEN GOTO xlibconf
$ OPEN/READ source 'P2'
$ input_open = 1
$ replacements = 0
$ token = "@USE_FDS_BITS@"
$next_line:
$ READ/END=template_done source record
$replace_token:
$ position = F$LOCATE(token,record)
$ IF position .EQ. F$LENGTH(record) THEN GOTO write_line
$ tail = position + F$LENGTH(token)
$ record = F$EXTRACT(0,position,record) + "fds_bits" + -
    F$EXTRACT(tail,F$LENGTH(record)-tail,record)
$ replacements = replacements + 1
$ GOTO replace_token
$write_line:
$ WRITE generated record
$ GOTO next_line
$template_done:
$ CLOSE source
$ input_open = 0
$ IF replacements .NE. 1 THEN GOTO done
$ GOTO publish
$xlibconf:
$ WRITE generated "/* Defines needed to use Xlib from non-imake projects */"
$ WRITE generated "#undef XTHREADS"
$ WRITE generated "#undef XUSE_MTSAFE_API"
$publish:
$ CLOSE generated
$ output_open = 0
$ candidate = F$SEARCH(temporary)
$ IF candidate .EQS. "" THEN GOTO done
$ RENAME/NOLOG 'candidate' 'P3'
$ result = $STATUS
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$ GOTO done
$interrupted:
$ result = 2
$done:
$ SET NOON
$ IF input_open THEN CLOSE source
$ IF output_open THEN CLOSE generated
$ EXIT result
