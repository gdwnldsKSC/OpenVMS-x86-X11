$! Build modular BDF packages into the ordinary font directories.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ opened = 0
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ parse_style = F$GETJPI("","PARSE_STYLE_PERM")
$ SET PROCESS/PARSE_STYLE=EXTENDED
$ section = F$EDIT(P1,"UPCASE,TRIM")
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF (section .NES. "MISC") .AND. (section .NES. "75DPI") .AND. -
     (section .NES. "100DPI") .AND. (section .NES. "CYRILLIC") THEN GOTO done
$ converter = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.PROGRAMS.BDFTOPCF]BDFTOPCF.EXE")
$ deriver = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.FONTS.UTIL]UCS2ANY.EXE")
$ IF (converter .EQS. "") .OR. (deriver .EQS. "") THEN GOTO done
$ bdftopcf = "$" + converter
$ ucs2any = "$" + deriver
$ output = "[.VMS-SUPPORT._BUILD.P64.FONTS." + section + "]"
$ absolute_output = F$PARSE(output,,,"DEVICE") + F$PARSE(output,,,"DIRECTORY")
$ generated = F$EXTRACT(0,F$LENGTH(absolute_output)-1,absolute_output) + ".GENERATED]"
$ IF F$SEARCH(absolute_output + "GENERATED.DIR") .EQS. "" THEN CREATE/DIRECTORY 'generated'
$ count = 0
$ OPEN/READ listing [.VMS-SUPPORT.FONTS.BDF]PACKAGES.LIST
$ opened = 1
$ next_package:
$ READ/END=packages_done listing line
$ IF F$ELEMENT(0,"|",line) .NES. section THEN GOTO next_package
$ package = F$ELEMENT(1,"|",line)
$ source = "[.FONT." + package + "]"
$ pattern = source + "*.BDF;0"
$ GOSUB convert_directory
$ notice_pattern = source + "COPY*.*;0"
$ GOSUB copy_notices
$ notice_pattern = source + "README.*;0"
$ GOSUB copy_notices
$ notice_pattern = source + "*LEGALNOTICE.*;0"
$ GOSUB copy_notices
$ GOTO next_package
$ packages_done:
$ CLOSE listing
$ opened = 0
$ IF count .EQ. 0 THEN GOTO failed
$ COPY/NOLOG [.FONT.FONT-ALIAS.'section']FONTS.ALIAS 'output'FONTS.ALIAS;0
$ COPY/NOLOG [.FONT.FONT-ALIAS]COPYING. 'output'COPYING.FONT-ALIAS;0
$ OPEN/READ listing [.VMS-SUPPORT.FONTS.BDF]DERIVED.LIST
$ opened = 1
$ next_derived:
$ READ/END=derived_done listing line
$ IF F$ELEMENT(0,"|",line) .NES. section THEN GOTO next_derived
$ package = F$ELEMENT(1,"|",line)
$ leaf = F$ELEMENT(2,"|",line)
$ encoding = F$ELEMENT(3,"|",line)
$ source = F$SEARCH("[.FONT." + package + "]" + leaf + ".BDF")
$ map = ""
$ IF encoding .NES. "JISX0201.1976-0" THEN -
     map = F$SEARCH("[.FONT.FONT-UTIL]MAP-" + encoding + ".")
$ IF encoding .EQS. "JISX0201.1976-0" THEN -
     map = F$SEARCH("[.FONT.FONT-UTIL]MAP-JISX0201.1976-0")
$ IF (source .EQS. "") .OR. (map .EQS. "") THEN GOTO failed
$! ucs2any writes its basename-derived output in the current directory.
$! Unix path syntax is required by its unchanged basename calculation.
$ source = "/VMS_FONT_ROOT/font/" + F$EDIT(package,"LOWERCASE") + "/" + leaf + ".bdf"
$ SET DEFAULT 'generated'
$ DEFINE/USER_MODE VMS_FONT_ROOT 'saved_default'
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ ucs2any "''source'" "''map'" "''encoding'"
$ result = $STATUS
$ SET DEFAULT 'saved_default'
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ name = leaf + "-" + encoding
$ IF encoding .EQS. "JISX0201.1976-0" THEN name = leaf + "-JISX0201^.1976-0"
$ input = generated + name + ".BDF"
$ destination = absolute_output + name + ".PCF"
$ GOSUB convert_font
$ GOTO next_derived
$ derived_done:
$ CLOSE listing
$ opened = 0
$ ignored = F$SEARCH("",2)
$ previous = F$SEARCH(output + "FONTS.DIR;0",2)
$ index_directory = "./VMS-SUPPORT/_BUILD/P64/FONTS/" + section + "/"
$ @[.VMS-SUPPORT._BUILD.P64.PROGRAMS.MKFONTDIR]MKFONTDIR.COM "''index_directory'"
$ ignored = F$SEARCH("",2)
$ current = F$SEARCH(output + "FONTS.DIR;0",2)
$ IF (current .EQS. "") .OR. (current .EQS. previous) THEN GOTO failed
$ OPEN/READ index_file 'current'
$ READ/END=bad_index index_file indexed_count
$ CLOSE index_file
$ IF F$INTEGER(indexed_count) .NE. count THEN GOTO failed
$ WRITE SYS$OUTPUT "BDF ", section, ": ", count, " fonts; index ", current
$ result = 1
$ GOTO done
$ convert_directory:
$ ignored = F$SEARCH("",1)
$ convert_next:
$ input = F$SEARCH(pattern,1)
$ IF input .EQS. "" THEN RETURN
$ name = F$PARSE(input,,,"NAME")
$ destination = output + name + ".PCF"
$ GOSUB convert_font
$ GOTO convert_next
$ convert_font:
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ bdftopcf -o "''destination'" "''input'"
$ ignored = F$SEARCH("",2)
$ IF F$SEARCH(destination,2) .EQS. "" THEN GOTO failed
$ count = count + 1
$ RETURN
$ copy_notices:
$ ignored = F$SEARCH("",3)
$ next_notice:
$ notice = F$SEARCH(notice_pattern,3)
$ IF notice .EQS. "" THEN RETURN
$ notice_name = F$PARSE(notice,,,"NAME")
$ notice_type = F$PARSE(notice,,,"TYPE")
$ notice_name = F$EDIT(notice_name,"UPCASE")
$ IF notice_type .NES. "." THEN -
     notice_name = notice_name + "_" + F$EXTRACT(1,F$LENGTH(notice_type),notice_type)
$ COPY/NOLOG 'notice' 'output''notice_name'.'package';0
$ GOTO next_notice
$ bad_index:
$ CLOSE index_file
$ GOTO failed
$ interrupted:
$ result = 2
$ GOTO done
$ failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$ done:
$ SET NOON
$ IF opened THEN CLOSE listing
$ SET DEFAULT 'saved_default'
$ SET PROCESS/PARSE_STYLE='parse_style'
$ EXIT result
