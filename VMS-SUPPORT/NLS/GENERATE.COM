$! Preprocess the upstream data, then apply CppSedMagic/R63Compat.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ parse_style = F$GETJPI("","PARSE_STYLE_PERM")
$ SET PROCESS/PARSE_STYLE=EXTENDED
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") THEN GOTO done
$ IF (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF (P3 .NES. "") .AND. (P3 .NES. "WCHAR32") THEN GOTO done
$ IF (P4 .NES. "") .AND. (P4 .NES. "R63") THEN GOTO done
$ source = F$SEARCH(P1)
$ IF source .EQS. "" THEN GOTO done
$ filter = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.NLS]NLSFILTER.EXE")
$ IF filter .EQS. "" THEN GOTO done
$ destination = F$PARSE(P2,,,"DEVICE","SYNTAX_ONLY") + F$PARSE(P2,,,"DIRECTORY","SYNTAX_ONLY")
$ name = F$PARSE(P2,,,"NAME","SYNTAX_ONLY")
$ type = F$PARSE(P2,,,"TYPE","SYNTAX_ONLY")
$ raw = destination + name + "_" + F$EXTRACT(1,F$LENGTH(type),type) + "_CPP.TMP"
$ CREATE/DIRECTORY 'destination'
$ defines = ""
$ IF P3 .EQS. "WCHAR32" THEN defines = "/DEFINE=(WCHAR32=1)"
$ CC/PREPROCESS_ONLY='raw' 'defines' 'source'
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ nls_filter :== $'filter'
$ IF P4 .EQS. "R63" THEN GOTO compatibility
$ nls_filter "''raw'" "''P2'"
$ result = $STATUS
$ GOTO done
$ compatibility:
$ nls_filter "''raw'" "''P2'" "R63"
$ result = $STATUS
$ GOTO done
$ interrupted:
$ result = 2
$ GOTO done
$ failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$ done:
$ SET PROCESS/PARSE_STYLE='parse_style'
$ EXIT result
