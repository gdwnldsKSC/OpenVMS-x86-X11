$! Original component build entry point. No system installation or driver actions.
$ SET NOON
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ saved_parse_style = ""
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ step = "extended filename parsing"
$ saved_parse_style = F$GETJPI("","PARSE_STYLE_PERM")
$ SET PROCESS/PARSE_STYLE=EXTENDED
$ IF .NOT. $STATUS THEN GOTO failed
$ step = "arguments"
$ target = F$EDIT(P1,"UPCASE,TRIM")
$ IF target .EQS. "" THEN target = "ALL"
$ targets = "|ALL|HEADERS|XAU|XDMCP|ZLIB|FONTENC|EXPAT|XFONT|FNTSTUBS|FS|ICE|SM|ICEAUTH|MAKESTRS|MAKEDEPEND|GTF|FREETYPE2|MKFONTSCALE|"
$ targets = targets + "UCS2ANY|DAMAGE|MI|FB|RENDER|DIX|SHADOW|SYNC|RANDR|DBE|RECORD|XFIXES|COMPOSITE|DAMAGEEXT|EXT|XINPUT|OS|VFB|XKB|"
$ targets = targets + "XKBFILE|ROOTLESS|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ targets = targets + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
$ targets = targets + "XSS|XVMC|XXF86VM|"
$ targets = targets + "DMX|"
$ targets = targets + "XMUU|XPM|"
$ targets = targets + "XCB|XCB_SHAPE|X11|X11_XCB|XAUTH|XDPYINFO|XWININFO|"
$ targets = targets + "XSET|XMODMAP|XEV|BISON|XKBCOMP|SETXKBMAP|XKB_DATA|XVFB|XSERVER_CORE|XSERVER|"
$ targets = targets + "XT|XMU|MKFONTDIR|FONTS|FONTS_BDF|FONTS_MISC|FONTS_75DPI|FONTS_100DPI|FONTS_CYRILLIC|"
$ targets = targets + "FONTS_ENCODINGS|FONTS_SCALED|FONTS_TYPE1|FONTS_TTF|FONTS_OTF|"
$ targets = targets + "NLS|NLS_XLC_LOCALE|NLS_COMPOSE|X11_DATA|RUNTIME|"
$ targets = targets + "XF86_VIDEO_VMWARE|SHADOWFB|VGAHW|RAMDAC|XF86CONFIG|COMMON|BUS|MISC|XORGXKB|"
$ targets = targets + "XF86_VIDEO_FBDEV|"
$ targets = targets + "LOADER|XF86_INPUT_KEYBOARD|XF86_INPUT_MOUSE|"
$ targets = targets + "XF86MODES|DDC|I2C|STUB|PCIACCESS|"
$ targets = targets + "PIXMAN|CONFIG|GAWK|"
$ targets = targets + "REGEX|"
$ targets = targets + "XFONT2|XCVT|XORG|XORG_MODULES|"
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
$ IF F$SEARCH("[.PROTO.XORGPROTO.INCLUDE.X11]XFUNCPROTO.H") .EQS. "" THEN GOTO missing_source
$ IF (target .NES. "XORG") .AND. (target .NES. "XORG_MODULES") THEN GOTO runtime_ready
$ @[.VMS-SUPPORT]BUILD.COM XKBCOMP
$ IF .NOT. $STATUS THEN GOTO failed
$ @[.VMS-SUPPORT]BUILD.COM XKB_DATA
$ IF .NOT. $STATUS THEN GOTO failed
$runtime_ready:
$ step = "create output directories"
$ IF F$SEARCH("[.VMS-SUPPORT]_BUILD.DIR") .EQS. "" THEN CREATE/DIRECTORY [.VMS-SUPPORT._BUILD]
$ profile_name = "P64"
$ IF (target .EQS. "XORG") .OR. (target .EQS. "XORG_MODULES") THEN profile_name = "P64_XORG"
$ profile = "[.VMS-SUPPORT._BUILD." + profile_name
$ directory = profile + "]"
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD]" + profile_name + ".DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".INCLUDE.XCB]"
$ IF F$SEARCH(profile + ".INCLUDE]XCB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11]"
$ pixman_directory = profile + ".INCLUDE.PIXMAN]"
$ IF F$SEARCH(profile + ".INCLUDE]PIXMAN.DIR") .EQS. "" THEN CREATE/DIRECTORY 'pixman_directory'
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
$ directory = profile + ".LIB]"
$ pixman_directory = profile + ".LIB.PIXMAN]"
$ IF F$SEARCH(profile + "]LIB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XCB]"
$ IF F$SEARCH(profile + ".LIB]XCB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.PCIACCESS]"
$ IF F$SEARCH(profile + ".LIB]PCIACCESS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XFONT2]"
$ IF F$SEARCH(profile + ".LIB]XFONT2.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XCVT]"
$ IF F$SEARCH(profile + ".LIB]XCVT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.LIBXCVT]"
$ IF F$SEARCH(profile + ".INCLUDE]LIBXCVT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ IF F$SEARCH(profile + ".LIB]PIXMAN.DIR") .EQS. "" THEN CREATE/DIRECTORY 'pixman_directory'
$ directory = profile + ".LIB.X11]"
$ IF F$SEARCH(profile + ".LIB]X11.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.X11_XCB]"
$ IF F$SEARCH(profile + ".LIB]X11_XCB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XAU]"
$ IF F$SEARCH(profile + ".LIB]XAU.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XDMCP]"
$ IF F$SEARCH(profile + ".LIB]XDMCP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.ZLIB]"
$ IF F$SEARCH(profile + ".LIB]ZLIB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.REGEX]"
$ IF F$SEARCH(profile + ".LIB]REGEX.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONTENC]"
$ IF F$SEARCH(profile + ".LIB]FONTENC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.EXPAT]"
$ IF F$SEARCH(profile + ".LIB]EXPAT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FREETYPE2]"
$ IF F$SEARCH(profile + ".LIB]FREETYPE2.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".LIB.DMX]"
$ IF F$SEARCH(profile + ".LIB]DMX.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XMUU]"
$ IF F$SEARCH(profile + ".LIB]XMUU.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XPM]"
$ IF F$SEARCH(profile + ".LIB]XPM.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT]"
$ IF F$SEARCH(profile + ".LIB]FONT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.UTIL]"
$ IF F$SEARCH(profile + ".LIB.FONT]UTIL.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.FC]"
$ IF F$SEARCH(profile + ".LIB.FONT]FC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.BUILTINS]"
$ IF F$SEARCH(profile + ".LIB.FONT]BUILTINS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.FONT.FREETYPE]"
$ IF F$SEARCH(profile + ".LIB.FONT]FREETYPE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".PROGRAMS.XAUTH]"
$ IF F$SEARCH(profile + ".PROGRAMS]XAUTH.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XDPYINFO]"
$ IF F$SEARCH(profile + ".PROGRAMS]XDPYINFO.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XWININFO]"
$ IF F$SEARCH(profile + ".PROGRAMS]XWININFO.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.ICEAUTH]"
$ IF F$SEARCH(profile + ".PROGRAMS]ICEAUTH.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.MKFONTSCALE]"
$ IF F$SEARCH(profile + ".PROGRAMS]MKFONTSCALE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER]"
$ config_directory = profile + ".PROGRAMS.XSERVER.CONFIG]"
$ IF F$SEARCH(profile + ".PROGRAMS]XSERVER.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]CONFIG.DIR") .EQS. "" THEN CREATE/DIRECTORY 'config_directory'
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
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT.SYNC]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.MIEXT]SYNC.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ directory = profile + ".PROGRAMS.XSERVER.DAMAGEEXT]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]DAMAGEEXT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XEXT]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XEXT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XI]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XI.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.OS]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]OS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.VFB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW]VFB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.XKB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER]XKB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.MIEXT.ROOTLESS]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.MIEXT]ROOTLESS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
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
$ server_directories = "DIX|OS|HW|HW.VFB|MI|MIEXT|MIEXT.DAMAGE|FB|RENDER|XKB|XI|XEXT|RANDR|XFIXES|DAMAGEEXT|SYNC"
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
$ font_directories = "MISC|75DPI|100DPI|CYRILLIC|ENCODINGS|ENCODINGS.LARGE|TYPE1|TTF|OTF"
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
$ directory = profile + ".TOOLS.GAWK]"
$ IF F$SEARCH(profile + ".TOOLS]GAWK.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.SHADOWFB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]SHADOWFB.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.VGAHW]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]VGAHW.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.RAMDAC]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]RAMDAC.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.PARSER]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]PARSER.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.COMMON]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]COMMON.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.MODES]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]MODES.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.DDC]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]DDC.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.I2C]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]I2C.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.LOADER]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]LOADER.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]OS-SUPPORT.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT.BUS]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT]BUS.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT.MISC]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT]MISC.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT.STUB]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT]STUB.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.DIXMODS]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86]DIXMODS.DIR") .EQS. "" -
    THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".DRIVER]"
$ IF F$SEARCH(profile + "]DRIVER.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".DRIVER.XF86-VIDEO-VMWARE]"
$ IF F$SEARCH(profile + ".DRIVER]XF86-VIDEO-VMWARE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".DRIVER.XF86-VIDEO-FBDEV]"
$ IF F$SEARCH(profile + ".DRIVER]XF86-VIDEO-FBDEV.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".DRIVER.XF86-INPUT-KEYBOARD]"
$ IF F$SEARCH(profile + ".DRIVER]XF86-INPUT-KEYBOARD.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".DRIVER.XF86-INPUT-MOUSE]"
$ IF F$SEARCH(profile + ".DRIVER]XF86-INPUT-MOUSE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ IF profile_name .EQS. "P64" THEN GOTO component_build
$ directory = profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT.OPENVMS]"
$ IF F$SEARCH(profile + ".PROGRAMS.XSERVER.HW.XFREE86.OS-SUPPORT]OPENVMS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".IMAGES]"
$ IF F$SEARCH(profile + "]IMAGES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LINK]"
$ IF F$SEARCH(profile + "]LINK.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".IMAGES.MODULES]"
$ IF F$SEARCH(profile + ".IMAGES]MODULES.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".IMAGES.MODULES.DRIVERS]"
$ IF F$SEARCH(profile + ".IMAGES.MODULES]DRIVERS.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".IMAGES.MODULES.INPUT]"
$ IF F$SEARCH(profile + ".IMAGES.MODULES]INPUT.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".IMAGES.CONFIG_D]"
$ IF F$SEARCH(profile + ".IMAGES]CONFIG_D.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ MMS/DESCRIPTION=[.VMS-SUPPORT]DESCRIP.MMS/MACRO=("OUT=[.VMS-SUPPORT._BUILD.P64_XORG","XKB_BIN_DIRECTORY=./VMS-SUPPORT/_BUILD/P64/LIB/X11/XKB","DIX_DDX_DEFINES=,DDXBEFORERESET") 'target'
$ result = $STATUS
$ GOTO done
$component_build:
$ MMS/DESCRIPTION=[.VMS-SUPPORT]DESCRIP.MMS 'target'
$ result = $STATUS
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Usage: @BUILD [target] (default: ALL)"
$ WRITE SYS$OUTPUT "See README.md for the component targets."
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
$ IF saved_parse_style .EQS. "" THEN GOTO restored
$ SET PROCESS/PARSE_STYLE='saved_parse_style'
$ parse_restore_result = $STATUS
$ IF .NOT. parse_restore_result THEN result = parse_restore_result
$restored:
$ EXIT result
