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
$ build_profile = "P64"
$ IF F$LOCATE("._BUILD.P64_XORG.",F$EDIT(P2,"UPCASE")) .LT. F$LENGTH(P2) THEN build_profile = "P64_XORG"
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ profiles = "||ZLIB|FONTENC|EXPAT|FONT_UTIL|FNTSTUBS|FONT_BITMAP|FONT_FONTFILE|FONT_FC|FS|ICE|SM|ICEAUTH|MAKEDEPEND|GTF|RTL|"
$ profiles = profiles + "DAMAGE|MI|FB|RENDER|DIX|SHADOW|SYNC|RANDR|DBE|RECORD|XFIXES|COMPOSITE|DAMAGEEXT|EXT|XINPUT|OS|VFB|XKB|"
$ profiles = profiles + "XKBFILE|ROOTLESS|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ profiles = profiles + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
$ profiles = profiles + "XSS|XVMC|XXF86VM|"
$ profiles = profiles + "DMX|"
$ profiles = profiles + "XMUU|XPM|"
$ profiles = profiles + "FONT_BUILTINS|FONT_FREETYPE|"
$ profiles = profiles + "X11|X11_MAKEKEYS|XCB|"
$ profiles = profiles + "XAUTH|XDPYINFO|XWININFO|"
$ profiles = profiles + "XSET|XMODMAP|XEV|XKBCOMP|SETXKBMAP|"
$ profiles = profiles + "XT|XMU|"
$ profiles = profiles + "XF86_VIDEO_VMWARE|SHADOWFB|VGAHW|RAMDAC|XF86CONFIG|COMMON|BUS|MISC|XORGXKB|"
$ profiles = profiles + "LOADER|"
$ profiles = profiles + "XF86MODES|DDC|I2C|STUB|PCIACCESS|"
$ profiles = profiles + "PCIACCESS_NATIVE|"
$ profiles = profiles + "XF86_INPUT_KEYBOARD|XF86_INPUT_MOUSE|"
$ profiles = profiles + "PIXMAN|CONFIG|"
$ profiles = profiles + "REGEX|REGEX_MKH|"
$ profiles = profiles + "XFONT2|XCVT|RTL_LOCAL|"
$ IF F$LOCATE("|",P4) .NE. F$LENGTH(P4) THEN GOTO done
$ IF F$LOCATE("|" + P4 + "|",profiles) .EQ. F$LENGTH(profiles) THEN GOTO done
$ IF (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ rtl_headers = F$SEARCH("SYS$LIBRARY:DECC$RTLDEF.TLB")
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF (P4 .EQS. "OS") .OR. (P4 .EQS. "XCB") .OR. (P4 .EQS. "PIXMAN") .OR. -
     (P4 .EQS. "XFONT2") .OR. -
     (P4 .EQS. "X11") .OR. (P4 .EQS. "XWININFO") .OR. -
     (P4 .EQS. "XDPYINFO") THEN GOSUB os_native_headers
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF (P4 .EQS. "GTF") .OR. (P4 .EQS. "RTL") .OR. (P4 .EQS. "RTL_LOCAL") THEN GOTO rtl_headers
$ IF P4 .EQS. "ZLIB" THEN GOTO zlib_headers
$ IF P4 .EQS. "REGEX" THEN GOTO regex_headers
$ IF P4 .EQS. "REGEX_MKH" THEN GOTO rtl_headers
$ IF P4 .EQS. "FONTENC" THEN GOTO fontenc_headers
$ IF P4 .EQS. "PIXMAN" THEN GOTO pixman_headers
$ IF P4 .EQS. "XCVT" THEN GOTO xcvt_headers
$ IF P4 .EQS. "PCIACCESS" THEN GOTO pciaccess_headers
$ IF P4 .EQS. "PCIACCESS_NATIVE" THEN GOTO pciaccess_headers
$ inherited_x11 = F$TRNLNM("X11")
$ inherited_table = F$TRNLNM("X11",,,,,"TABLE_NAME")
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]"
$ IF (P4 .EQS. "XCB") .OR. (P4 .EQS. "X11") .OR. (P4 .EQS. "XWININFO") .OR. -
     (P4 .EQS. "XDPYINFO") THEN -
    include_directory = "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.XCB]," + -
    include_directory + ",""./VMS-SUPPORT/_BUILD/''build_profile'/INCLUDE"""
