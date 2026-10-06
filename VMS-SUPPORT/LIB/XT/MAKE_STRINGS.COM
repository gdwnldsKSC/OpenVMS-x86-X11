$! Run upstream makestrs using its default Unix string-table ABI.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ IF (P1 .NES. "") .OR. (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ generator = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.CONFIG.UTIL]MAKESTRS.EXE")
$ input = F$SEARCH("[.LIB.XT.UTIL]STRING.LIST")
$ IF (generator .EQS. "") .OR. (input .EQS. "") THEN GOTO done
$ base = F$PARSE("[.VMS-SUPPORT._BUILD.P64.LIB.XT]")
$ base = F$PARSE(base,,,"DEVICE") + F$PARSE(base,,,"DIRECTORY")
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.LIB.XT]UTIL.DIR") .EQS. "" -
     THEN CREATE/DIRECTORY [.VMS-SUPPORT._BUILD.P64.LIB.XT.UTIL]
$ COPY/NOLOG [.LIB.XT.UTIL]STRDEFS.CT [.VMS-SUPPORT._BUILD.P64.LIB.XT.UTIL]STRDEFS.CT
$ COPY/NOLOG [.LIB.XT.UTIL]STRDEFS.HT [.VMS-SUPPORT._BUILD.P64.LIB.XT.UTIL]STRDEFS.HT
$ COPY/NOLOG [.LIB.XT.UTIL]SHELL.HT [.VMS-SUPPORT._BUILD.P64.LIB.XT.UTIL]SHELL.HT
$ SET DEFAULT 'base'
$ DEFINE/USER_MODE SYS$INPUT 'input'
$ DEFINE/USER_MODE SYS$OUTPUT STRINGDEFS.C
$ RUN 'generator'
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ IF F$SEARCH("STRINGDEFS.C") .EQS. "" THEN GOTO interrupted
$ IF F$SEARCH("STRINGDEFS.H") .EQS. "" THEN GOTO interrupted
$ IF F$SEARCH("SHELL.H") .EQS. "" THEN GOTO interrupted
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET DEFAULT 'saved_default'
$ EXIT result
