$! Original component build entry point. No system installation or driver actions.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ step = "arguments"
$ target = F$EDIT(P1,"UPCASE,TRIM")
$ IF target .EQS. "" THEN target = "ALL"
$ targets = "|ALL|HEADERS|XAU|XDMCP|ZLIB|FONTENC|EXPAT|LBXUTIL|XFONT|FNTSTUBS|FS|PSRES|REGEX|ICE|SM|BDFTOPCF|FSLSFONTS|SHOWFONT|XFSINFO|ICEAUTH|MAKEPSRES|SHOWRGB|MAKESTRS|MAKEDEPEND|LNDIR|ATOBM|GTF|FREETYPE2|MKFONTSCALE|FONTTOSFNT|REVPATH|RMAN|"
$ targets = targets + "UCS2ANY|DAMAGE|MI|CBRT|FB|RENDER|DIX|XPSTUBS|SHADOW|RANDR|DBE|"
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. (P5 .NES. "") .OR. -
     (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
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
$ directory = profile + ".INCLUDE.FREETYPE2]"
$ IF F$SEARCH(profile + ".INCLUDE]FREETYPE2.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.FREETYPE2.FREETYPE]"
$ IF F$SEARCH(profile + ".INCLUDE.FREETYPE2]FREETYPE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.FREETYPE2.FREETYPE.CONFIG]"
$ IF F$SEARCH(profile + ".INCLUDE.FREETYPE2.FREETYPE]CONFIG.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.FREETYPE2.FREETYPE.CACHE]"
$ IF F$SEARCH(profile + ".INCLUDE.FREETYPE2.FREETYPE]CACHE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.FREETYPE2.FREETYPE.INTERNAL]"
$ IF F$SEARCH(profile + ".INCLUDE.FREETYPE2.FREETYPE]INTERNAL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.FREETYPE2.FREETYPE.INTERNAL.SERVICES]"
$ IF F$SEARCH(profile + ".INCLUDE.FREETYPE2.FREETYPE.INTERNAL]SERVICES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11]"
$ IF F$SEARCH(profile + ".INCLUDE]X11.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.FONTS]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]FONTS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.EXTENSIONS]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]EXTENSIONS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.ICE]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]ICE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.SM]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]SM.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".LIB.FREETYPE2]"
$ IF F$SEARCH(profile + ".LIB]FREETYPE2.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".LIB.REGEX]"
$ IF F$SEARCH(profile + ".LIB]REGEX.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.ICE]"
$ IF F$SEARCH(profile + ".LIB]ICE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.SM]"
$ IF F$SEARCH(profile + ".LIB]SM.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".CONFIG]"
$ IF F$SEARCH(profile + "]CONFIG.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".CONFIG.UTIL]"
$ IF F$SEARCH(profile + ".CONFIG]UTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".CONFIG.MAKEDEPEND]"
$ IF F$SEARCH(profile + ".CONFIG]MAKEDEPEND.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".FONTS]"
$ IF F$SEARCH(profile + "]FONTS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".FONTS.UTIL]"
$ IF F$SEARCH(profile + ".FONTS]UTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".PROGRAMS.ICEAUTH]"
$ IF F$SEARCH(profile + ".PROGRAMS]ICEAUTH.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.MAKEPSRES]"
$ IF F$SEARCH(profile + ".PROGRAMS]MAKEPSRES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.SHOWRGB]"
$ IF F$SEARCH(profile + ".PROGRAMS]SHOWRGB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.BITMAP]"
$ IF F$SEARCH(profile + ".PROGRAMS]BITMAP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.MKFONTSCALE]"
$ IF F$SEARCH(profile + ".PROGRAMS]MKFONTSCALE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.FONTTOSFNT]"
$ IF F$SEARCH(profile + ".PROGRAMS]FONTTOSFNT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER]"
$ IF F$SEARCH(profile + ".PROGRAMS]XSERVER.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.MI]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]MI.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.FB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]FB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.RENDER]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]RENDER.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.DIX]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]DIX.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]MIEXT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT.DAMAGE]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.MIEXT]DAMAGE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT.SHADOW]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.MIEXT]SHADOW.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.RANDR]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]RANDR.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.DBE]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]DBE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]HW.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW]XFREE86.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.ETC]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]ETC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ WRITE SYS$OUTPUT "X11 component build: " + target + ", native x86-64, pointer64"
$ step = "MMS " + target
$ MMS/DESCRIPTION=[.VMS-SUPPORT]DESCRIP.MMS 'target'
$ result = $STATUS
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Usage: @BUILD [", F$EXTRACT(1,F$LENGTH(targets)-2,targets), "]"
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