$ IF P4 .EQS. "XCB" THEN include_directory = -
    "[.VMS-SUPPORT.LIB.XCB],[.VMS-SUPPORT._BUILD.''build_profile'.LIB.XCB]," + -
    "[.XCB.LIBXCB.SRC],[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.XCB]," + include_directory
$ IF P4 .EQS. "XF86CONFIG" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.HW.XFREE86.PARSER],[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]"
$ IF (P4 .EQS. "X11") .OR. (P4 .EQS. "X11_MAKEKEYS") THEN -
    include_directory = include_directory + ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11]"
$ IF P4 .EQS. "X11_MAKEKEYS" THEN include_directory = include_directory + -
    ",""./lib/libX11/src/util"""
$ IF P4 .EQS. "X11" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.EXTENSIONS],[.LIB.LIBX11.SRC]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.LIB.X11],[.LIB.LIBXAU],[.LIB.XTRANS]," + -
    """./lib/libX11/src/xlibi18n"""
$ IF P4 .EQS. "X11" THEN include_directory = include_directory + -
    ",[.LIB.LIBX11.INCLUDE.X11],[.LIB.LIBX11.SRC.XCMS]," + -
    "[.LIB.LIBX11.SRC.XKB],[.LIB.LIBX11.SRC.XLIBI18N]"
$ IF P4 .EQS. "XKB" THEN include_directory = -
    "[.XSERVER.XORG-SERVER.XKB]," + include_directory
$ IF P4 .EQS. "EXPAT" THEN include_directory = include_directory + -
    ",[.EXTRAS.EXPAT.LIB],[.EXTRAS.EXPAT.VMS]"
$ IF P4 .EQS. "FONT_UTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.STUBS]"
$ font_renderers = "|FONT_BUILTINS|FONT_FREETYPE|"
$ IF F$LOCATE("|" + P4 + "|",font_renderers) .LT. F$LENGTH(font_renderers) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11],[.LIB.LIBXFONT.INCLUDE.X11.FONTS]"
$ IF P4 .EQS. "FONT_BUILTINS" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFONT.SRC.BUILTINS],[.LIB.LIBXFONT.SRC.BITMAP]"
$ IF P4 .EQS. "FONT_FREETYPE" THEN include_directory = include_directory + -
    ",[.LIB.LIBXFONT.SRC.FREETYPE],[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.FREETYPE2]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.FREETYPE2.FREETYPE.CONFIG]"
$ IF P4 .EQS. "XFONT2" THEN include_directory = -
    "[.VMS-SUPPORT.LIB.XFONT2],[.LIB.LIBXFONT2.INCLUDE]," + -
    "[.LIB.LIBXFONT2.INCLUDE.X11.FONTS]," + include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.FREETYPE2]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.FREETYPE2.FREETYPE.CONFIG]," + -
    "[.LIB.LIBXFONT2.SRC.BITMAP],[.LIB.XTRANS]," + -
    """./lib/libXfont2"",""./VMS-SUPPORT/_BUILD/''build_profile'/INCLUDE"""
$ IF P4 .EQS. "FNTSTUBS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.STUBS]"
$ IF P4 .EQS. "FONT_FC" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.FC],[.LIB.XTRANS]"
$ IF P4 .EQS. "FS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS],[.LIB.LIBFS.SRC],[.LIB.XTRANS]"
$ IF P4 .EQS. "ICE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.ICE],[.LIB.LIBICE.SRC],[.LIB.XTRANS]"
$ IF P4 .EQS. "SM" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.SM],[.LIB.LIBSM.SRC]"
$ IF P4 .EQS. "XKBFILE" THEN -
    include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.EXTENSIONS]," + -
    "[.LIB.LIBXKBFILE.SRC]"
