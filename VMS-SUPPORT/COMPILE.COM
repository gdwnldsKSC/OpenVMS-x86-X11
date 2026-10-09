$! Single-object action: source, output base, CFLAGS, header profile, optional CPPFLAGS.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ default_scoped = 0
$ scoped = 0
$ freetype_scoped = 0
$ inherited_freetype = ""
$ inherited_freetype_table = ""
$ first_include = "/NOFIRST_INCLUDE"
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ profiles = "||ZLIB|FONTENC|EXPAT|LBXUTIL_IMAGE|LBXUTIL|FONT_UTIL|FNTSTUBS|FONT_BITMAP|FONT_FONTFILE|FONT_FC|FS|ICE|SM|BDFTOPCF|ICEAUTH|SHOWRGB|MAKEDEPEND|GTF|RTL|"
$ profiles = profiles + "DAMAGE|MI|FB|RENDER|DIX|SHADOW|RANDR|DBE|RECORD|LAYER|XFIXES|COMPOSITE|CW|DAMAGEEXT|EXT|XINPUT|OS|MFB|VFB|XKB|LBX|"
$ profiles = profiles + "CFB|XKBFILE|XKBUI|ROOTLESS|XTRAP|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ profiles = profiles + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
$ profiles = profiles + "XSS|XVMC|XXF86VM|XXF86MISC|"
$ profiles = profiles + "XEVIE|XFONTCACHE|DMX|XP|"
$ profiles = profiles + "OLDX|XMUU|XPM|XPRINTAPPUTIL|"
$ profiles = profiles + "FONT_BUILTINS|FONT_FONTCACHE|FONT_FREETYPE|FONT_SPEEDO|"
$ profiles = profiles + "X11|X11_MAKEKEYS|"
$ profiles = profiles + "XAUTH|XDPYINFO|XWININFO|"
$ profiles = profiles + "XSET|XMODMAP|XEV|XKBCOMP|SETXKBMAP|"
$ profiles = profiles + "XT|XMU|"
$ IF F$LOCATE("|",P4) .NE. F$LENGTH(P4) THEN GOTO done
$ IF F$LOCATE("|" + P4 + "|",profiles) .EQ. F$LENGTH(profiles) THEN GOTO done
$ IF (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ rtl_headers = F$SEARCH("SYS$LIBRARY:DECC$RTLDEF.TLB")
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF P4 .EQS. "OS" THEN GOSUB os_native_headers
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF (P4 .EQS. "GTF") .OR. (P4 .EQS. "RTL") THEN GOTO rtl_headers
$ IF P4 .EQS. "ZLIB" THEN GOTO zlib_headers
$ IF P4 .EQS. "FONTENC" THEN GOTO fontenc_headers
$ inherited_x11 = F$TRNLNM("X11")
$ inherited_table = F$TRNLNM("X11",,,,,"TABLE_NAME")
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]"
$ IF (P4 .EQS. "X11") .OR. (P4 .EQS. "X11_MAKEKEYS") THEN -
    include_directory = include_directory + ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]"
$ IF P4 .EQS. "X11" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS],[.LIB.LIBX11.SRC]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.X11],[.LIB.LIBXAU],[.LIB.XTRANS]," + -
    """./lib/libX11/src/xlibi18n"""
$ IF P4 .EQS. "X11" THEN include_directory = include_directory + -
    ",[.LIB.LIBX11.INCLUDE.X11],[.LIB.LIBX11.SRC.XCMS]," + -
    "[.LIB.LIBX11.SRC.XKB],[.LIB.LIBX11.SRC.XLIBI18N]"
$ IF P4 .EQS. "XKB" THEN include_directory = -
    "[.XSERVER.XORG-SERVER.XKB]," + include_directory
$ IF P4 .EQS. "EXPAT" THEN include_directory = include_directory + -
    ",[.EXTRAS.EXPAT.LIB],[.EXTRAS.EXPAT.VMS]"
