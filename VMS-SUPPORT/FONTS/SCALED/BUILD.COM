$! Stage the unchanged upstream scalable fonts and all accompanying notices.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ section = F$EDIT(P1,"UPCASE,TRIM")
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF (section .NES. "SPEEDO") .AND. (section .NES. "TYPE1") .AND. -
     (section .NES. "TTF") .AND. (section .NES. "OTF") .AND. (section .NES. "CID") THEN GOTO done
$ executable = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.PROGRAMS.MKFONTSCALE]MKFONTSCALE.EXE")
$ IF executable .EQS. "" THEN GOTO done
$ mkfontscale = "$" + executable
$ output = "[.VMS-SUPPORT._BUILD.P64.FONTS." + section + "]"
$ GOSUB stage_'section'
$ SET DEFAULT 'output'
$ IF section .EQS. "SPEEDO" THEN GOTO fonts_dir
$ ignored = F$SEARCH("",2)
$ previous = F$SEARCH("FONTS.SCALE;0",2)
$ DEFINE/USER_MODE FONT_ENCODINGS_DIRECTORY "../encodings/encodings.dir"
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ mkfontscale -r -e "../encodings" -e "../encodings/large" "."
$ ignored = F$SEARCH("",2)
$ current = F$SEARCH("FONTS.SCALE;0",2)
$ GOSUB check_index
$fonts_dir:
$ ignored = F$SEARCH("",2)
$ previous = F$SEARCH("FONTS.DIR;0",2)
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ mkfontscale -b -s -l -r -e "../encodings" -e "../encodings/large" "."
$ ignored = F$SEARCH("",2)
$ current = F$SEARCH("FONTS.DIR;0",2)
$ GOSUB check_index
$ WRITE SYS$OUTPUT "Scaled ", section, ": ", count, " font names; index ", current
$ result = 1
$ GOTO done
$stage_SPEEDO:
$ COPY/NOLOG [.FONTS.SCALED.SPEEDO]*.SPD;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.SPEEDO]FONTS.SCALE 'output'FONTS.SCALE
$ COPY/NOLOG [.FONTS.SCALED.SPEEDO]COPYRIGHT. 'output'COPYRIGHT.
$ RETURN
$stage_TYPE1:
$ COPY/NOLOG [.FONTS.SCALED.TYPE1]*.PFA;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.TYPE1]*.PFB;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.TYPE1]*.AFM;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.TYPE1]COPYRIGHT.*;0 'output'
$ RETURN
$stage_TTF:
$ COPY/NOLOG [.FONTS.SCALED.TTF]*.TTF;0 'output'
$ COPY/NOLOG [.EXTRAS.FONTS.BITSTREAM-VERA]*.TTF;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.ETHIOPIC]*.TTF;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.TTF]COPYRIGHT.*;0 'output'
$ COPY/NOLOG [.EXTRAS.FONTS.BITSTREAM-VERA]COPYRIGHT.TXT 'output'COPYRIGHT.BITSTREAM_VERA
$ COPY/NOLOG [.EXTRAS.FONTS.BITSTREAM-VERA]README.TXT 'output'README.BITSTREAM_VERA
$ COPY/NOLOG [.EXTRAS.FONTS.BITSTREAM-VERA]RELEASENOTES.TXT 'output'RELEASENOTES.BITSTREAM_VERA
$ COPY/NOLOG [.FONTS.SCALED.ETHIOPIC]LICENSE.TXT 'output'LICENSE.ETHIOPIC
$ RETURN
$stage_OTF:
$ COPY/NOLOG [.FONTS.SCALED.ETHIOPIC]*.OTF;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.MELTHO]*.OTF;0 'output'
$ COPY/NOLOG [.FONTS.SCALED.ETHIOPIC]LICENSE.TXT 'output'LICENSE.ETHIOPIC
$ COPY/NOLOG [.FONTS.SCALED.MELTHO]LICENSE.TXT 'output'LICENSE.MELTHO
$ COPY/NOLOG [.FONTS.SCALED.MELTHO]README. 'output'README.MELTHO
$ RETURN
$stage_CID:
$ RETURN
$check_index:
$ IF (current .EQS. "") .OR. (current .EQS. previous) THEN GOTO done
$ OPEN/READ index_file 'current'
$ READ/END=bad_index index_file count
$ CLOSE index_file
$ IF (section .NES. "CID") .AND. (F$INTEGER(count) .LE. 0) THEN GOTO done
$ IF F$INTEGER(count) .LT. 0 THEN GOTO done
$ RETURN
$bad_index:
$ CLOSE index_file
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ SET DEFAULT 'saved_default'
$ EXIT result