$ IF P4 .EQS. "XKBFILE" THEN GOSUB xkbfile_native
$ IF (P4 .EQS. "XKBFILE") .AND. -
    (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "XT" THEN include_directory = -
    "[.VMS-SUPPORT._BUILD.''build_profile'.LIB.XT],[.LIB.LIBXT.SRC]," + include_directory
$ IF P4 .EQS. "XMU" THEN include_directory = -
    "[.LIB.LIBXMU.SRC],[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.XMU]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.LIB.XT]," + include_directory
$ client_extensions = "|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ client_extensions = client_extensions + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
$ client_extensions = client_extensions + "XSS|XVMC|XXF86VM|"
$ client_extensions = client_extensions + "DMX|"
$ client_extensions = client_extensions + "XMUU|XPM|"
$ client_extensions = client_extensions + "XAUTH|XDPYINFO|XWININFO|"
$ client_extensions = client_extensions + "XSET|XMODMAP|XEV|XKBCOMP|SETXKBMAP|"
$ client_extensions = client_extensions + "XT|XMU|"
$ IF F$LOCATE("|" + P4 + "|",client_extensions) .LT. F$LENGTH(client_extensions) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.EXTENSIONS],[.LIB.LIBX11.SRC]"
$ IF P4 .EQS. "XEXT" THEN include_directory = include_directory + -
    ",[.LIB.LIBXEXT.SRC]"
$ IF P4 .EQS. "XRENDER" THEN include_directory = include_directory + -
    ",[.LIB.LIBXRENDER.SRC],[.VMS-SUPPORT._BUILD.''build_profile'.LIB.XRENDER]"
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
    ",[.LIB.LIBXCURSOR.SRC],[.VMS-SUPPORT._BUILD.''build_profile'.LIB.XCURSOR]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.XCURSOR]"
$ IF P4 .EQS. "XSS" THEN include_directory = include_directory + -
    ",[.LIB.LIBXSCRNSAVER.SRC]"
$ IF P4 .EQS. "XVMC" THEN include_directory = include_directory + -
    ",[.LIB.LIBXVMC.SRC]"
$ IF P4 .EQS. "XXF86VM" THEN include_directory = include_directory + -
    ",[.LIB.LIBXXF86VM.SRC]"
$ IF P4 .EQS. "DMX" THEN include_directory = include_directory + -
    ",[.LIB.LIBDMX.SRC]"
$ IF P4 .EQS. "XMUU" THEN include_directory = include_directory + -
    ",[.LIB.LIBXMU.SRC]"
$ IF P4 .EQS. "XPM" THEN include_directory = include_directory + -
    ",[.LIB.LIBXPM.SRC]"
$ IF P4 .EQS. "ICEAUTH" THEN include_directory = include_directory + -
    ",[.APP.ICEAUTH]"
$ IF P4 .EQS. "XAUTH" THEN include_directory = -
    "[.APP.XAUTH]," + include_directory
$ IF P4 .EQS. "XWININFO" THEN include_directory = -
    "[.VMS-SUPPORT._BUILD.''build_profile'.PROGRAMS.XWININFO]," + include_directory + -
    ",[.APP.XWININFO]"
$ IF P4 .EQS. "XSET" THEN include_directory = include_directory + ",[.APP.XSET]"
$ IF P4 .EQS. "XMODMAP" THEN include_directory = -
    "[.VMS-SUPPORT._BUILD.''build_profile'.PROGRAMS.XMODMAP]," + include_directory + ",[.APP.XMODMAP]"
$ IF P4 .EQS. "XEV" THEN include_directory = include_directory + ",[.APP.XEV]"
$ IF P4 .EQS. "XKBCOMP" THEN include_directory = -
    "[.VMS-SUPPORT._BUILD.''build_profile'.PROGRAMS.XKBCOMP],[.APP.XKBCOMP]," + include_directory
$ IF P4 .EQS. "MAKEDEPEND" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11],[.UTIL.MAKEDEPEND]"
$ server_profiles = "|DAMAGE|MI|FB|RENDER|DIX|SHADOW|SYNC|RANDR|DBE|RECORD|XFIXES|COMPOSITE|DAMAGEEXT|EXT|XINPUT|OS|VFB|XKB|ROOTLESS|XF86CONFIG|"
$ server_profiles = server_profiles + "SHADOWFB|VGAHW|RAMDAC|COMMON|BUS|MISC|XORGXKB|"
$ server_profiles = server_profiles + "LOADER|"
$ server_profiles = server_profiles + "XF86MODES|DDC|I2C|STUB|"
$ server_profiles = server_profiles + "XF86_INPUT_KEYBOARD|XF86_INPUT_MOUSE|"
$ server_profiles = server_profiles + "CONFIG|"
$ IF (F$LOCATE("|" + P4 + "|",server_profiles) .LT. F$LENGTH(server_profiles)) .OR. -
     (P4 .EQS. "XF86_VIDEO_VMWARE") THEN P3 = P3 + "/UNDEFINE=__HIDE_FORBIDDEN_NAMES"