$ IF P4 .EQS. "LBXUTIL_IMAGE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.LBXUTIL.IMAGE],[.LIB.LIBLBXUTIL.SRC.IMAGE]"
$ IF P4 .EQS. "LBXUTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.XSERVER.XORG-SERVER.INCLUDE],[.LIB.LIBLBXUTIL.SRC.LBX_ZLIB]"
$ IF P4 .EQS. "FONT_UTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.INCLUDE.X11.FONTS]"
$ font_renderers = "|FONT_BUILTINS|FONT_FONTCACHE|FONT_FREETYPE|FONT_SPEEDO|"
$ IF F$LOCATE("|" + P4 + "|",font_renderers) .LT. F$LENGTH(font_renderers) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11],[.LIB.LIBXFONT.INCLUDE.X11.FONTS]"
$ IF P4 .EQS. "FONT_BUILTINS" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFONT.SRC.BUILTINS],[.LIB.LIBXFONT.SRC.BITMAP]"
$ IF P4 .EQS. "FONT_FONTCACHE" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFONT.SRC.FONTCACHE]"
$ IF P4 .EQS. "FONT_SPEEDO" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFONT.SRC.SPEEDO]"
$ IF P4 .EQS. "FONT_FREETYPE" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFONT.SRC.FREETYPE],[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2.FREETYPE.CONFIG]"
$ IF P4 .EQS. "FNTSTUBS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.STUBS]"
$ IF P4 .EQS. "FONT_FC" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.FC],[.LIB.XTRANS]"
$ IF P4 .EQS. "FS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.LIBFS.SRC],[.LIB.XTRANS]"
$ IF P4 .EQS. "ICE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.ICE],[.LIB.LIBICE.SRC],[.LIB.XTRANS]"
$ IF P4 .EQS. "SM" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.SM],[.LIB.LIBSM.SRC]"
$ IF (P4 .EQS. "XKBFILE") .OR. (P4 .EQS. "XKBUI") THEN -
    include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.LIB.LIBXKBFILE.SRC]"
$ IF P4 .EQS. "XKBUI" THEN include_directory = include_directory + -
    ",[.LIB.LIBXKBUI.SRC]"
$ IF P4 .EQS. "XT" THEN include_directory = -
    "[.VMS-SUPPORT._BUILD.P64.LIB.XT],[.LIB.LIBXT.SRC]," + include_directory
$ IF P4 .EQS. "XMU" THEN include_directory = -
    "[.LIB.LIBXMU.SRC],[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.XMU]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.XT]," + include_directory
$ client_extensions = "|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ client_extensions = client_extensions + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
$ client_extensions = client_extensions + "XSS|XVMC|XXF86VM|XXF86MISC|"
$ client_extensions = client_extensions + "XEVIE|XFONTCACHE|DMX|XP|"
$ client_extensions = client_extensions + "OLDX|XMUU|XPM|XPRINTAPPUTIL|"
$ client_extensions = client_extensions + "XAUTH|XDPYINFO|XWININFO|"
$ client_extensions = client_extensions + "XSET|XMODMAP|XEV|XKBCOMP|SETXKBMAP|"
$ client_extensions = client_extensions + "XT|XMU|"
$ IF F$LOCATE("|" + P4 + "|",client_extensions) .LT. F$LENGTH(client_extensions) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS],[.LIB.LIBX11.SRC]"
$ IF P4 .EQS. "XEXT" THEN include_directory = include_directory + -
    ",[.LIB.LIBXEXT.SRC]"
$ IF P4 .EQS. "XRENDER" THEN include_directory = include_directory + -
    ",[.LIB.LIBXRENDER.SRC],[.VMS-SUPPORT._BUILD.P64.LIB.XRENDER]"
$ IF P4 .EQS. "XRANDR" THEN include_directory = include_directory + -
    ",[.LIB.LIBXRANDR.SRC]"
$ IF P4 .EQS. "XINERAMA" THEN include_directory = include_directory + -
    ",[.LIB.LIBXINERAMA.SRC]"
$ IF P4 .EQS. "XI" THEN include_directory = include_directory + -
    ",[.LIB.LIBXI.SRC]"
$ IF P4 .EQS. "XTST" THEN include_directory = include_directory + -
    ",[.LIB.LIBXTST.SRC]"
$ IF P4 .EQS. "XV" THEN include_directory = include_directory + -
    ",[.LIB.LIBXV.SRC]"
$ IF P4 .EQS. "XRES" THEN include_directory = include_directory + -
    ",[.LIB.LIBXRES.SRC]"
$ IF P4 .EQS. "XFIXESLIB" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFIXES.SRC]"
$ IF P4 .EQS. "XDAMAGE" THEN include_directory = include_directory + -
    ",[.LIB.LIBXDAMAGE.SRC]"
$ IF P4 .EQS. "XCOMPOSITE" THEN include_directory = include_directory + -
    ",[.LIB.LIBXCOMPOSITE.SRC]"
$ IF P4 .EQS. "XCURSOR" THEN include_directory = include_directory + -
    ",[.LIB.LIBXCURSOR.SRC],[.VMS-SUPPORT._BUILD.P64.LIB.XCURSOR]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.XCURSOR]"
