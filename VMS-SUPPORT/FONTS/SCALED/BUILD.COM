$! Stage the unchanged upstream scalable fonts and all accompanying notices.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ opened = 0
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
$ GOSUB stage_packages
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
$stage_packages:
$ IF section .EQS. "CID" THEN RETURN
$ extensions = "PFA|PFB|AFM"
$ IF section .EQS. "SPEEDO" THEN extensions = "SPD"
$ IF section .EQS. "TTF" THEN extensions = "TTF"
$ IF section .EQS. "OTF" THEN extensions = "OTF"
$ OPEN/READ listing [.VMS-SUPPORT.FONTS.SCALED]PACKAGES.LIST
$ opened = 1
$next_package:
$ READ/END=packages_done listing line
$ IF F$ELEMENT(0,"|",line) .NES. section THEN GOTO next_package
$ package = F$ELEMENT(1,"|",line)
$ source = "[.FONT." + package + "]"
$ extension_index = 0
$next_extension:
$ extension = F$ELEMENT(extension_index,"|",extensions)
$ IF extension .EQS. "|" THEN GOTO notices
$ ignored = F$SEARCH("",1)
$next_font:
$ input = F$SEARCH(source + "*." + extension + ";0",1)
$ IF input .EQS. "" THEN GOTO extension_done
$ target = output + F$PARSE(input,,,"NAME") + F$PARSE(input,,,"TYPE") + ";0"
$ COPY/NOLOG 'input' 'target'
$ GOTO next_font
$extension_done:
$ extension_index = extension_index + 1
$ GOTO next_extension
$notices:
$ notice_patterns = "COPY*.*;0|LICENSE.*;0|README.*;0"
$ notice_index = 0
$next_notice_pattern:
$ notice_pattern = F$ELEMENT(notice_index,"|",notice_patterns)
$ IF notice_pattern .EQS. "|" THEN GOTO notices_done
$ ignored = F$SEARCH("",2)
$next_notice:
$ input = F$SEARCH(source + notice_pattern,2)
$ IF input .EQS. "" THEN GOTO notice_pattern_done
$ name = F$PARSE(input,,,"NAME")
$ type = F$PARSE(input,,,"TYPE")
$ IF type .NES. "." THEN name = name + "_" + F$EXTRACT(1,F$LENGTH(type),type)
$ COPY/NOLOG 'input' 'output''name'.'package';0
$ GOTO next_notice
$notice_pattern_done:
$ notice_index = notice_index + 1
$ GOTO next_notice_pattern
$notices_done:
$ IF section .EQS. "SPEEDO" THEN COPY/NOLOG 'source'FONTS.SCALE 'output'FONTS.SCALE;0
$ GOTO next_package
$packages_done:
$ CLOSE listing
$ opened = 0
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
$ IF opened THEN CLOSE listing
$ SET DEFAULT 'saved_default'
$ EXIT result