$ IF (F$LOCATE("|" + P4 + "|",server_profiles) .LT. F$LENGTH(server_profiles)) .OR. -
     (P4 .EQS. "XF86_VIDEO_VMWARE") THEN GOSUB os_native_headers
$ IF rtl_headers .EQS. "" THEN GOTO done
$! Native case folding makes server headers collide with client Xfixes/XKB files.
$! Keep server include directories ahead of exported client extension headers.
$ IF F$LOCATE("|" + P4 + "|",server_profiles) .LT. F$LENGTH(server_profiles) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.RANDR]," + -
    "[.XSERVER.XORG-SERVER.DAMAGEEXT]," + -
    "[.XSERVER.XORG-SERVER.INCLUDE],[.XSERVER.XORG-SERVER.XEXT]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.EXTENSIONS]," + -
    "[.XSERVER.XORG-SERVER.MIEXT.DAMAGE],[.XSERVER.XORG-SERVER.MIEXT.SYNC]," + -
    "[.XSERVER.XORG-SERVER.MI],[.XSERVER.XORG-SERVER.RENDER]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.PIXMAN],[.XSERVER.XORG-SERVER.XI]"
$ IF P4 .EQS. "MI" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XEXT]," + -
    "[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.DAMAGEEXT]"
$ IF P4 .EQS. "CONFIG" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.CONFIG]"
$ IF P4 .EQS. "XF86_INPUT_KEYBOARD" THEN include_directory = include_directory + -
    ",[.DRIVER.XF86-INPUT-KEYBOARD.SRC]"
$ IF P4 .EQS. "XF86_INPUT_MOUSE" THEN include_directory = include_directory + -
    ",[.DRIVER.XF86-INPUT-MOUSE.SRC],[.DRIVER.XF86-INPUT-MOUSE.INCLUDE]"
$ IF P4 .EQS. "SHADOWFB" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.BUS]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.SHADOWFB]"
$ IF P4 .EQS. "VGAHW" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.BUS]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.DDC]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.I2C]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.VGAHW]"
$ IF P4 .EQS. "RAMDAC" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.BUS]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.RAMDAC]"
$ common_profiles = "|COMMON|BUS|MISC|XORGXKB|XF86CONFIG|SHADOWFB|VGAHW|RAMDAC|"
$ common_profiles = common_profiles + "LOADER|"
$ common_profiles = common_profiles + "XF86MODES|DDC|I2C|STUB|"
$ common_profiles = common_profiles + "XF86_INPUT_KEYBOARD|XF86_INPUT_MOUSE|"
$ xf86common_profile = F$LOCATE("|" + P4 + "|",common_profiles) .LT. F$LENGTH(common_profiles)
$ IF xf86common_profile THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.PROGRAMS.XSERVER.HW.XFREE86.COMMON]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.BUS]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.DDC],[.XSERVER.XORG-SERVER.HW.XFREE86.I2C]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.LOADER]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.PARSER]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.MODES],[.XSERVER.XORG-SERVER.HW.XFREE86.RAMDAC]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.INT10]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.VGAHW]," + -
    "[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.RANDR],[.XSERVER.XORG-SERVER.XKB]," + -
    "[.XSERVER.XORG-SERVER.XI],[.XSERVER.XORG-SERVER.OS],[.XSERVER.XORG-SERVER.FB]"
$ IF P4 .EQS. "XF86MODES" THEN include_directory = include_directory + -
    ",""./VMS-SUPPORT/_BUILD/''build_profile'/INCLUDE"""
$ internal_common = xf86common_profile .AND. (P4 .NES. "XF86_INPUT_KEYBOARD") .AND. -
    (P4 .NES. "XF86_INPUT_MOUSE")
$ IF internal_common THEN GOSUB xf86common_native
$ IF internal_common .AND. -
    (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF (P4 .EQS. "FB") .OR. (P4 .EQS. "RENDER") .OR. -
    (P4 .EQS. "SHADOW") THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.FB],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "SHADOW" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MIEXT.SHADOW]"
