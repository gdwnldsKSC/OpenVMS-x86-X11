$! Build the read-only authority checker from the source root, after BUILD XAU.
$! No server/core relink, installation, privilege changes, or image execution.
$ SET NOON
$ result = 2
$ options_open = 0
$ x11_scoped = 0
$ saved_parse = F$GETJPI("","PARSE_STYLE_PERM")
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ IF P1 .NES. "" .OR. P2 .NES. "" .OR. P3 .NES. "" .OR. P4 .NES. "" THEN GOTO usage
$ IF P5 .NES. "" .OR. P6 .NES. "" .OR. P7 .NES. "" .OR. P8 .NES. "" THEN GOTO usage
$ IF F$EDIT(F$GETSYI("ARCH_NAME"),"TRIM") .NES. "x86_64" THEN GOTO failed
$ SET PROCESS/PARSE_STYLE=EXTENDED
$ source = F$SEARCH("[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]XORG_AUTH.C")
$ xau = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.LIB.XAU]XAU.OLB")
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XAUTH.H")
$ rtl = F$SEARCH("SYS$LIBRARY:DECC$RTLDEF.TLB")
$ IF source .EQS. "" .OR. xau .EQS. "" .OR. header .EQS. "" .OR. rtl .EQS. "" THEN GOTO missing
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XFUNCPROTO.H") .EQS. "" THEN GOTO missing
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XFUNCS.H") .EQS. "" THEN GOTO missing
$ IF F$TRNLNM("LNK$LIBRARY") .NES. "" THEN GOTO failed
$ index = 1
$library_check:
$ IF F$TRNLNM("LNK$LIBRARY_" + F$STRING(index)) .NES. "" THEN GOTO failed
$ index = index + 1
$ IF index .LE. 999 THEN GOTO library_check
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO failed
$ inherited_x11 = F$TRNLNM("X11")
$ inherited_table = F$TRNLNM("X11",,,,,"TABLE_NAME")
$ headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ CREATE/DIRECTORY [.VMS-SUPPORT._BUILD.P64_XORG.LINK]
$ CREATE/DIRECTORY [.VMS-SUPPORT._BUILD.P64_XORG.IMAGES]
$ output = "[.VMS-SUPPORT._BUILD.P64_XORG.LINK]XORG_AUTH"
$ image = "[.VMS-SUPPORT._BUILD.P64_XORG.IMAGES]XORG_AUTH.EXE"
$ DEFINE/USER_MODE/PROCESS X11 'headers'
$ x11_scoped = 1
$ IF F$TRNLNM("X11","LNM$FILE_DEV") .NES. headers THEN GOTO failed
$ CC/STANDARD=C99/POINTER_SIZE=64=ARGV/NAMES=(AS_IS,SHORTENED) -
    /PREFIX=ALL_ENTRIES/MAIN=POSIX_EXIT/NODEBUG/WARNINGS=(ENABLE=LEVEL4) -
    /NOFIRST_INCLUDE/NESTED_INCLUDE_DIRECTORY=NONE -
    /INCLUDE_DIRECTORY=([.VMS-SUPPORT._BUILD.P64.INCLUDE],"") -
    /LIST='output'.LIS/SHOW=INCLUDE -
    /MMS_DEPENDENCIES=(FILE='output'.MMS,SYSTEM_INCLUDE_FILES) -
    /OBJECT='output'.OBJ 'source' + 'rtl'/LIBRARY
$ IF .NOT. $STATUS THEN GOTO failed
$ IF F$TRNLNM("X11") .NES. inherited_x11 THEN GOTO failed
$ IF F$TRNLNM("X11",,,,,"TABLE_NAME") .NES. inherited_table THEN GOTO failed
$ OPEN/WRITE options_file 'output'.OPT
$ options_open = 1
$ WRITE options_file "CASE_SENSITIVE=YES"
$ WRITE options_file output + ".OBJ"
$ WRITE options_file xau + "/LIBRARY"
$ CLOSE options_file
$ options_open = 0
$ LINK/EXECUTABLE='image'/MAP='output'.MAP/FULL 'output'.OPT/OPTIONS
$ IF .NOT. $STATUS THEN GOTO failed
$ WRITE SYS$OUTPUT "Built XORG_AUTH.EXE; no server or authority file was started or changed."
$ result = 1
$ GOTO done
$usage:
$ WRITE SYS$ERROR "Run @BUILD_XORG_AUTH with no arguments from the public source root."
$ GOTO done
$missing:
$ WRITE SYS$ERROR "BUILD_XORG_AUTH requires source-tree BUILD XAU outputs and installed C RTL headers."
$ GOTO done
$interrupted:
$ result = 44
$ GOTO done
$failed:
$ result = 2
$done:
$ SET NOON
$ IF options_open THEN CLOSE options_file
$ IF .NOT. x11_scoped THEN GOTO restore_parse
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN -
    DEASSIGN/USER_MODE/PROCESS X11
$ IF F$TRNLNM("X11") .NES. inherited_x11 THEN result = 2
$ IF F$TRNLNM("X11",,,,,"TABLE_NAME") .NES. inherited_table THEN result = 2
$restore_parse:
$ SET PROCESS/PARSE_STYLE='saved_parse'
$ IF .NOT. $STATUS THEN result = $STATUS
$ EXIT result
