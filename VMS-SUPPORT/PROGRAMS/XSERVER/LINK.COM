$! Link a caller-built DDX object/options file with the public P64 server core.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO usage
$ IF (P4 .NES. "") .OR. (P5 .NES. "") .OR. (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
$ ddx_input = F$SEARCH(P1)
$ IF ddx_input .EQS. "" THEN GOTO usage
$ input_type = F$EDIT(F$PARSE(ddx_input,,,"TYPE"),"UPCASE")
$ IF (input_type .NES. ".OBJ") .AND. (input_type .NES. ".OPT") THEN GOTO usage
$ image = P2
$ map = P3
$ IF F$EDIT(F$PARSE(image,,,"TYPE"),"UPCASE") .NES. ".EXE" THEN GOTO usage
$ IF F$EDIT(F$PARSE(map,,,"TYPE"),"UPCASE") .NES. ".MAP" THEN GOTO usage
$ IF F$TRNLNM("LNK$LIBRARY") .NES. "" THEN GOTO libraries
$ index = 1
$check_libraries:
$ IF F$TRNLNM("LNK$LIBRARY_" + F$STRING(index)) .NES. "" THEN GOTO libraries
$ index = index + 1
$ IF index .LE. 999 THEN GOTO check_libraries
$ input_qualifier = ""
$ IF input_type .EQS. ".OPT" THEN input_qualifier = "/OPTIONS"
$ LINK/NOTRACEBACK/FULL/MAP='map'/EXE='image' -
    'ddx_input''input_qualifier', -
    [.VMS-SUPPORT.PROGRAMS.XSERVER]CORE.OPT/OPTIONS
$ result = $STATUS
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Usage: @LINK DDX.OBJ-or-DDX.OPT output.EXE output.MAP"
$ GOTO done
$libraries:
$ WRITE SYS$OUTPUT "XSERVER requires no LNK$LIBRARY search-list overrides."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ EXIT result
