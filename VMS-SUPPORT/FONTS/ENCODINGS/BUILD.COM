$! Preserve upstream encoding bytes and build relocatable native indexes.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ IF (P1 .NES. "") .OR. (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ executable = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.PROGRAMS.MKFONTSCALE]MKFONTSCALE.EXE")
$ IF executable .EQS. "" THEN GOTO done
$ mkfontscale = "$" + executable
$ COPY/NOLOG [.FONT.ENCODINGS]*.ENC;0 [.VMS-SUPPORT._BUILD.P64.FONTS.ENCODINGS]*.ENC;0
$ COPY/NOLOG [.FONT.ENCODINGS.LARGE]*.ENC;0 [.VMS-SUPPORT._BUILD.P64.FONTS.ENCODINGS.LARGE]*.ENC;0
$ SET DEFAULT [.VMS-SUPPORT._BUILD.P64.FONTS.ENCODINGS]
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ mkfontscale -n -r -e "." -e "./large" "."
$ ignored = F$SEARCH("",2)
$ current = F$SEARCH("ENCODINGS.DIR;0",2)
$ GOSUB check_index
$ SET DEFAULT [.LARGE]
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ mkfontscale -n -r -e "." "."
$ ignored = F$SEARCH("",2)
$ current = F$SEARCH("ENCODINGS.DIR;0",2)
$ GOSUB check_index
$ result = 1
$ GOTO done
$check_index:
$ IF current .EQS. "" THEN GOTO done
$ OPEN/READ index_file 'current'
$ READ/END=bad_index index_file count
$ CLOSE index_file
$ IF F$INTEGER(count) .LE. 0 THEN GOTO done
$ WRITE SYS$OUTPUT "Encoding index ", current, ": ", count
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
