$! Keep ODS-5 dotted locale directory names out of MMS's CDD syntax.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO failed
$ result = 2
$ list_open = 0
$ IF (P1 .NES. "XLC_LOCALE") .AND. (P1 .NES. "COMPOSE") THEN GOTO done
$ IF P2 .EQS. "" THEN GOTO done
$ IF (P3 .NES. "") .OR. (P4 .NES. "") .OR. (P5 .NES. "") .OR. -
     (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ prefix = F$EXTRACT(0,F$LENGTH(P2)-1,P2) + "."
$ option = ""
$ IF P1 .EQS. "XLC_LOCALE" THEN option = "WCHAR32"
$ OPEN/READ list_file [.VMS-SUPPORT.NLS]'P1'.LIST
$ list_open = 1
$ next_locale:
$ READ/END=complete list_file locale
$ directory = ""
$ index = 0
$ escape_loop:
$ IF index .EQ. F$LENGTH(locale) THEN GOTO escaped
$ letter = F$EXTRACT(index,1,locale)
$ IF letter .EQS. "." THEN directory = directory + "^"
$ directory = directory + letter
$ index = index + 1
$ GOTO escape_loop
$ escaped:
$ source = "[.LIB.LIBX11.NLS." + directory + "]" + P1 + ".PRE"
$ destination = prefix + directory + "]" + P1 + "."
$ @[.VMS-SUPPORT.NLS]GENERATE.COM 'source' 'destination' "''option'"
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ GOTO next_locale
$ complete:
$ result = 1
$ GOTO done
$ failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$ done:
$ IF list_open THEN CLOSE list_file
$ EXIT result
