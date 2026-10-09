$! Explicit foreground launcher. No authority provisioning or system startup changes.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO usage
$ IF (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
$ display_number = P4
$ IF display_number .EQS. "" THEN display_number = "0"
$ character = 0
$check_display:
$ IF character .GE. F$LENGTH(display_number) THEN GOTO display_checked
$ letter = F$EXTRACT(character,1,display_number)
$ IF F$LOCATE(letter,"0123456789") .EQ. 10 THEN GOTO usage
$ character = character + 1
$ GOTO check_display
$display_checked:
$ IF F$LENGTH(display_number) .GT. 5 THEN GOTO usage
$ IF F$INTEGER(display_number) .GT. 59535 THEN GOTO usage
$ no_listen = F$EDIT(P5,"LOWERCASE,TRIM")
$ IF (no_listen .NES. "") .AND. (no_listen .NES. "tcp") .AND. (no_listen .NES. "local") THEN GOTO usage
$ allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:-.[]"
$ character = 0
$check_root:
$ IF character .GE. F$LENGTH(P2) THEN GOTO root_checked
$ letter = F$EXTRACT(character,1,P2)
$ IF F$LOCATE(letter,allowed) .EQ. F$LENGTH(allowed) THEN GOTO usage
$ character = character + 1
$ GOTO check_root
$root_checked:
$ IF F$EXTRACT(F$LENGTH(P2)-1,1,P2) .NES. "]" THEN GOTO usage
$ root = F$PARSE(P2,,,"DEVICE","SYNTAX_ONLY") + F$PARSE(P2,,,"DIRECTORY","SYNTAX_ONLY")
$ prefix = F$EXTRACT(0,F$LENGTH(root)-1,root)
$ image = F$SEARCH(P1)
$ IF image .EQS. "" THEN GOTO missing
$ IF F$EDIT(F$PARSE(image,,,"TYPE"),"UPCASE") .NES. ".EXE" THEN GOTO missing
$ ignored = F$SEARCH("",1)
$ authority = F$SEARCH(P3,1)
$ IF authority .EQS. "" THEN GOTO missing
$ IF F$FILE_ATTRIBUTES(authority,"EOF") .LE. 0 THEN GOTO missing
$ file_specs = image + "|" + authority
$ spec_index = 0
$check_filespec:
$ spec = F$ELEMENT(spec_index,"|",file_specs)
$ IF spec .EQS. "|" THEN GOTO filespecs_checked
$ allowed_file = allowed + ";"
$ character = 0
$check_file_character:
$ IF character .GE. F$LENGTH(spec) THEN GOTO filespec_checked
$ letter = F$EXTRACT(character,1,spec)
$ IF F$LOCATE(letter,allowed_file) .EQ. F$LENGTH(allowed_file) THEN GOTO usage
$ character = character + 1
$ GOTO check_file_character
$filespec_checked:
$ spec_index = spec_index + 1
$ GOTO check_filespec
$filespecs_checked:
$ IF F$SEARCH(prefix + ".LIB.X11.XKB]XKBCOMP.EXE;0") .EQS. "" THEN GOTO missing
$ IF F$SEARCH(prefix + ".LIB.X11.XKB.RULES]XORG.;0") .EQS. "" THEN GOTO missing
$ IF F$SEARCH(prefix + ".LIB.X11]RGB.TXT;0") .EQS. "" THEN GOTO missing
$ device = F$PARSE(root,,,"DEVICE","SYNTAX_ONLY")
$ unix_root = "/" + F$EXTRACT(0,F$LENGTH(device)-1,device)
$ directory = F$PARSE(root,,,"DIRECTORY","SYNTAX_ONLY")
$ directory = F$EXTRACT(1,F$LENGTH(directory)-2,directory)
$ part_index = 0
$unix_directory:
$ part = F$ELEMENT(part_index,".",directory)
$ IF part .EQS. "." THEN GOTO unix_done
$ unix_root = unix_root + "/" + part
$ part_index = part_index + 1
$ GOTO unix_directory
$unix_done:
$ sections = "MISC|75DPI|100DPI|CYRILLIC"
$ section_index = 0
$ font_path = ""
$font_next:
$ section = F$ELEMENT(section_index,"|",sections)
$ IF section .EQS. "|" THEN GOTO fonts_done
$ IF F$SEARCH(prefix + ".FONTS." + section + "]FONTS.DIR;0") .EQS. "" THEN GOTO missing
$ IF font_path .NES. "" THEN font_path = font_path + ","
$ font_path = font_path + unix_root + "/FONTS/" + section
$ section_index = section_index + 1
$ GOTO font_next
$fonts_done:
$ xkb_path = unix_root + "/LIB/X11/XKB"
$ rgb_path = unix_root + "/LIB/X11/rgb"
$ screen = ":" + display_number
$ server = "$" + image
$! The default compiled XKB_BIN_DIRECTORY is relative to this runtime root.
$ SET DEFAULT 'root'
$ IF no_listen .NES. "" THEN GOTO restricted
$ server "''screen'" "-auth" "''authority'" "-fp" "''font_path'" "-xkbdir" "''xkb_path'" "-co" "''rgb_path'"
$ result = $STATUS
$ GOTO done
$restricted:
$ server "''screen'" "-auth" "''authority'" "-fp" "''font_path'" "-xkbdir" "''xkb_path'" "-co" "''rgb_path'" "-nolisten" "''no_listen'"
$ result = $STATUS
$ GOTO done
$missing:
$ WRITE SYS$OUTPUT "RUN_SERVER: image, nonempty authority, or runtime data missing."
$ result = 2
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Usage: @RUN_SERVER server.exe runtime-directory authority-file [display-number] [nolisten-transport]"
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
$ SET DEFAULT 'saved_default'
$ EXIT result
