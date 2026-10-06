$! Link upstream xmodmap against the native P64 client libraries.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF (P1 .NES. "") .OR. (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF F$TRNLNM("LNK$LIBRARY") .NES. "" THEN GOTO libraries
$ index = 1
$check_libraries:
$ IF F$TRNLNM("LNK$LIBRARY_" + F$STRING(index)) .NES. "" THEN GOTO libraries
$ index = index + 1
$ IF index .LE. 999 THEN GOTO check_libraries
$ base = "[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XMODMAP]"
$ LINK/NOTRACEBACK/FULL/MAP='base'XMODMAP.MAP/EXE='base'XMODMAP.EXE -
    'base'XMODMAP.OBJ, -
    'base'PF.OBJ, -
    'base'HANDLE.OBJ, -
    'base'EXEC.OBJ, -
    [.VMS-SUPPORT._BUILD.P64.LIB.X11]X11.OLB/LIBRARY/INCLUDE=GLOBALS
$ result = $STATUS
$ GOTO done
$libraries:
$ WRITE SYS$OUTPUT "XMODMAP requires no LNK$LIBRARY search-list overrides."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ EXIT result
