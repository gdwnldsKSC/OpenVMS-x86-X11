$! Original component build entry point. No system installation or driver actions.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ step = "arguments"
$ target = F$EDIT(P1,"UPCASE,TRIM")
$ IF target .EQS. "" THEN target = "ALL"
$ targets = "|ALL|HEADERS|XAU|XDMCP|ZLIB|FONTENC|EXPAT|LBXUTIL|XFONT|FNTSTUBS|FS|ICE|SM|BDFTOPCF|FSLSFONTS|SHOWFONT|XFSINFO|ICEAUTH|SHOWRGB|MAKESTRS|MAKEDEPEND|LNDIR|ATOBM|GTF|FREETYPE2|MKFONTSCALE|FONTTOSFNT|REVPATH|"
$ targets = targets + "UCS2ANY|DAMAGE|MI|CBRT|FB|RENDER|DIX|XPSTUBS|SHADOW|RANDR|DBE|RECORD|LAYER|XFIXES|COMPOSITE|CW|DAMAGEEXT|EXT|XINPUT|OS|MFB|VFB|XKB|LBX|"
$ targets = targets + "CFB|CFB16|CFB24|CFB32|XKBFILE|XKBUI|ROOTLESS|XTRAP|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ targets = targets + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
$ targets = targets + "XSS|XVMC|XXF86VM|XXF86MISC|"
$ targets = targets + "XEVIE|XFONTCACHE|DMX|XP|"
$ targets = targets + "OLDX|XMUU|XPM|XPRINTAPPUTIL|"
$ targets = targets + "X11|XAUTH|XDPYINFO|XWININFO|"
$ targets = targets + "XSET|XMODMAP|XEV|BISON|XKBCOMP|SETXKBMAP|XKB_DATA|XVFB|XSERVER_CORE|XSERVER|"
$ targets = targets + "XT|XMU|MKFONTDIR|FONTS|FONTS_BDF|FONTS_MISC|FONTS_75DPI|FONTS_100DPI|FONTS_CYRILLIC|"
$ targets = targets + "FONTS_ENCODINGS|FONTS_SCALED|FONTS_SPEEDO|FONTS_TYPE1|FONTS_TTF|FONTS_OTF|FONTS_CID|"
$ targets = targets + "NLS|NLS_XLC_LOCALE|NLS_COMPOSE|X11_DATA|RUNTIME|"
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
$ IF F$SEARCH("[.PROTO.XPROTO]XFUNCPROTO.H") .EQS. "" THEN GOTO missing_source
$ step = "create output directories"
$ IF F$SEARCH("[.VMS-SUPPORT]_BUILD.DIR") .EQS. "" THEN CREATE/DIRECTORY [.VMS-SUPPORT._BUILD]
$ profile = "[.VMS-SUPPORT._BUILD.P64"
$ directory = profile + "]"
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD]P64.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE]"
$ IF F$SEARCH(profile + "]INCLUDE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".INCLUDE.X11.XTRANS]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]XTRANS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.XCURSOR]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]XCURSOR.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.XMU]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]XMU.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.XPRINTUTIL]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]XPRINTUTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11.XPRINTAPPUTIL]"
$ IF F$SEARCH(profile + ".INCLUDE.X11]XPRINTAPPUTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB]"
$ IF F$SEARCH(profile + "]LIB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.X11]"
$ IF F$SEARCH(profile + ".LIB]X11.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".LIB.ICE]"
$ IF F$SEARCH(profile + ".LIB]ICE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.SM]"
$ IF F$SEARCH(profile + ".LIB]SM.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XKBFILE]"
$ IF F$SEARCH(profile + ".LIB]XKBFILE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XKBUI]"
$ IF F$SEARCH(profile + ".LIB]XKBUI.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XEXT]"
$ IF F$SEARCH(profile + ".LIB]XEXT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XRENDER]"
$ IF F$SEARCH(profile + ".LIB]XRENDER.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XRANDR]"
$ IF F$SEARCH(profile + ".LIB]XRANDR.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XINERAMA]"
$ IF F$SEARCH(profile + ".LIB]XINERAMA.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XI]"
$ IF F$SEARCH(profile + ".LIB]XI.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XTST]"
$ IF F$SEARCH(profile + ".LIB]XTST.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XV]"
$ IF F$SEARCH(profile + ".LIB]XV.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XRES]"
$ IF F$SEARCH(profile + ".LIB]XRES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XFIXES]"
$ IF F$SEARCH(profile + ".LIB]XFIXES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XDAMAGE]"
$ IF F$SEARCH(profile + ".LIB]XDAMAGE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XCOMPOSITE]"
$ IF F$SEARCH(profile + ".LIB]XCOMPOSITE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XCURSOR]"
$ IF F$SEARCH(profile + ".LIB]XCURSOR.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XSS]"
$ IF F$SEARCH(profile + ".LIB]XSS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XVMC]"
$ IF F$SEARCH(profile + ".LIB]XVMC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XXF86VM]"
$ IF F$SEARCH(profile + ".LIB]XXF86VM.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XXF86MISC]"
$ IF F$SEARCH(profile + ".LIB]XXF86MISC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XEVIE]"
$ IF F$SEARCH(profile + ".LIB]XEVIE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XFONTCACHE]"
$ IF F$SEARCH(profile + ".LIB]XFONTCACHE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.DMX]"
$ IF F$SEARCH(profile + ".LIB]DMX.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XP]"
$ IF F$SEARCH(profile + ".LIB]XP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.OLDX]"
$ IF F$SEARCH(profile + ".LIB]OLDX.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XMUU]"
$ IF F$SEARCH(profile + ".LIB]XMUU.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XPM]"
$ IF F$SEARCH(profile + ".LIB]XPM.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XPRINTAPPUTIL]"
$ IF F$SEARCH(profile + ".LIB]XPRINTAPPUTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT]"
$ IF F$SEARCH(profile + ".LIB]FONT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.UTIL]"
$ IF F$SEARCH(profile + ".LIB.FONT]UTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.FC]"
$ IF F$SEARCH(profile + ".LIB.FONT]FC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.BUILTINS]"
$ IF F$SEARCH(profile + ".LIB.FONT]BUILTINS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.FONTCACHE]"
$ IF F$SEARCH(profile + ".LIB.FONT]FONTCACHE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.FREETYPE]"
$ IF F$SEARCH(profile + ".LIB.FONT]FREETYPE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.SPEEDO]"
$ IF F$SEARCH(profile + ".LIB.FONT]SPEEDO.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".PROGRAMS.XAUTH]"
$ IF F$SEARCH(profile + ".PROGRAMS]XAUTH.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XDPYINFO]"
$ IF F$SEARCH(profile + ".PROGRAMS]XDPYINFO.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XWININFO]"
$ IF F$SEARCH(profile + ".PROGRAMS]XWININFO.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.ICEAUTH]"
$ IF F$SEARCH(profile + ".PROGRAMS]ICEAUTH.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT.LAYER]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.MIEXT]LAYER.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.RANDR]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]RANDR.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.DBE]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]DBE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.RECORD]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]RECORD.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XFIXES]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XFIXES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.COMPOSITE]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]COMPOSITE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]HW.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW]XFREE86.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.ETC]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]ETC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ WRITE SYS$OUTPUT "X11 component build: " + target + ", native x86-64, pointer64"
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT.CW]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.MIEXT]CW.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.DAMAGEEXT]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]DAMAGEEXT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XEXT]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XEXT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XI]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XI.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.OS]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]OS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.MFB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]MFB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.VFB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW]VFB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XKB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XKB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.LBX]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]LBX.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.CFB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]CFB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.CFB16]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]CFB16.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.CFB24]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]CFB24.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.CFB32]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]CFB32.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT.ROOTLESS]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.MIEXT]ROOTLESS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XTRAP]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XTRAP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ step = "MMS " + target
$ directory = profile + ".PROGRAMS.XSET]"
$ IF F$SEARCH(profile + ".PROGRAMS]XSET.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XMODMAP]"
$ IF F$SEARCH(profile + ".PROGRAMS]XMODMAP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XEV]"
$ IF F$SEARCH(profile + ".PROGRAMS]XEV.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XKBCOMP]"
$ IF F$SEARCH(profile + ".PROGRAMS]XKBCOMP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.SETXKBMAP]"
$ IF F$SEARCH(profile + ".PROGRAMS]SETXKBMAP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ server_base = profile + ".PROGRAMS.XSERVER.HW.VFB.SERVER"
$ directory = server_base + "]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.VFB]SERVER.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ server_directories = "DIX|OS|HW|HW.VFB|MI|MIEXT|MIEXT.DAMAGE|FB|RENDER|XKB|XI"
$ server_index = 0
$server_directory_loop:
$ server_directory = F$ELEMENT(server_index,"|",server_directories)
$ IF server_directory .EQS. "|" THEN GOTO server_directories_done
$ directory = server_base + "." + server_directory + "]"
$ check_directory = server_base + "]" + server_directory + ".DIR"
$ IF server_directory .EQS. "HW.VFB" THEN check_directory = server_base + ".HW]VFB.DIR"
$ IF server_directory .EQS. "MIEXT.DAMAGE" THEN check_directory = server_base + ".MIEXT]DAMAGE.DIR"
$ IF F$SEARCH(check_directory) .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ server_index = server_index + 1
$ GOTO server_directory_loop
$server_directories_done:
$ directory = profile + ".LIB.XT]"
$ IF F$SEARCH(profile + ".LIB]XT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XMU]"
$ IF F$SEARCH(profile + ".LIB]XMU.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.MKFONTDIR]"
$ IF F$SEARCH(profile + ".PROGRAMS]MKFONTDIR.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ font_directories = "MISC|75DPI|100DPI|CYRILLIC|ENCODINGS|ENCODINGS.LARGE|SPEEDO|TYPE1|TTF|OTF|CID"
$ font_index = 0
$font_directory_loop:
$ font_directory = F$ELEMENT(font_index,"|",font_directories)
$ IF font_directory .EQS. "|" THEN GOTO font_directories_done
$ directory = profile + ".FONTS." + font_directory + "]"
$ check_directory = profile + ".FONTS]" + font_directory + ".DIR"
$ IF font_directory .EQS. "ENCODINGS.LARGE" THEN -
    check_directory = profile + ".FONTS.ENCODINGS]LARGE.DIR"
$ IF F$SEARCH(check_directory) .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ font_index = font_index + 1
$ GOTO font_directory_loop
$font_directories_done:
$ directory = profile + ".NLS]"
$ IF F$SEARCH(profile + "]NLS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".TOOLS]"
$ IF F$SEARCH(profile + "]TOOLS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".TOOLS.BISON]"
$ IF F$SEARCH(profile + ".TOOLS]BISON.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