$ IF P4 .EQS. "XSS" THEN include_directory = include_directory + -
    ",[.LIB.LIBXSCRNSAVER.SRC]"
$ IF P4 .EQS. "XVMC" THEN include_directory = include_directory + -
    ",[.LIB.LIBXVMC.SRC]"
$ IF P4 .EQS. "XXF86VM" THEN include_directory = include_directory + -
    ",[.LIB.LIBXXF86VM.SRC]"
$ IF P4 .EQS. "XXF86MISC" THEN include_directory = include_directory + -
    ",[.LIB.LIBXXF86MISC.SRC]"
$ IF P4 .EQS. "XEVIE" THEN include_directory = include_directory + -
    ",[.LIB.LIBXEVIE.SRC]"
$ IF P4 .EQS. "XFONTCACHE" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFONTCACHE.SRC]"
$ IF P4 .EQS. "DMX" THEN include_directory = include_directory + -
    ",[.LIB.LIBDMX.SRC]"
$ IF P4 .EQS. "XP" THEN include_directory = include_directory + -
    ",[.LIB.LIBXP.SRC]"
$ IF P4 .EQS. "OLDX" THEN include_directory = include_directory + -
    ",[.LIB.LIBOLDX.SRC]"
$ IF P4 .EQS. "XMUU" THEN include_directory = include_directory + -
    ",[.LIB.LIBXMU.SRC]"
$ IF P4 .EQS. "XPM" THEN include_directory = include_directory + -
    ",[.LIB.LIBXPM.SRC]"
$ IF P4 .EQS. "XPRINTAPPUTIL" THEN include_directory = include_directory + -
    ",[.LIB.LIBXPRINTAPPUTIL.SRC]"
$ IF P4 .EQS. "XPRINTAPPUTIL" THEN GOSUB xprintapputil_native
$ IF (P4 .EQS. "XPRINTAPPUTIL") .AND. (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "ICEAUTH" THEN include_directory = include_directory + -
    ",[.APP.ICEAUTH]"
$ IF P4 .EQS. "XAUTH" THEN include_directory = -
    "[.APP.XAUTH]," + include_directory
$ IF P4 .EQS. "XWININFO" THEN include_directory = include_directory + -
    ",[.APP.XWININFO],[.APP.XLSFONTS]"
$ IF P4 .EQS. "XSET" THEN include_directory = include_directory + ",[.APP.XSET]"
$ IF P4 .EQS. "XMODMAP" THEN include_directory = include_directory + ",[.APP.XMODMAP]"
$ IF P4 .EQS. "XEV" THEN include_directory = include_directory + ",[.APP.XEV]"
$ IF P4 .EQS. "XKBCOMP" THEN include_directory = -
    "[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XKBCOMP],[.APP.XKBCOMP]," + include_directory
$ IF P4 .EQS. "SHOWRGB" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.INCLUDE]"
$ IF P4 .EQS. "MAKEDEPEND" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11],[.UTIL.MAKEDEPEND],[.UTIL.IMAKE]"
$ server_profiles = "|DAMAGE|MI|FB|RENDER|DIX|SHADOW|RANDR|DBE|RECORD|LAYER|XFIXES|COMPOSITE|CW|DAMAGEEXT|EXT|XINPUT|OS|MFB|VFB|XKB|LBX|CFB|ROOTLESS|XTRAP|"
$! Native case folding makes server xfixes.h collide with client Xfixes.h.
$! Keep the server header ahead of the exported client extension directory.
$ IF F$LOCATE("|" + P4 + "|",server_profiles) .LT. F$LENGTH(server_profiles) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.XSERVER.XORG-SERVER.XFIXES]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.XSERVER.XORG-SERVER.MIEXT.DAMAGE],[.XSERVER.XORG-SERVER.INCLUDE]," + -
    "[.XSERVER.XORG-SERVER.MI],[.XSERVER.XORG-SERVER.RENDER]"
$ IF P4 .EQS. "MI" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MFB],[.XSERVER.XORG-SERVER.XEXT]," + -
    "[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.DAMAGEEXT]"
$ IF (P4 .EQS. "FB") .OR. (P4 .EQS. "RENDER") .OR. -
    (P4 .EQS. "SHADOW") .OR. (P4 .EQS. "LAYER") THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.FB],[.XSERVER.XORG-SERVER.XEXT]"
