$! Stage native public build outputs. Existing file versions are preserved.
$! Does not install images, change startup, write authority files, or start a server.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ parse_style = F$GETJPI("","PARSE_STYLE_PERM")
$ SET PROCESS/PARSE_STYLE=EXTENDED
$ result = 2
$ opened = 0
$ index_open = 0
$ copied = 0
$ IF P1 .EQS. "" THEN GOTO usage
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
$ allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:-.[]"
$ character = 0
$check_destination:
$ IF character .GE. F$LENGTH(P1) THEN GOTO destination_checked
$ letter = F$EXTRACT(character,1,P1)
$ IF F$LOCATE(letter,allowed) .EQ. F$LENGTH(allowed) THEN GOTO usage
$ character = character + 1
$ GOTO check_destination
$destination_checked:
$ IF F$EXTRACT(F$LENGTH(P1)-1,1,P1) .NES. "]" THEN GOTO usage
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ support = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY")
$ SET DEFAULT 'support'
$ SET DEFAULT [-]
$ SET DEFAULT [-]
$ IF F$SEARCH("[.INCLUDE]XFUNCPROTO.H") .EQS. "" THEN GOTO usage
$ root = F$PARSE(P1,,,"DEVICE","SYNTAX_ONLY") + F$PARSE(P1,,,"DIRECTORY","SYNTAX_ONLY")
$ root_directory = F$PARSE(root,,,"DIRECTORY","SYNTAX_ONLY")
$ IF root_directory .EQS. "[000000]" THEN GOTO usage
$ IF root .EQS. F$ENVIRONMENT("DEFAULT") THEN GOTO usage
$ prefix = F$EXTRACT(0,F$LENGTH(root)-1,root)
$ device = F$PARSE(root,,,"DEVICE","SYNTAX_ONLY")
$ unix_root = "/" + F$EXTRACT(0,F$LENGTH(device)-1,device)
$ directory = F$EXTRACT(1,F$LENGTH(root_directory)-2,root_directory)
$ part_index = 0
$unix_directory:
$ part = F$ELEMENT(part_index,".",directory)
$ IF part .EQS. "." THEN GOTO unix_done
$ unix_root = unix_root + "/" + part
$ part_index = part_index + 1
$ GOTO unix_directory
$unix_done:
$ CREATE/DIRECTORY 'root'
$ bin = prefix + ".BIN]"
$ xlib = prefix + ".LIB.X11]"
$ nls = prefix + ".NLS]"
$ xkb = prefix + ".LIB.X11.XKB]"
$ CREATE/DIRECTORY 'bin'
$ CREATE/DIRECTORY 'xlib'
$ CREATE/DIRECTORY 'nls'
$ programs = "XAUTH|XDPYINFO|XWININFO|XSET|XMODMAP|XEV|SETXKBMAP|MKFONTSCALE"
$ program_index = 0
$program_next:
$ program = F$ELEMENT(program_index,"|",programs)
$ IF program .EQS. "|" THEN GOTO programs_done
$ input = "[.VMS-SUPPORT._BUILD.P64.PROGRAMS." + program + "]" + program + ".EXE"
$ output = bin + program + ".EXE"
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ program_index = program_index + 1
$ GOTO program_next
$programs_done:
$ indexer = "$" + F$SEARCH(bin + "MKFONTSCALE.EXE;0")
$ sections = "MISC|75DPI|100DPI|CYRILLIC"
$ section_index = 0
$font_section:
$ section = F$ELEMENT(section_index,"|",sections)
$ IF section .EQS. "|" THEN GOTO fonts_done
$ source_directory = "[.VMS-SUPPORT._BUILD.P64.FONTS." + section + "]"
$ target_directory = prefix + ".FONTS." + section + "]"
$ CREATE/DIRECTORY 'target_directory'
$ ignored = F$SEARCH("",1)
$ font_count = 0
$font_next:
$ input = F$SEARCH(source_directory + "*.PCF;0",1)
$ IF input .EQS. "" THEN GOTO font_files_done
$ output = target_directory + F$PARSE(input,,,"NAME") + F$PARSE(input,,,"TYPE")
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ font_count = font_count + 1
$ GOTO font_next
$font_files_done:
$ IF font_count .EQ. 0 THEN GOTO missing
$ notices = "FONTS.ALIAS"
$ IF (section .EQS. "75DPI") .OR. (section .EQS. "100DPI") THEN notices = notices + "|LU_LEGALNOTICE."
$ IF section .EQS. "CYRILLIC" THEN notices = notices + "|COPYRIGHT."
$ IF section .EQS. "MISC" THEN notices = notices + "|README.MISC|README.CLEARLYU|README.ARABIC24"
$ notice_index = 0
$notice_next:
$ notice = F$ELEMENT(notice_index,"|",notices)
$ IF notice .EQS. "|" THEN GOTO notices_done
$ input = source_directory + notice
$ output = target_directory + notice
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ notice_index = notice_index + 1
$ GOTO notice_next
$notices_done:
$ font_path = unix_root + "/FONTS/" + section + "/"
$ ignored = F$SEARCH("",2)
$ previous_index = F$SEARCH(target_directory + "FONTS.DIR;0",2)
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ indexer "-b" "-s" "-l" "''font_path'"
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ ignored = F$SEARCH("",2)
$ current_index = F$SEARCH(target_directory + "FONTS.DIR;0",2)
$ IF (current_index .EQS. "") .OR. (current_index .EQS. previous_index) THEN GOTO missing
$ OPEN/READ index_file 'current_index'
$ index_open = 1
$ READ/END=missing index_file indexed_count
$ CLOSE index_file
$ index_open = 0
$ IF F$INTEGER(indexed_count) .NE. font_count THEN GOTO missing
$ WRITE SYS$OUTPUT "RUNTIME_FONTS=",section,"|FILES=",font_count
$ section_index = section_index + 1
$ GOTO font_section
$fonts_done:
$ databases = "XERRORDB.|XKEYSYMDB.|XCMS.TXT"
$ database_index = 0
$database_next:
$ database = F$ELEMENT(database_index,"|",databases)
$ IF database .EQS. "|" THEN GOTO databases_done
$ input = "[.VMS-SUPPORT._BUILD.P64.LIB.X11]" + database
$ output = xlib + database
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ database_index = database_index + 1
$ GOTO database_next
$databases_done:
$ input = "[.PROGRAMS.RGB]RGB.TXT"
$ output = xlib + "RGB.TXT"
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ locale_indices = "LOCALE.ALIAS|LOCALE.DIR|COMPOSE.DIR"
$ locale_index = 0
$locale_index_next:
$ locale_file = F$ELEMENT(locale_index,"|",locale_indices)
$ IF locale_file .EQS. "|" THEN GOTO locale_indices_done
$ input = "[.VMS-SUPPORT._BUILD.P64.NLS]" + locale_file
$ output = nls + locale_file
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ locale_index = locale_index + 1
$ GOTO locale_index_next
$locale_indices_done:
$ locale_sets = "XLC_LOCALE|COMPOSE"
$ locale_set_index = 0
$locale_set_next:
$ locale_set = F$ELEMENT(locale_set_index,"|",locale_sets)
$ IF locale_set .EQS. "|" THEN GOTO locales_done
$ OPEN/READ listing [.VMS-SUPPORT.NLS]'locale_set'.LIST
$ opened = 1
$locale_next:
$ READ/END=locale_set_done listing locale
$ escaped = ""
$ character = 0
$locale_escape:
$ IF character .GE. F$LENGTH(locale) THEN GOTO locale_escaped
$ letter = F$EXTRACT(character,1,locale)
$ IF letter .EQS. "." THEN escaped = escaped + "^"
$ escaped = escaped + letter
$ character = character + 1
$ GOTO locale_escape
$locale_escaped:
$ target_directory = prefix + ".NLS." + escaped + "]"
$ CREATE/DIRECTORY 'target_directory'
$ input = "[.VMS-SUPPORT._BUILD.P64.NLS." + escaped + "]" + locale_set + "."
$ output = target_directory + locale_set + "."
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ GOTO locale_next
$locale_set_done:
$ CLOSE listing
$ opened = 0
$ locale_set_index = locale_set_index + 1
$ GOTO locale_set_next
$locales_done:
$ @[.VMS-SUPPORT.PROGRAMS.XKBCOMP]STAGE_DATA.COM "''xkb'" "[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XKBCOMP]XKBCOMP.EXE"
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ input = "[.VMS-SUPPORT.RUNTIME]RUN_SERVER.COM"
$ output = root + "RUN_SERVER.COM"
$ GOSUB copy_file
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ WRITE SYS$OUTPUT "RUNTIME_STAGED=",root,"|COPIED=",copied,"|XKB_DATA=STAGED|FONT_INDICES=4"
$ WRITE SYS$OUTPUT "RUNTIME_UNIX_ROOT=",unix_root
$ result = 1
$ GOTO done
$copy_file:
$ result = 2
$ ignored = F$SEARCH("",2)
$ found = F$SEARCH(input,2)
$ IF found .EQS. "" THEN RETURN
$ COPY/NOLOG 'found' 'output';0
$ result = $STATUS
$ IF result .AND. 1 THEN copied = copied + 1
$ RETURN
$missing:
$ WRITE SYS$OUTPUT "RUNTIME: required input or complete font index missing."
$ result = 2
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Usage: @STAGE explicit-native-runtime-directory"
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
$ IF index_open THEN CLOSE index_file
$ IF opened THEN CLOSE listing
$ SET DEFAULT 'saved_default'
$ SET PROCESS/PARSE_STYLE='parse_style'
$ EXIT result
