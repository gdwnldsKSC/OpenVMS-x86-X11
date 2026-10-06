$! Compile the complete supplied BDF section and its upstream aliases.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ section = F$EDIT(P1,"UPCASE,TRIM")
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF (section .NES. "MISC") .AND. (section .NES. "75DPI") .AND. -
     (section .NES. "100DPI") .AND. (section .NES. "CYRILLIC") THEN GOTO done
$ converter = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.PROGRAMS.BDFTOPCF]BDFTOPCF.EXE")
$ IF converter .EQS. "" THEN GOTO done
$ bdftopcf = "$" + converter
$ converter_time = F$CVTIME(F$FILE_ATTRIBUTES(converter,"RDT"),"COMPARISON")
$ output = "[.VMS-SUPPORT._BUILD.P64.FONTS." + section + "]"
$ source = "[.FONTS.BDF." + section + "]"
$ count = 0
$ pattern = source + "*.BDF;0"
$ GOSUB convert_directory
$ IF count .EQ. 0 THEN GOTO done
$ COPY/NOLOG 'source'FONTS.ALIAS 'output'FONTS.ALIAS
$ IF (section .EQS. "75DPI") .OR. (section .EQS. "100DPI") THEN -
     COPY/NOLOG 'source'LU_LEGALNOTICE. 'output'LU_LEGALNOTICE.
$ IF section .EQS. "CYRILLIC" THEN COPY/NOLOG 'source'COPYRIGHT. 'output'COPYRIGHT.
$ IF section .EQS. "MISC"
$ THEN
$   pattern = "[.EXTRAS.FONTS.CLEARLYU]*.BDF;0"
$   GOSUB convert_directory
$   pattern = "[.EXTRAS.FONTS.ARABIC24]*.BDF;0"
$   GOSUB convert_directory
$   COPY/NOLOG [.FONTS.BDF.MISC]README. 'output'README.MISC
$   COPY/NOLOG [.EXTRAS.FONTS.CLEARLYU]README. 'output'README.CLEARLYU
$   COPY/NOLOG [.EXTRAS.FONTS.ARABIC24]README. 'output'README.ARABIC24
$ ENDIF
$ ignored = F$SEARCH("",2)
$ previous = F$SEARCH(output + "FONTS.DIR;0",2)
$ index_directory = "./VMS-SUPPORT/_BUILD/P64/FONTS/" + section + "/"
$ @[.VMS-SUPPORT._BUILD.P64.PROGRAMS.MKFONTDIR]MKFONTDIR.COM "''index_directory'"
$ ignored = F$SEARCH("",2)
$ current = F$SEARCH(output + "FONTS.DIR;0",2)
$ IF (current .EQS. "") .OR. (current .EQS. previous) THEN GOTO done
$ OPEN/READ index_file 'current'
$ READ/END=bad_index index_file indexed_count
$ CLOSE index_file
$ IF F$INTEGER(indexed_count) .LT. count THEN GOTO done
$ WRITE SYS$OUTPUT "BDF ", section, ": ", count, " fonts; index ", current
$ result = 1
$ GOTO done
$convert_directory:
$ ignored = F$SEARCH("",1)
$convert_next:
$ input = F$SEARCH(pattern,1)
$ IF input .EQS. "" THEN RETURN
$ count = count + 1
$ name = F$EDIT(F$PARSE(input,,,"NAME"),"UPCASE")
$ length = F$LENGTH(name)
$ IF (length .GE. 3) .AND. (F$EXTRACT(length-3,3,name) .EQS. "-L1") THEN -
     name = F$EXTRACT(0,length-3,name) + "-ISO8859-1"
$ IF name .EQS. "7X14-JISX0201" THEN name = "7X14-JISX0201^.1976-0"
$ destination = output + name + ".PCF"
$ ignored = F$SEARCH("",2)
$ current = F$SEARCH(destination,2)
$ IF current .NES. ""
$ THEN
$   output_time = F$CVTIME(F$FILE_ATTRIBUTES(current,"RDT"),"COMPARISON")
$   input_time = F$CVTIME(F$FILE_ATTRIBUTES(input,"RDT"),"COMPARISON")
$   IF (output_time .GES. input_time) .AND. (output_time .GES. converter_time) THEN GOTO convert_next
$ ENDIF
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ bdftopcf -o "''destination'" "''input'"
$ ignored = F$SEARCH("",2)
$ IF F$SEARCH(destination,2) .EQS. "" THEN GOTO failed
$ GOTO convert_next
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
$ EXIT result
