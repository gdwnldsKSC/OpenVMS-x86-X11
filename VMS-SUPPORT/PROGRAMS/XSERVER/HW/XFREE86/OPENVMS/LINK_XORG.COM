$! Build and run the native ELF symbol/options generator, then link the images.
$! Invoke from the source root: @LINK_XORG manifest.txt output-directory-prefix
$! The image and options directories must already exist. No installation.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO failed
$ result = 2
$ saved_parse = F$GETJPI("","PARSE_STYLE_PERM")
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .NES. "") THEN GOTO done
$ IF F$EDIT(F$GETSYI("ARCH_NAME"),"TRIM") .NES. "x86_64" THEN GOTO done
$ SET PROCESS/PARSE_STYLE=EXTENDED
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ helper = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY") + "XORG_LINK_OPTIONS.C"
$ output = P2 + "XORG_LINK_OPTIONS"
$ object = P2 + "XORG_LINK_OPTIONS.OBJ"
$ image = P2 + "XORG_LINK_OPTIONS.EXE"
$ IF F$TRNLNM("LNK$LIBRARY") .NES. "" THEN GOTO failed
$ n = 1
$library_check:
$ IF F$TRNLNM("LNK$LIBRARY_" + F$STRING(n)) .NES. "" THEN GOTO failed
$ n = n + 1
$ IF n .LE. 999 THEN GOTO library_check
$ flags = "/STANDARD=C99/POINTER_SIZE=64=ARGV/NAMES=(AS_IS,SHORTENED)/PREFIX=ALL_ENTRIES/MAIN=POSIX_EXIT"
$ @[.VMS-SUPPORT]COMPILE.COM 'helper' 'output' "''flags'" RTL
$ IF .NOT. $STATUS THEN GOTO failed
$ LINK/EXECUTABLE='image' 'object'
$ IF .NOT. $STATUS THEN GOTO failed
$ generator = "$" + F$SEARCH(image)
$ generator "''P1'" "''P2'"
$ IF .NOT. $STATUS THEN GOTO failed
$ generated = P2 + "LINK_IMAGES.COM"
$ @'generated'
$ IF .NOT. $STATUS THEN GOTO failed
$ result = 1
$ GOTO done
$failed:
$ result = 2
$done:
$ SET PROCESS/PARSE_STYLE='saved_parse'
$ EXIT 'result'