$ IF (P4 .EQS. "SHADOW") .OR. (P4 .EQS. "LAYER") THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MIEXT.SHADOW]"
$ IF P4 .EQS. "LAYER" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MIEXT.LAYER]"
$ IF P4 .EQS. "LAYER" THEN GOSUB layer_names
$ IF (P4 .EQS. "LAYER") .AND. (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "RANDR" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.RANDR]"
$ IF P4 .EQS. "DBE" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.DBE]"
$ IF P4 .EQS. "RECORD" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.RECORD],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "XFIXES" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "COMPOSITE" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.COMPOSITE],[.XSERVER.XORG-SERVER.XFIXES]," + -
    "[.XSERVER.XORG-SERVER.DAMAGEEXT],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "DIX" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.DIX],[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.LBX]"
$ IF P4 .EQS. "CW" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MIEXT.CW],[.XSERVER.XORG-SERVER.FB]," + -
    "[.XSERVER.XORG-SERVER.COMPOSITE],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "DAMAGEEXT" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.DAMAGEEXT],[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "EXT" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.HW.XFREE86.DIXMODS.EXTMOD],[.XSERVER.XORG-SERVER.XPRINT]"
$ IF P4 .EQS. "XINPUT" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XI],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "OS" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.OS],[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.LBX]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXAU],[.LIB.XTRANS]"
$ IF P4 .EQS. "MFB" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MFB]"
$ IF P4 .EQS. "VFB" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.HW.VFB],[.XSERVER.XORG-SERVER.FB]," + -
    "[.XSERVER.XORG-SERVER.MFB],[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.XI]"
$ IF P4 .EQS. "XKB" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XSERVER.XKB]," + -
    "[.XSERVER.XORG-SERVER.XKB],[.XSERVER.XORG-SERVER.HW.XFREE86.DIXMODS.EXTMOD]," + -
    "[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.XI]"
$! Slash-bearing upstream includes require a Unix-style search directory.
$ IF P4 .EQS. "XKB" THEN include_directory = include_directory + -
    ",""./VMS-SUPPORT/_BUILD/P64/INCLUDE/X11"""
$ IF P4 .EQS. "LBX" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.LBX],[.XSERVER.XORG-SERVER.XEXT],[.LIB.LIBXFONT.INCLUDE.X11.FONTS]"
$ IF P4 .EQS. "CFB" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.CFB],[.XSERVER.XORG-SERVER.CFB24]," + -
    "[.XSERVER.XORG-SERVER.MFB],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "CFB" THEN GOSUB cfb_native
$ IF (P4 .EQS. "CFB") .AND. (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "ROOTLESS" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MIEXT.ROOTLESS],[.XSERVER.XORG-SERVER.FB]," + -
    "[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "ROOTLESS" THEN GOSUB rootless_config
$ IF (P4 .EQS. "ROOTLESS") .AND. (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "XTRAP" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XTRAP],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "FONT_FONTFILE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.FONTFILE]"
$ IF (P4 .EQS. "FONT_BITMAP") .OR. (P4 .EQS. "BDFTOPCF") THEN -
    include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.BITMAP]"
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XFUNCPROTO.H")
$ IF header .EQS. "" THEN GOTO done
$ headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS X11 'headers'
$ scoped = 1
$ IF F$TRNLNM("X11","LNM$FILE_DEV") .NES. headers THEN GOTO done
$ IF P4 .EQS. "FONT_FREETYPE" THEN GOTO freetype_headers
$ GOTO compile
$freetype_headers:
$ inherited_freetype = F$TRNLNM("FREETYPE")
$ inherited_freetype_table = F$TRNLNM("FREETYPE",,,,,"TABLE_NAME")
$ IF F$TRNLNM("FREETYPE","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.FREETYPE2.FREETYPE]FREETYPE.H")
$ IF header .EQS. "" THEN GOTO done
$ freetype_headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS FREETYPE 'freetype_headers'
$ freetype_scoped = 1
$ IF F$TRNLNM("FREETYPE","LNM$FILE_DEV") .NES. freetype_headers THEN GOTO done
$ GOTO compile
$rtl_headers:
$! These utilities need only the explicit native RTL header library, not X11.
$ include_directory = """"""
$ GOTO compile
$zlib_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]ZLIB.H") .EQS. "" THEN GOTO done
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]ZCONF.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE],[.EXTRAS.ZLIB]"
$ GOTO compile
$fontenc_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]FONTENC.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.LIBFONTENC.SRC]," + -
    """./VMS-SUPPORT/_BUILD/P64/INCLUDE"""
$ GOTO compile
$compile:
$! Each modular source directory contains its own private headers.
$! Keep staged public/configured headers earlier in the explicit search list.
$ IF (P4 .NES. "GTF") .AND. (P4 .NES. "RTL") THEN -
    include_directory = include_directory + "," + -
    F$PARSE(P1,,,"DEVICE") + F$PARSE(P1,,,"DIRECTORY")
