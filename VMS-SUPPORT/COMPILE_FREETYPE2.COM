$! Original FreeType aggregate action: source, output base, flags, source group.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ scoped = 0
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ IF (P5 .NES. "") .OR. (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ sources = "|BASE/FTAPI|BASE/FTBASE|BASE/FTBBOX|BASE/FTBDF|BASE/FTDEBUG|BASE/FTGLYPH|BASE/FTINIT|BASE/FTMM|BASE/FTPFR|BASE/FTSTROKE|BASE/FTSYNTH|BASE/FTSYSTEM|BASE/FTTYPE1|BASE/FTWINFNT|BASE/FTXF86|AUTOFIT/AUTOFIT|AUTOHINT/AUTOHINT|BDF/BDF|CFF/CFF|CID/TYPE1CID|GZIP/FTGZIP|LZW/FTLZW|PCF/PCF|PFR/PFR|PSAUX/PSAUX|PSHINTER/PSHINTER|PSNAMES/PSNAMES|RASTER/RASTER|SFNT/SFNT|SMOOTH/SMOOTH|TRUETYPE/TRUETYPE|TYPE1/TYPE1|TYPE42/TYPE42|WINFONTS/WINFNT|"
$ group = F$EDIT(P4,"UPCASE")
$ module = F$EDIT(F$PARSE(P1,,,"NAME"),"UPCASE")
$ key = "|" + group + "/" + module + "|"
$ IF F$LOCATE(key,sources) .EQ. F$LENGTH(sources) THEN GOTO done
$ source_directory = "[.EXTRAS.FREETYPE.SRC." + group + "]"
$ IF F$EDIT(F$PARSE(P1,,,"DEVICE"),"UPCASE") .NES. -
     F$EDIT(F$PARSE(source_directory,,,"DEVICE"),"UPCASE") THEN GOTO done
$ IF F$EDIT(F$PARSE(P1,,,"DIRECTORY"),"UPCASE") .NES. -
     F$EDIT(F$PARSE(source_directory,,,"DIRECTORY"),"UPCASE") THEN GOTO done
$ IF F$EDIT(F$PARSE(P1,,,"TYPE"),"UPCASE") .NES. ".C" THEN GOTO done
$ rtl_headers = F$SEARCH("SYS$LIBRARY:DECC$RTLDEF.TLB")
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]FT2BUILD.H") .EQS. "" THEN GOTO done
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2.FREETYPE]FREETYPE.H")
$ IF header .EQS. "" THEN GOTO done
$ headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ inherited_freetype = F$TRNLNM("FREETYPE")
$ inherited_table = F$TRNLNM("FREETYPE",,,,,"TABLE_NAME")
$ IF F$TRNLNM("FREETYPE","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$! Group headers/bodies precede exports, including gzip's modified zlib headers.
$ include_directory = source_directory + "," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2.FREETYPE.CONFIG],[.EXTRAS.FREETYPE.SRC]"
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS FREETYPE 'headers'
$ scoped = 1
$ IF F$TRNLNM("FREETYPE","LNM$FILE_DEV") .NES. headers THEN GOTO done
$ CC 'P3' /NOFIRST_INCLUDE/NESTED_INCLUDE_DIRECTORY=NONE -
    /INCLUDE_DIRECTORY=('include_directory',"") -
    /LIST='P2'.LIS/SHOW=INCLUDE -
    /MMS_DEPENDENCIES=(FILE='P2'.MMS,SYSTEM_INCLUDE_FILES) -
    /OBJECT='P2'.OBJ 'P1' + 'rtl_headers'/LIBRARY
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ IF F$TRNLNM("FREETYPE") .NES. inherited_freetype THEN result = 2
$ IF F$TRNLNM("FREETYPE",,,,,"TABLE_NAME") .NES. inherited_table THEN result = 2
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ IF scoped .AND. (F$TRNLNM("FREETYPE","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS FREETYPE
$ IF .NOT. result THEN WRITE SYS$OUTPUT "FreeType compile failed: " + P1
$ EXIT result