$ IF P4 .EQS. "RANDR" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.RANDR]"
$ IF P4 .EQS. "DBE" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.DBE],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "RECORD" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.RECORD],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "XFIXES" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "COMPOSITE" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.COMPOSITE],[.XSERVER.XORG-SERVER.XFIXES]," + -
    "[.XSERVER.XORG-SERVER.DAMAGEEXT],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "DIX" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.DIX],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "DAMAGEEXT" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.DAMAGEEXT],[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "EXT" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XEXT]," + -
    "[.XSERVER.XORG-SERVER.MI]"
$ IF P4 .EQS. "XINPUT" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.XI],[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "OS" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.OS],[.XSERVER.XORG-SERVER.XEXT]," + -
    "[.LIB.LIBXFONT2.INCLUDE.X11.FONTS],[.LIB.LIBXAU],[.LIB.XTRANS]," + -
    "[.XSERVER.XORG-SERVER.PRESENT]"
$ IF P4 .EQS. "VFB" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.HW.VFB],[.XSERVER.XORG-SERVER.FB]," + -
    "[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.XI]"
$ IF P4 .EQS. "XKB" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.PROGRAMS.XSERVER.XKB]," + -
    "[.XSERVER.XORG-SERVER.XKB]," + -
    "[.XSERVER.XORG-SERVER.XEXT],[.XSERVER.XORG-SERVER.XI]"
$! Slash-bearing upstream includes require a Unix-style search directory.
$ IF P4 .EQS. "XKB" THEN include_directory = include_directory + -
    ",""./VMS-SUPPORT/_BUILD/''build_profile'/INCLUDE/X11"""
$ IF P4 .EQS. "ROOTLESS" THEN include_directory = include_directory + -
    ",[.XSERVER.XORG-SERVER.MIEXT.ROOTLESS],[.XSERVER.XORG-SERVER.FB]," + -
    "[.XSERVER.XORG-SERVER.XEXT]"
$ IF P4 .EQS. "FONT_FONTFILE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.FONTFILE]"
$ IF P4 .EQS. "FONT_BITMAP" THEN -
    include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.LIB.LIBXFONT.INCLUDE.X11.FONTS],[.LIB.LIBXFONT.SRC.BITMAP]"
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11]XFUNCPROTO.H")
$! Upstream VMware config.h includes the public server SDK before any source.
$ IF P4 .EQS. "XF86_VIDEO_VMWARE" THEN -
    first_include = "/FIRST_INCLUDE=[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]XORG-SERVER.H"
$ IF P4 .EQS. "XF86_VIDEO_VMWARE" THEN include_directory = -
    "[.DRIVER.XF86-VIDEO-VMWARE.SRC]," + include_directory + -
    ",[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]," + -
    "[.XSERVER.XORG-SERVER.XFIXES],[.XSERVER.XORG-SERVER.RANDR]," + -
    "[.XSERVER.XORG-SERVER.INCLUDE],[.XSERVER.XORG-SERVER.XEXT]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.EXTENSIONS]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.BUS]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.RAMDAC]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.VGAHW]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.SHADOWFB]," + -
    "[.XSERVER.XORG-SERVER.MIEXT.DAMAGE]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.DDC]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.I2C]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.MODES]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.PARSER]," + -
    "[.XSERVER.XORG-SERVER.MI]," + -
    "[.XSERVER.XORG-SERVER.FB],[.XSERVER.XORG-SERVER.RENDER],[.XSERVER.XORG-SERVER.XEXT]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.PIXMAN]"
$ IF header .EQS. "" THEN GOTO done
$ headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ x11_first = headers
$ IF P4 .EQS. "XFONT2" THEN -
    x11_first = F$PARSE("[.LIB.LIBXFONT2.INCLUDE.X11]",,,"DEVICE") + -
    F$PARSE("[.LIB.LIBXFONT2.INCLUDE.X11]",,,"DIRECTORY")
