$! Original component build entry point. No system installation or driver actions.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ step = "arguments"
$ target = F$EDIT(P1,"UPCASE,TRIM")
$ IF target .EQS. "" THEN target = "ALL"
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. (P5 .NES. "") .OR. -
     (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
$ targets = "|ALL|HEADERS|XAU|XDMCP|ZLIB|FONTENC|EXPAT|LBXUTIL|XFONT|FNTSTUBS|FS|PSRES|BDFTOPCF|FSLSFONTS|SHOWFONT|XFSINFO|"
$ IF F$LOCATE("|",target) .NE. F$LENGTH(target) THEN GOTO usage
$ IF F$LOCATE("|" + target + "|",targets) .EQ. F$LENGTH(targets) THEN GOTO usage
$ IF F$EDIT(F$GETSYI("ARCH_NAME"),"TRIM") .NES. "x86_64" THEN GOTO unsupported
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ step = "locate source tree"
$ support_directory = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY")
$ SET DEFAULT 'support_directory'
$ SET DEFAULT [-]
$ IF F$SEARCH("[.INCLUDE]XFUNCPROTO.H") .EQS. "" THEN GOTO missing_source
$ step = "create output directories"
$ IF F$SEARCH("[.VMS-SUPPORT]_BUILD.DIR") .EQS. "" THEN CREATE/DIRECTORY [.VMS-SUPPORT._BUILD]
$ profile = "[.VMS-SUPPORT._BUILD.P64"
$ directory = profile + "]"
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD]P64.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE]"
$ IF F$SEARCH(profile + "]INCLUDE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.DPS]"
$ IF F$SEARCH(profile + ".INCLUDE]DPS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11]"
$ IF F$SEARCH(profile + ".INCLUDE]X11.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.FONTS]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]FONTS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.EXTENSIONS]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]EXTENSIONS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB]"
$ IF F$SEARCH(profile + "]LIB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XAU]"
$ IF F$SEARCH(profile + ".LIB]XAU.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XDMCP]"
$ IF F$SEARCH(profile + ".LIB]XDMCP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.ZLIB]"
$ IF F$SEARCH(profile + ".LIB]ZLIB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONTENC]"
$ IF F$SEARCH(profile + ".LIB]FONTENC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.EXPAT]"
$ IF F$SEARCH(profile + ".LIB]EXPAT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.LBXUTIL]"
$ IF F$SEARCH(profile + ".LIB]LBXUTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.LBXUTIL.IMAGE]"
$ IF F$SEARCH(profile + ".LIB.LBXUTIL]IMAGE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FNTSTUBS]"
$ IF F$SEARCH(profile + ".LIB]FNTSTUBS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FS]"
$ IF F$SEARCH(profile + ".LIB]FS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.PSRES]"
$ IF F$SEARCH(profile + ".LIB]PSRES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT]"
$ IF F$SEARCH(profile + ".LIB]FONT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.UTIL]"
$ IF F$SEARCH(profile + ".LIB.FONT]UTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.FC]"
$ IF F$SEARCH(profile + ".LIB.FONT]FC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.BITMAP]"
$ IF F$SEARCH(profile + ".LIB.FONT]BITMAP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.FONTFILE]"
$ IF F$SEARCH(profile + ".LIB.FONT]FONTFILE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS]"
$ IF F$SEARCH(profile + "]PROGRAMS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.BDFTOPCF]"
$ IF F$SEARCH(profile + ".PROGRAMS]BDFTOPCF.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.FSLSFONTS]"
$ IF F$SEARCH(profile + ".PROGRAMS]FSLSFONTS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.SHOWFONT]"
$ IF F$SEARCH(profile + ".PROGRAMS]SHOWFONT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XFSINFO]"
$ IF F$SEARCH(profile + ".PROGRAMS]XFSINFO.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ WRITE SYS$OUTPUT "X11 component build: " + target + ", native x86-64, pointer64"
$ step = "MMS " + target
$ MMS/DESCRIPTION=[.VMS-SUPPORT]DESCRIP.MMS 'target'
$ result = $STATUS
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Usage: @BUILD [ALL|HEADERS|XAU|XDMCP|ZLIB|FONTENC|EXPAT|LBXUTIL|XFONT|FNTSTUBS|FS|PSRES|BDFTOPCF|FSLSFONTS|SHOWFONT|XFSINFO]"
$ GOTO done
$unsupported:
$ WRITE SYS$OUTPUT "This initial build profile supports OpenVMS x86-64 only."
$ GOTO done
$missing_source:
$ WRITE SYS$OUTPUT "Run this procedure from its original VMS-SUPPORT location in the source tree."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$ WRITE SYS$OUTPUT "X11 build failed at " + step + ": " + F$MESSAGE(result)
$done:
$ SET NOON
$ SET DEFAULT 'saved_default'
$ EXIT result