$ IF (P4 .EQS. "MI") .OR. (P4 .EQS. "EXT") THEN GOSUB mi_context
$ CC 'P3' 'P5' 'first_include' /NESTED_INCLUDE_DIRECTORY=NONE -
    /INCLUDE_DIRECTORY=('include_directory',"") -
    /LIST='P2'.LIS/SHOW=INCLUDE -
    /MMS_DEPENDENCIES=(FILE='P2'.MMS,SYSTEM_INCLUDE_FILES) -
    /OBJECT='P2'.OBJ 'P1' + 'rtl_headers'/LIBRARY
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ IF .NOT. scoped THEN GOTO done
$ IF F$TRNLNM("X11") .NES. inherited_x11 THEN result = 2
$ IF F$TRNLNM("X11",,,,,"TABLE_NAME") .NES. inherited_table THEN result = 2
$ IF freetype_scoped .AND. (F$TRNLNM("FREETYPE") .NES. inherited_freetype) THEN result = 2
$ IF freetype_scoped .AND. -
    (F$TRNLNM("FREETYPE",,,,,"TABLE_NAME") .NES. inherited_freetype_table) THEN result = 2
$ GOTO done
$layer_names:
$ layer_names_header = F$SEARCH("[.VMS-SUPPORT.PROGRAMS.XSERVER.MIEXT.LAYER]VMS_LAYER_NAMES.H")
$ IF layer_names_header .NES. "" THEN first_include = "/FIRST_INCLUDE=" + layer_names_header
$ RETURN
$cfb_native:
$ cfb_native_header = F$SEARCH("[.VMS-SUPPORT.PROGRAMS.XSERVER.CFB]VMS_CFB_NATIVE.H")
$ IF cfb_native_header .NES. "" THEN first_include = "/FIRST_INCLUDE=" + cfb_native_header
$ RETURN
$rootless_config:
$ rootless_config_header = F$SEARCH("[.VMS-SUPPORT.PROGRAMS.XSERVER.MIEXT.ROOTLESS]VMS_ROOTLESS_CONFIG.H")
$ IF rootless_config_header .NES. "" THEN first_include = "/FIRST_INCLUDE=" + rootless_config_header
$ RETURN
$xprintapputil_native:
$ xprintapputil_native_header = F$SEARCH("[.VMS-SUPPORT.LIB.XPRINTAPPUTIL]VMS_XPRINTAPPUTIL_NATIVE.H")
$ IF xprintapputil_native_header .NES. "" THEN first_include = "/FIRST_INCLUDE=" + xprintapputil_native_header
$ RETURN
$os_native_headers:
$ starlet_headers = F$SEARCH("SYS$LIBRARY:SYS$STARLET_C.TLB")
$ IF starlet_headers .EQS. "" THEN rtl_headers = ""
$ IF starlet_headers .NES. "" THEN rtl_headers = rtl_headers + "/LIBRARY+" + starlet_headers
$ RETURN
$mi_context:
$! Match the Unix module working directory for ../mfb/maskbits.h.
$! Root every explicit input/output/include before changing the default.
$ P1 = F$PARSE(P1)
$ P2 = F$PARSE(P2,,,"DEVICE") + F$PARSE(P2,,,"DIRECTORY") + F$PARSE(P2,,,"NAME")
$ rooted_includes = ""
$ include_index = 0
$mi_include_loop:
$ include_part = F$ELEMENT(include_index,",",include_directory)
$ IF include_part .EQS. "," THEN GOTO mi_context_ready
$ IF rooted_includes .NES. "" THEN rooted_includes = rooted_includes + ","
$ rooted_includes = rooted_includes + F$PARSE(include_part,,,"DEVICE") + -
    F$PARSE(include_part,,,"DIRECTORY")
$ include_index = include_index + 1
$ GOTO mi_include_loop
$mi_context_ready:
$ include_directory = rooted_includes
$ source_directory = F$PARSE(P1,,,"DEVICE") + F$PARSE(P1,,,"DIRECTORY")
$ default_scoped = 1
$ SET DEFAULT 'source_directory'
$ RETURN
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ IF default_scoped THEN SET DEFAULT 'saved_default'
$ IF freetype_scoped .AND. (F$TRNLNM("FREETYPE","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS FREETYPE
$ IF scoped .AND. (F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS X11
$ IF .NOT. result THEN WRITE SYS$OUTPUT "X11 component compile failed: " + P1
$ EXIT result
