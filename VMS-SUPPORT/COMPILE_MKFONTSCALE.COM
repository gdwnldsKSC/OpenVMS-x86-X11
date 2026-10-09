$! Original mkfontscale action: public source, output base, flags.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ x11_scoped = 0
$ freetype_scoped = 0
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ IF (P4 .NES. "") .OR. (P5 .NES. "") .OR. (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ sources = "|MKFONTSCALE|LIST|HASH|IDENT|"
$ module = F$EDIT(F$PARSE(P1,,,"NAME"),"UPCASE")
$ IF F$LOCATE("|" + module + "|",sources) .EQ. F$LENGTH(sources) THEN GOTO done
$ source_directory = "[.APP.MKFONTSCALE]"
$ IF F$EDIT(F$PARSE(P1,,,"DEVICE"),"UPCASE") .NES. -
     F$EDIT(F$PARSE(source_directory,,,"DEVICE"),"UPCASE") THEN GOTO done
$ IF F$EDIT(F$PARSE(P1,,,"DIRECTORY"),"UPCASE") .NES. -
     F$EDIT(F$PARSE(source_directory,,,"DIRECTORY"),"UPCASE") THEN GOTO done
$ IF F$EDIT(F$PARSE(P1,,,"TYPE"),"UPCASE") .NES. ".C" THEN GOTO done
$ rtl_headers = F$SEARCH("SYS$LIBRARY:DECC$RTLDEF.TLB")
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]FT2BUILD.H") .EQS. "" THEN GOTO done
$ x11_header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XFUNCPROTO.H")
$ IF x11_header .EQS. "" THEN GOTO done
$ x11_headers = F$PARSE(x11_header,,,"DEVICE") + F$PARSE(x11_header,,,"DIRECTORY")
$ freetype_header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2.FREETYPE]FREETYPE.H")
$ IF freetype_header .EQS. "" THEN GOTO done
$ freetype_headers = F$PARSE(freetype_header,,,"DEVICE") + -
    F$PARSE(freetype_header,,,"DIRECTORY")
$ inherited_x11 = F$TRNLNM("X11")
$ inherited_x11_table = F$TRNLNM("X11",,,,,"TABLE_NAME")
$ inherited_freetype = F$TRNLNM("FREETYPE")
$ inherited_freetype_table = F$TRNLNM("FREETYPE",,,,,"TABLE_NAME")
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ IF F$TRNLNM("FREETYPE","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2.FREETYPE.CONFIG]," + -
    source_directory
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS X11 'x11_headers'
$ x11_scoped = 1
$ IF F$TRNLNM("X11","LNM$FILE_DEV") .NES. x11_headers THEN GOTO done
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS FREETYPE 'freetype_headers'
$ freetype_scoped = 1
$ IF F$TRNLNM("FREETYPE","LNM$FILE_DEV") .NES. freetype_headers THEN GOTO done
$ CC 'P3' /NOFIRST_INCLUDE/NESTED_INCLUDE_DIRECTORY=NONE -
    /INCLUDE_DIRECTORY=('include_directory',"") -
    /LIST='P2'.LIS/SHOW=INCLUDE -
    /MMS_DEPENDENCIES=(FILE='P2'.MMS,SYSTEM_INCLUDE_FILES) -
    /OBJECT='P2'.OBJ 'P1' + 'rtl_headers'/LIBRARY
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ IF F$TRNLNM("X11") .NES. inherited_x11 THEN result = 2
$ IF F$TRNLNM("X11",,,,,"TABLE_NAME") .NES. inherited_x11_table THEN result = 2
$ IF F$TRNLNM("FREETYPE") .NES. inherited_freetype THEN result = 2
$ IF F$TRNLNM("FREETYPE",,,,,"TABLE_NAME") .NES. inherited_freetype_table THEN result = 2
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ IF freetype_scoped .AND. -
     (F$TRNLNM("FREETYPE","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS FREETYPE
$ IF x11_scoped .AND. (F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS X11
$ IF .NOT. result THEN WRITE SYS$OUTPUT "mkfontscale compile failed: " + P1
$ EXIT result
