$! Link the unchanged upstream converter with its explicit native font objects.
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
$ base = "[.VMS-SUPPORT._BUILD.P64.PROGRAMS.BDFTOPCF]"
$ bitmap = "[.VMS-SUPPORT._BUILD.P64.LIB.FONT.BITMAP]"
$ LINK/NOTRACEBACK/FULL/MAP='base'BDFTOPCF.MAP/EXE='base'BDFTOPCF.EXE -
    'base'BDFTOPCF.OBJ, -
    'bitmap'BDFREAD.OBJ,'bitmap'BDFUTILS.OBJ,'bitmap'BITMAP.OBJ, -
    'bitmap'BITMAPUTIL.OBJ,'bitmap'FONTINK.OBJ,'bitmap'PCFREAD.OBJ, -
    'bitmap'PCFWRITE.OBJ,[.VMS-SUPPORT._BUILD.P64.LIB.FONT.FONTFILE]DEFAULTS.OBJ, -
    [.VMS-SUPPORT._BUILD.P64.LIB.FONTIO]FONTIO.OLB/LIBRARY, -
    [.VMS-SUPPORT._BUILD.P64.LIB.FONTUTIL]FONTUTIL.OLB/LIBRARY, -
    [.VMS-SUPPORT._BUILD.P64.LIB.ZLIB]Z.OLB/LIBRARY
$ result = $STATUS
$ GOTO done
$libraries:
$ WRITE SYS$OUTPUT "BDFTOPCF requires no LNK$LIBRARY search-list overrides."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ EXIT result
