$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO failed
$ input_open = 0
$ output_open = 0
$ result = 2
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ OPEN/READ source 'P1'
$ input_open = 1
$ OPEN/WRITE target 'P2'
$ output_open = 1
$read_line:
$ READ/END=complete source line
$ index = 0
$replace:
$ kind = F$ELEMENT(index,",","MAJOR,MINOR,MICRO")
$ IF kind .EQS. "," THEN GOTO write_line
$ token = "@PIXMAN_VERSION_" + kind + "@"
$ position = F$LOCATE(token,line)
$ IF position .EQ. F$LENGTH(line) THEN GOTO next_kind
$ value = F$ELEMENT(index,".",P3)
$ line = F$EXTRACT(0,position,line) + value + -
    F$EXTRACT(position+F$LENGTH(token),F$LENGTH(line),line)
$ GOTO replace
$next_kind:
$ index = index + 1
$ GOTO replace
$write_line:
$ WRITE target line
$ GOTO read_line
$complete:
$ result = 1
$ GOTO done
$failed:
$ result = 2
$done:
$ SET NOON
$ IF input_open THEN CLOSE source
$ IF output_open THEN CLOSE target
$ EXIT result