$ IF P4 .EQS. "XFONT2" THEN DEFINE/USER_MODE/TABLE=LNM$PROCESS X11 'x11_first','headers'
$ IF P4 .NES. "XFONT2" THEN DEFINE/USER_MODE/TABLE=LNM$PROCESS X11 'headers'
$ scoped = 1
$ IF F$TRNLNM("X11","LNM$FILE_DEV") .NES. x11_first THEN GOTO done
$ IF (P4 .EQS. "FONT_FREETYPE") .OR. (P4 .EQS. "XFONT2") THEN GOTO freetype_headers
$ GOTO compile
$freetype_headers:
$ inherited_freetype = F$TRNLNM("FREETYPE")
$ inherited_freetype_table = F$TRNLNM("FREETYPE",,,,,"TABLE_NAME")
$ IF F$TRNLNM("FREETYPE","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.FREETYPE2.FREETYPE]FREETYPE.H")
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
$pixman_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.PIXMAN]PIXMAN.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.PIXMAN],[.EXTRAS.PIXMAN.PIXMAN]," + -
    """./extras/pixman/pixman"""
$ GOTO compile
$xcvt_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.LIBXCVT]LIBXCVT.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]," + -
    """./VMS-SUPPORT/_BUILD/''build_profile'/INCLUDE"""
$ GOTO compile
$pciaccess_headers:
$ IF P4 .EQS. "PCIACCESS_NATIVE" THEN GOSUB pciaccess_native_headers
$ IF rtl_headers .EQS. "" THEN GOTO done
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]PCIACCESS.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT.LIB.PCIACCESS],[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE],[.LIB.LIBPCIACCESS.SRC]"
$ pciaccess_config = F$SEARCH("[.VMS-SUPPORT.LIB.PCIACCESS]CONFIG.H")
$ IF pciaccess_config .EQS. "" THEN GOTO done
$ first_include = "/FIRST_INCLUDE=" + pciaccess_config
$ GOTO compile
$zlib_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]ZLIB.H") .EQS. "" THEN GOTO done
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]ZCONF.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE],[.EXTRAS.ZLIB]"
$ GOTO compile
$regex_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]REGEX.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE],[.VMS-SUPPORT._BUILD.''build_profile'.LIB.REGEX],[.EXTRAS.REGEX]"
$ GOTO compile
$fontenc_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS]FONTENC.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE.X11.FONTS],[.LIB.LIBFONTENC.SRC]," + -
    """./VMS-SUPPORT/_BUILD/''build_profile'/INCLUDE"""
$ GOTO compile
$compile:
$ IF build_profile .EQS. "P64_XORG" THEN GOSUB xorg_profile
$ IF rtl_headers .EQS. "" THEN GOTO done
$! Each modular source directory contains its own private headers.
$! Keep staged public/configured headers earlier in the explicit search list.
$ IF (P4 .NES. "GTF") .AND. (P4 .NES. "RTL") THEN -
    include_directory = include_directory + "," + -
    F$PARSE(P1,,,"DEVICE") + F$PARSE(P1,,,"DIRECTORY")
$ IF (P4 .EQS. "EXT") .OR. (P4 .EQS. "RAMDAC") THEN GOSUB mi_context
$! Recipes scope reviewed diagnostics; unexpected warnings remain fatal.
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
$xorg_profile:
$ IF F$LOCATE("OS-SUPPORT.OPENVMS",F$EDIT(P1,"UPCASE")) .LT. F$LENGTH(P1) THEN GOSUB os_native_headers
$ xorg_profiles = "|DAMAGE|MI|FB|RENDER|DIX|SHADOW|SYNC|RANDR|DBE|RECORD|XFIXES|COMPOSITE|DAMAGEEXT|EXT|XINPUT|OS|VFB|XKB|ROOTLESS|"
$ xorg_profiles = xorg_profiles + "XF86CONFIG|SHADOWFB|VGAHW|RAMDAC|COMMON|BUS|MISC|XORGXKB|LOADER|XF86MODES|DDC|I2C|STUB|CONFIG|XF86_VIDEO_VMWARE|"
$ xorg_profiles = xorg_profiles + "XF86_INPUT_KEYBOARD|XF86_INPUT_MOUSE|"
$ IF F$LOCATE("|" + P4 + "|",xorg_profiles) .EQ. F$LENGTH(xorg_profiles) THEN RETURN
$ xorg_include = "[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]SERVER_PROFILE.H"
$ xorg_external = (P4 .EQS. "XF86_VIDEO_VMWARE") .OR. -
    (P4 .EQS. "XF86_INPUT_KEYBOARD") .OR. (P4 .EQS. "XF86_INPUT_MOUSE")
