$! Copy the upstream XKB install data and native compiler to an explicit directory.
$! Existing versions are preserved. No process, logical-name, or protection changes.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ opened = 0
$ count = 0
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") THEN GOTO usage
$ IF (P3 .NES. "") .OR. (P4 .NES. "") .OR. (P5 .NES. "") .OR. -
     (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
$ allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:-.[]"
$ index = 0
$check_destination:
$ IF index .GE. F$LENGTH(P1) THEN GOTO destination_checked
$ letter = F$EXTRACT(index,1,P1)
$ IF F$LOCATE(letter,allowed) .EQ. F$LENGTH(allowed) THEN GOTO usage
$ index = index + 1
$ GOTO check_destination
$destination_checked:
$ IF F$EXTRACT(F$LENGTH(P1)-1,1,P1) .NES. "]" THEN GOTO usage
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ support_directory = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY")
$ SET DEFAULT 'support_directory'
$ SET DEFAULT [-]
$ SET DEFAULT [-]
$ SET DEFAULT [-]
$ IF F$SEARCH("[.DATA.XKBDATA]MAKEFILE.AM") .EQS. "" THEN GOTO usage
$ destination = F$PARSE(P1,,,"DEVICE","SYNTAX_ONLY") + F$PARSE(P1,,,"DIRECTORY","SYNTAX_ONLY")
$ IF F$PARSE(destination,,,"DIRECTORY","SYNTAX_ONLY") .EQS. "[000000]" THEN GOTO usage
$ IF destination .EQS. F$ENVIRONMENT("DEFAULT") THEN GOTO usage
$ compiler = F$SEARCH(P2)
$ IF compiler .EQS. "" THEN GOTO missing
$ CREATE/DIRECTORY 'destination'
$ previous_directory = ""
$ OPEN/READ listing [.VMS-SUPPORT.PROGRAMS.XKBCOMP]DATA.LIST
$ opened = 1
$next:
$ READ/END=data_done listing line
$ IF line .EQS. "" THEN GOTO next
$ IF F$EXTRACT(0,1,line) .EQS. "!" THEN GOTO next
$ relative = F$ELEMENT(0,"|",line)
$ GOSUB native_relative
$ IF result .NE. 1 THEN GOTO done
$ source = "[.DATA.XKBDATA" + relative_directory + "]" + relative_name
$ source = F$SEARCH(source)
$ IF source .EQS. "" THEN GOTO missing
$ relative = F$ELEMENT(1,"|",line)
$ GOSUB native_relative
$ IF result .NE. 1 THEN GOTO done
$ directory = F$EXTRACT(0,F$LENGTH(destination)-1,destination) + relative_directory + "]"
$ IF directory .NES. previous_directory THEN CREATE/DIRECTORY 'directory'
$ previous_directory = directory
$ target = directory + relative_name + ";0"
$ COPY/NOLOG 'source' 'target'
$ count = count + 1
$ GOTO next
$data_done:
$ CLOSE listing
$ opened = 0
$ target = destination + "XKBCOMP.EXE;0"
$ COPY/NOLOG 'compiler' 'target'
$ WRITE SYS$OUTPUT "XKB_DATA_FILES=",count,"|COMPILER=",target
$ result = 1
$ GOTO done
$native_relative:
$ result = 2
$ relative_directory = ""
$ relative_name = ""
$ relative_index = 0
$relative_part:
$ part = F$ELEMENT(relative_index,"/",relative)
$ IF (part .EQS. "") .OR. (part .EQS. "/") .OR. (part .EQS. ".") .OR. (part .EQS. "..") THEN RETURN
$ valid = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-"
$ letter_index = 0
$relative_character:
$ IF letter_index .GE. F$LENGTH(part) THEN GOTO relative_valid
$ letter = F$EXTRACT(letter_index,1,part)
$ IF F$LOCATE(letter,valid) .EQ. F$LENGTH(valid) THEN RETURN
$ letter_index = letter_index + 1
$ GOTO relative_character
$relative_valid:
$ IF F$ELEMENT(relative_index+1,"/",relative) .EQS. "/" THEN GOTO relative_leaf
$ IF F$LOCATE(".",part) .NE. F$LENGTH(part) THEN RETURN
$ relative_directory = relative_directory + "." + part
$ relative_index = relative_index + 1
$ GOTO relative_part
$relative_leaf:
$ relative_name = part
$ IF F$LOCATE(".",part) .EQ. F$LENGTH(part) THEN relative_name = part + "."
$ result = 1
$ RETURN
$missing:
$ WRITE SYS$OUTPUT "XKB_DATA: a declared source or compiler is missing."
$ result = 2
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Usage: @STAGE_DATA native-destination-directory native-xkbcomp-image"
$ result = 2
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
