$! Generate the upstream yacc grammar with a caller-configured GNU Bison.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ grammar_open = 0
$ notice_open = 0
$ IF (P1 .NES. "") .OR. (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF F$TYPE(BISON) .EQS. "" THEN GOTO missing
$ base = "[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XKBCOMP]"
$ target = base + "XKBPARSE.C"
$ raw = base + "XKBPARSE_RAW.C"
$ notice = base + "XKBPARSE_NOTICE.C"
$ grammar = "[.PROGRAMS.XKBCOMP]XKBPARSE.Y"
$ IF F$SEARCH(grammar) .EQS. "" THEN GOTO done
$ BISON "-l" "-o" 'raw' 'grammar'
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ result = 2
$ OPEN/READ grammar_file 'grammar'
$ grammar_open = 1
$ CREATE/FDL=SYS$INPUT 'notice'
RECORD
 FORMAT STREAM_LF
 CARRIAGE_CONTROL CARRIAGE_RETURN
$ OPEN/APPEND notice_file 'notice'
$ notice_open = 1
$notice_loop:
$ READ/END=done grammar_file line
$ IF F$EXTRACT(0,1,line) .EQS. "%" THEN GOTO notice_done
$ WRITE notice_file line
$ GOTO notice_loop
$notice_done:
$ CLOSE grammar_file
$ grammar_open = 0
$ CLOSE notice_file
$ notice_open = 0
$ COPY/NOLOG/CONCATENATE 'notice','raw' 'target'
$ result = $STATUS
$ IF F$SEARCH(target) .EQS. "" THEN result = 2
$ GOTO done
$missing:
$ WRITE SYS$OUTPUT "XKBCOMP parser generation requires a BISON foreign command."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ IF grammar_open THEN CLOSE grammar_file
$ IF notice_open THEN CLOSE notice_file
$ EXIT result
