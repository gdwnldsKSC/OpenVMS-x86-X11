$! Run the unchanged upstream table generator with the Imakefile's default options.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ redirected = 0
$ IF (P1 .NES. "") .OR. (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF F$TRNLNM("LNK$LIBRARY") .NES. "" THEN GOTO libraries
$ index = 1
$check_libraries:
$ IF F$TRNLNM("LNK$LIBRARY_" + F$STRING(index)) .NES. "" THEN GOTO libraries
$ index = index + 1
$ IF index .LE. 999 THEN GOTO check_libraries
$ IF F$TRNLNM("SYS$OUTPUT","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ base = "[.VMS-SUPPORT._BUILD.P64.LIB.LBX_IMAGE]"
$ LINK/NOTRACEBACK/FULL/MAP='base'MKG3STATES.MAP/EXE='base'MKG3STATES.EXE -
    'base'MKG3STATES.OBJ
$ temporary = base + "G3STATES_" + F$GETJPI("","PID") + ".TMP"
$ DEFINE/USER_MODE SYS$OUTPUT 'temporary'
$ redirected = 1
$ RUN 'base'MKG3STATES.EXE
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ RENAME/NOLOG 'temporary' 'base'G3STATES.H
$ result = $STATUS
$ GOTO done
$libraries:
$ WRITE SYS$ERROR "LBX table generator requires no LNK$LIBRARY search-list overrides."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ IF redirected .AND. (F$TRNLNM("SYS$OUTPUT","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE SYS$OUTPUT
$ EXIT result