$ xorg_input = (P4 .EQS. "COMMON") .AND. -
    (F$EDIT(F$PARSE(P1,,,"NAME"),"UPCASE") .EQS. "OPENVMS_INPUT")
$ IF xorg_input THEN include_directory = include_directory + -
    ",[.DRIVER.XF86-INPUT-KEYBOARD.SRC],[.DRIVER.XF86-INPUT-MOUSE.SRC]," + -
    "[.DRIVER.XF86-INPUT-MOUSE.INCLUDE]"
$ IF xorg_external .OR. xorg_input THEN -
    xorg_include = "[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]DRIVER_PROFILE.H"
$ IF xorg_external .OR. xorg_input THEN -
    first_include = "/FIRST_INCLUDE=[.VMS-SUPPORT._BUILD.''build_profile'.INCLUDE]XORG-SERVER.H"
$ xorg_include = F$SEARCH(xorg_include)
$ IF xorg_include .EQS. "" THEN rtl_headers = ""
$ IF rtl_headers .EQS. "" THEN RETURN
$ IF (P4 .EQS. "LOADER") .AND. (F$EDIT(F$PARSE(P1,,,"NAME"),"UPCASE") .EQS. "LOADER") -
    THEN xorg_include = xorg_include + "," + F$SEARCH("[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.OPENVMS]OPENVMS_LOADER.H")
$ IF (P4 .EQS. "OS") .AND. (F$EDIT(F$PARSE(P1,,,"NAME"),"UPCASE") .EQS. "OSPOLL") -
    THEN xorg_include = xorg_include + "," + F$SEARCH("[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.OPENVMS]OPENVMS_POLL.H")
$ include_directory = "[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]," + -
    "[.XSERVER.XORG-SERVER.HW.XFREE86.OS-SUPPORT.OPENVMS]," + include_directory
$ IF first_include .EQS. "/NOFIRST_INCLUDE" THEN GOTO xorg_first_only
$ xorg_old = F$EXTRACT(15,F$LENGTH(first_include)-15,first_include)
$ IF F$EXTRACT(0,1,xorg_old) .EQS. "(" THEN xorg_old = F$EXTRACT(1,F$LENGTH(xorg_old)-2,xorg_old)
$ first_include = "/FIRST_INCLUDE=(" + xorg_include + "," + xorg_old + ")"
$ RETURN
$xorg_first_only:
$ first_include = "/FIRST_INCLUDE=(" + xorg_include + ")"
$ RETURN
$xf86common_native:
$ xf86common_header = F$SEARCH("[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.COMMON]VMS_COMMON_CONFIG.H")
$ IF xf86common_header .NES. "" THEN first_include = "/FIRST_INCLUDE=" + xf86common_header
$ RETURN
$xkbfile_native:
$ xkbfile_native_header = F$SEARCH("[.VMS-SUPPORT.LIB.XKBFILE]VMS_XKBFILE_NATIVE.H")
$ IF xkbfile_native_header .NES. "" THEN first_include = "/FIRST_INCLUDE=" + xkbfile_native_header
$ RETURN
$os_native_headers:
$ starlet_headers = F$SEARCH("SYS$LIBRARY:SYS$STARLET_C.TLB")
$ IF starlet_headers .EQS. "" THEN rtl_headers = ""
$ IF (starlet_headers .NES. "") .AND. -
    (F$LOCATE(starlet_headers,rtl_headers) .EQ. F$LENGTH(rtl_headers)) -
    THEN rtl_headers = rtl_headers + "/LIBRARY+" + starlet_headers
$ RETURN
$pciaccess_native_headers:
$ GOSUB os_native_headers
$ IF rtl_headers .EQS. "" THEN RETURN
$ executive_headers = F$SEARCH("SYS$LIBRARY:SYS$LIB_C.TLB")
$ IF executive_headers .EQS. "" THEN rtl_headers = ""
$ IF executive_headers .EQS. "" THEN RETURN
$ rtl_headers = rtl_headers + "/LIBRARY+" + executive_headers
$ RETURN
$mi_context:
$! Match upstream relative includes in Xext and RAMDAC wrappers.
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
