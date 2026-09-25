$! Original single-object action: source, output base, flags, optional header profile.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ scoped = 0
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ IF (P4 .NES. "") .AND. (P4 .NES. "ZLIB") .AND. (P4 .NES. "FONTENC") .AND. -
     (P4 .NES. "EXPAT") .AND. (P4 .NES. "LBX_IMAGE") .AND. (P4 .NES. "LBXUTIL") THEN GOTO done
$ IF (P5 .NES. "") .OR. (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ rtl_headers = F$SEARCH("SYS$LIBRARY:DECC$RTLDEF.TLB")
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF P4 .EQS. "ZLIB" THEN GOTO zlib_headers
$ IF P4 .EQS. "FONTENC" THEN GOTO fontenc_headers
$ inherited_x11 = F$TRNLNM("X11")
$ inherited_table = F$TRNLNM("X11",,,,,"TABLE_NAME")
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]"
$ IF P4 .EQS. "EXPAT" THEN include_directory = include_directory + -
    ",[.EXTRAS.EXPAT.LIB],[.LIB.EXPAT]"
$ IF P4 .EQS. "LBX_IMAGE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.LBX_IMAGE],[.LIB.LBXUTIL.IMAGE]"
$ IF P4 .EQS. "LBXUTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.PROGRAMS.XSERVER.INCLUDE],[.LIB.LBXUTIL.LBX_ZLIB]"
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XFUNCPROTO.H")
$ IF header .EQS. "" THEN GOTO done
$ headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS X11 'headers'
$ scoped = 1
$ IF F$TRNLNM("X11","LNM$FILE_DEV") .NES. headers THEN GOTO done
$ GOTO compile
$zlib_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]ZLIB.H") .EQS. "" THEN GOTO done
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]ZCONF.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE],[.EXTRAS.ZLIB]"
$ GOTO compile
$fontenc_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]FONTENC.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.FONT.FONTFILE]"
$compile:
$ CC 'P3' /NOFIRST_INCLUDE/NESTED_INCLUDE_DIRECTORY=NONE -
    /INCLUDE_DIRECTORY=('include_directory',"") -
    /LIST='P2'.LIS/SHOW=INCLUDE -
    /MMS_DEPENDENCIES=(FILE='P2'.MMS,SYSTEM_INCLUDE_FILES) -
    /OBJECT='P2'.OBJ 'P1' + 'rtl_headers'/LIBRARY
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ IF .NOT. scoped THEN GOTO done
$ IF F$TRNLNM("X11") .NES. inherited_x11 THEN result = 2
$ IF F$TRNLNM("X11",,,,,"TABLE_NAME") .NES. inherited_table THEN result = 2
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ IF scoped .AND. (F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS X11
$ IF .NOT. result THEN WRITE SYS$ERROR "X11 component compile failed: " + P1
$ EXIT result
