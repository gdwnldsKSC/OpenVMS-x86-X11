$! Original single-object action: source, output base, flags, optional header profile.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ scoped = 0
$ dps_scoped = 0
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ profiles = "||ZLIB|FONTENC|EXPAT|LBXUTIL_IMAGE|LBXUTIL|FONT_UTIL|FNTSTUBS|FONT_BITMAP|FONT_FONTFILE|FONT_FC|FS|PSRES|REGEX|REGEX_MKH|ICE|SM|BDFTOPCF|ICEAUTH|MAKEPSRES|SHOWRGB|MAKEDEPEND|"
$ IF F$LOCATE("|",P4) .NE. F$LENGTH(P4) THEN GOTO done
$ IF F$LOCATE("|" + P4 + "|",profiles) .EQ. F$LENGTH(profiles) THEN GOTO done
$ IF (P5 .NES. "") .OR. (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ rtl_headers = F$SEARCH("SYS$LIBRARY:DECC$RTLDEF.TLB")
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF P4 .EQS. "ZLIB" THEN GOTO zlib_headers
$ IF P4 .EQS. "FONTENC" THEN GOTO fontenc_headers
$ IF (P4 .EQS. "REGEX") .OR. (P4 .EQS. "REGEX_MKH") THEN GOTO regex_headers
$ inherited_x11 = F$TRNLNM("X11")
$ inherited_table = F$TRNLNM("X11",,,,,"TABLE_NAME")
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]"
$ IF P4 .EQS. "EXPAT" THEN include_directory = include_directory + -
    ",[.EXTRAS.EXPAT.LIB],[.LIB.EXPAT]"
$ IF P4 .EQS. "LBXUTIL_IMAGE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.LBXUTIL.IMAGE],[.LIB.LBXUTIL.IMAGE]"
$ IF P4 .EQS. "LBXUTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.PROGRAMS.XSERVER.INCLUDE],[.LIB.LBXUTIL.LBX_ZLIB]"
$ IF P4 .EQS. "FONT_UTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.FONT.INCLUDE]"
$ IF P4 .EQS. "FNTSTUBS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.STUBS]"
$ IF P4 .EQS. "FONT_FC" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.FC],[.LIB.XTRANS]"
$ IF P4 .EQS. "FS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.FS],[.LIB.XTRANS]"
$ IF P4 .EQS. "ICE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.ICE],[.LIB.ICE],[.LIB.XTRANS]"
$ IF P4 .EQS. "SM" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.SM],[.LIB.SM]"
$ IF P4 .EQS. "ICEAUTH" THEN include_directory = include_directory + -
    ",[.PROGRAMS.ICEAUTH]"
$ IF P4 .EQS. "MAKEPSRES" THEN include_directory = include_directory + -
    ",[.PROGRAMS.MAKEPSRES]"
$ IF P4 .EQS. "SHOWRGB" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.INCLUDE]"
$ IF P4 .EQS. "MAKEDEPEND" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11],[.CONFIG.MAKEDEPEND],[.CONFIG.IMAKE]"
$ IF P4 .EQS. "FONT_FONTFILE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.FONTFILE]"
$ IF (P4 .EQS. "FONT_BITMAP") .OR. (P4 .EQS. "BDFTOPCF") THEN -
    include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.BITMAP]"
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XFUNCPROTO.H")
$ IF header .EQS. "" THEN GOTO done
$ headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS X11 'headers'
$ scoped = 1
$ IF F$TRNLNM("X11","LNM$FILE_DEV") .NES. headers THEN GOTO done
$ IF P4 .EQS. "PSRES" THEN GOTO psres_headers
$ GOTO compile
$psres_headers:
$ inherited_dps = F$TRNLNM("DPS")
$ inherited_dps_table = F$TRNLNM("DPS",,,,,"TABLE_NAME")
$ IF F$TRNLNM("DPS","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.DPS]PSRES.H")
$ IF header .EQS. "" THEN GOTO done
$ dps_headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS DPS 'dps_headers'
$ dps_scoped = 1
$ IF F$TRNLNM("DPS","LNM$FILE_DEV") .NES. dps_headers THEN GOTO done
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
$ GOTO compile
$regex_headers:
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.REGEX],[.EXTRAS.REGEX]"
$ IF (P4 .EQS. "REGEX") .AND. -
    (F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]REGEX.H") .EQS. "") THEN GOTO done
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
$ IF .NOT. dps_scoped THEN GOTO done
$ IF F$TRNLNM("DPS") .NES. inherited_dps THEN result = 2
$ IF F$TRNLNM("DPS",,,,,"TABLE_NAME") .NES. inherited_dps_table THEN result = 2
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ IF dps_scoped .AND. (F$TRNLNM("DPS","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS DPS
$ IF scoped .AND. (F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS X11
$ IF .NOT. result THEN WRITE SYS$OUTPUT "X11 component compile failed: " + P1
$ EXIT result
