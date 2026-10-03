$! Single-object action: source, output base, CFLAGS, header profile, optional CPPFLAGS.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ default_scoped = 0
$ scoped = 0
$ dps_scoped = 0
$ first_include = "/NOFIRST_INCLUDE"
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ profiles = "||ZLIB|FONTENC|EXPAT|LBXUTIL_IMAGE|LBXUTIL|FONT_UTIL|FNTSTUBS|FONT_BITMAP|FONT_FONTFILE|FONT_FC|FS|PSRES|REGEX|REGEX_MKH|ICE|SM|BDFTOPCF|ICEAUTH|MAKEPSRES|SHOWRGB|MAKEDEPEND|GTF|RTL|"
$ profiles = profiles + "DAMAGE|MI|FB|RENDER|DIX|SHADOW|RANDR|DBE|RECORD|LAYER|XFIXES|COMPOSITE|CW|DAMAGEEXT|EXT|XINPUT|OS|MFB|VFB|XKB|LBX|"
$ profiles = profiles + "CFB|XKBFILE|XKBUI|ROOTLESS|XTRAP|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ profiles = profiles + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
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
$ IF (P4 .EQS. "REGEX") .OR. (P4 .EQS. "REGEX_MKH") THEN GOTO regex_headers
$ inherited_x11 = F$TRNLNM("X11")
$ inherited_table = F$TRNLNM("X11",,,,,"TABLE_NAME")
$ IF F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]"
$ IF P4 .EQS. "XKB" THEN include_directory = -
    "[.PROGRAMS.XSERVER.XKB]," + include_directory
$ IF P4 .EQS. "EXPAT" THEN include_directory = include_directory + -
    ",[.EXTRAS.EXPAT.LIB],[.LIB.EXPAT]"
$ IF P4 .EQS. "LBXUTIL_IMAGE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.LBXUTIL.IMAGE],[.LIB.LBXUTIL.IMAGE]"
$ IF P4 .EQS. "LBXUTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.PROGRAMS.XSERVER.INCLUDE],[.LIB.LBXUTIL.LBX_ZLIB]"
$ IF P4 .EQS. "FONT_UTIL" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.FONT.INCLUDE]"
$ IF P4 .EQS. "FNTSTUBS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.STUBS]"
$ IF P4 .EQS. "FONT_FC" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.FC],[.LIB.XTRANS]"
$ IF P4 .EQS. "FS" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.FS],[.LIB.XTRANS]"
$ IF P4 .EQS. "ICE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.ICE],[.LIB.ICE],[.LIB.XTRANS]"
$ IF P4 .EQS. "SM" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.SM],[.LIB.SM]"
$ IF (P4 .EQS. "XKBFILE") .OR. (P4 .EQS. "XKBUI") THEN -
    include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.LIB.XKBFILE]"
$ IF P4 .EQS. "XKBUI" THEN include_directory = include_directory + -
    ",[.LIB.XKBUI]"
$ client_extensions = "|XEXT|XRENDER|XRANDR|XINERAMA|XI|XTST|XV|XRES|"
$ client_extensions = client_extensions + "XFIXESLIB|XDAMAGE|XCOMPOSITE|XCURSOR|"
$ IF F$LOCATE("|" + P4 + "|",client_extensions) .LT. F$LENGTH(client_extensions) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS],[.LIB.X11]"
$ IF P4 .EQS. "XEXT" THEN include_directory = include_directory + -
    ",[.LIB.XEXT]"
$ IF P4 .EQS. "XRENDER" THEN include_directory = include_directory + -
    ",[.LIB.XRENDER]"
$ IF P4 .EQS. "XRANDR" THEN include_directory = include_directory + -
    ",[.LIB.XRANDR]"
$ IF P4 .EQS. "XINERAMA" THEN include_directory = include_directory + -
    ",[.LIB.XINERAMA]"
$ IF P4 .EQS. "XI" THEN include_directory = include_directory + -
    ",[.LIB.XI]"
$ IF P4 .EQS. "XTST" THEN include_directory = include_directory + -
    ",[.LIB.XTST]"
$ IF P4 .EQS. "XV" THEN include_directory = include_directory + -
    ",[.LIB.XV]"
$ IF P4 .EQS. "XRES" THEN include_directory = include_directory + -
    ",[.LIB.XRES]"
$ IF P4 .EQS. "XFIXESLIB" THEN include_directory = include_directory + -
    ",[.LIB.XFIXES]"
$ IF P4 .EQS. "XDAMAGE" THEN include_directory = include_directory + -
    ",[.LIB.XDAMAGE]"
$ IF P4 .EQS. "XCOMPOSITE" THEN include_directory = include_directory + -
    ",[.LIB.XCOMPOSITE]"
$ IF P4 .EQS. "XCURSOR" THEN include_directory = include_directory + -
    ",[.LIB.XCURSOR]"
$ IF P4 .EQS. "ICEAUTH" THEN include_directory = include_directory + -
    ",[.PROGRAMS.ICEAUTH]"
$ IF P4 .EQS. "MAKEPSRES" THEN include_directory = include_directory + -
    ",[.PROGRAMS.MAKEPSRES]"
$ IF P4 .EQS. "SHOWRGB" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.INCLUDE]"
$ IF P4 .EQS. "MAKEDEPEND" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11],[.CONFIG.MAKEDEPEND],[.CONFIG.IMAKE]"
$ server_profiles = "|DAMAGE|MI|FB|RENDER|DIX|SHADOW|RANDR|DBE|RECORD|LAYER|XFIXES|COMPOSITE|CW|DAMAGEEXT|EXT|XINPUT|OS|MFB|VFB|XKB|LBX|CFB|ROOTLESS|XTRAP|"
$ IF F$LOCATE("|" + P4 + "|",server_profiles) .LT. F$LENGTH(server_profiles) -
    THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.EXTENSIONS]," + -
    "[.PROGRAMS.XSERVER.MIEXT.DAMAGE],[.PROGRAMS.XSERVER.INCLUDE]," + -
    "[.PROGRAMS.XSERVER.MI],[.PROGRAMS.XSERVER.RENDER]"
$ IF P4 .EQS. "MI" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.MFB],[.PROGRAMS.XSERVER.XEXT]," + -
    "[.PROGRAMS.XSERVER.XFIXES],[.PROGRAMS.XSERVER.DAMAGEEXT]"
$ IF (P4 .EQS. "FB") .OR. (P4 .EQS. "RENDER") .OR. -
    (P4 .EQS. "SHADOW") .OR. (P4 .EQS. "LAYER") THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.FB],[.PROGRAMS.XSERVER.XEXT]"
$ IF (P4 .EQS. "SHADOW") .OR. (P4 .EQS. "LAYER") THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.MIEXT.SHADOW]"
$ IF P4 .EQS. "LAYER" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.MIEXT.LAYER]"
$ IF P4 .EQS. "LAYER" THEN GOSUB layer_names
$ IF (P4 .EQS. "LAYER") .AND. (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "RANDR" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.RANDR]"
$ IF P4 .EQS. "DBE" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.DBE]"
$ IF P4 .EQS. "RECORD" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.RECORD],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "XFIXES" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.XFIXES],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "COMPOSITE" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.COMPOSITE],[.PROGRAMS.XSERVER.XFIXES]," + -
    "[.PROGRAMS.XSERVER.DAMAGEEXT],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "DIX" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.DIX],[.PROGRAMS.XSERVER.XEXT],[.PROGRAMS.XSERVER.LBX]"
$ IF P4 .EQS. "CW" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.MIEXT.CW],[.PROGRAMS.XSERVER.FB]," + -
    "[.PROGRAMS.XSERVER.COMPOSITE],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "DAMAGEEXT" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.DAMAGEEXT],[.PROGRAMS.XSERVER.XFIXES],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "EXT" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.XEXT],[.PROGRAMS.XSERVER.XEXT.EXTMOD],[.PROGRAMS.XSERVER.XPRINT]"
$ IF P4 .EQS. "XINPUT" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.XI],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "OS" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.OS],[.PROGRAMS.XSERVER.XEXT],[.PROGRAMS.XSERVER.LBX]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.XAU],[.LIB.XTRANS]"
$ IF P4 .EQS. "MFB" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.MFB]"
$ IF P4 .EQS. "VFB" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.HW.VFB],[.PROGRAMS.XSERVER.FB]," + -
    "[.PROGRAMS.XSERVER.MFB],[.PROGRAMS.XSERVER.XEXT],[.PROGRAMS.XSERVER.XI]"
$ IF P4 .EQS. "XKB" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XSERVER.XKB]," + -
    "[.PROGRAMS.XSERVER.XKB],[.PROGRAMS.XSERVER.XEXT.EXTMOD]," + -
    "[.PROGRAMS.XSERVER.XEXT],[.LIB.XKBFILE],[.LIB.X11],[.PROGRAMS.XSERVER.XI]"
$! Slash-bearing upstream includes require a Unix-style search directory.
$ IF P4 .EQS. "XKB" THEN include_directory = include_directory + -
    ",""./VMS-SUPPORT/_BUILD/P64/INCLUDE/X11"""
$ IF P4 .EQS. "LBX" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.LBX],[.PROGRAMS.XSERVER.XEXT],[.LIB.FONT.INCLUDE]"
$ IF P4 .EQS. "CFB" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.CFB],[.PROGRAMS.XSERVER.CFB24]," + -
    "[.PROGRAMS.XSERVER.MFB],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "CFB" THEN GOSUB cfb_native
$ IF (P4 .EQS. "CFB") .AND. (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "ROOTLESS" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.MIEXT.ROOTLESS],[.PROGRAMS.XSERVER.FB]," + -
    "[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "ROOTLESS" THEN GOSUB rootless_config
$ IF (P4 .EQS. "ROOTLESS") .AND. (first_include .EQS. "/NOFIRST_INCLUDE") THEN GOTO done
$ IF P4 .EQS. "XTRAP" THEN include_directory = include_directory + -
    ",[.PROGRAMS.XSERVER.XTRAP],[.PROGRAMS.XSERVER.XEXT]"
$ IF P4 .EQS. "FONT_FONTFILE" THEN include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.FONTFILE]"
$ IF (P4 .EQS. "FONT_BITMAP") .OR. (P4 .EQS. "BDFTOPCF") THEN -
    include_directory = include_directory + -
    ",[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]," + -
    "[.LIB.FONT.INCLUDE],[.LIB.FONT.BITMAP]"
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11]XFUNCPROTO.H")
$ IF header .EQS. "" THEN GOTO done
$ headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS X11 'headers'
$ scoped = 1
$ IF F$TRNLNM("X11","LNM$FILE_DEV") .NES. headers THEN GOTO done
$ IF P4 .EQS. "PSRES" THEN GOTO psres_headers
$ GOTO compile
$rtl_headers:
$! These utilities need only the explicit native RTL header library, not X11.
$ include_directory = """"""
$ GOTO compile
$psres_headers:
$ inherited_dps = F$TRNLNM("DPS")
$ inherited_dps_table = F$TRNLNM("DPS",,,,,"TABLE_NAME")
$ IF F$TRNLNM("DPS","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ header = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.DPS]PSRES.H")
$ IF header .EQS. "" THEN GOTO done
$ dps_headers = F$PARSE(header,,,"DEVICE") + F$PARSE(header,,,"DIRECTORY")
$ DEFINE/USER_MODE/TABLE=LNM$PROCESS DPS 'dps_headers'
$ dps_scoped = 1
$ IF F$TRNLNM("DPS","LNM$FILE_DEV") .NES. dps_headers THEN GOTO done
$ GOTO compile
$zlib_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]ZLIB.H") .EQS. "" THEN GOTO done
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]ZCONF.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE],[.EXTRAS.ZLIB]"
$ GOTO compile
$fontenc_headers:
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS]FONTENC.H") .EQS. "" THEN GOTO done
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.P64.INCLUDE.X11.FONTS],[.LIB.FONT.FONTFILE]"
$ GOTO compile
$regex_headers:
$ include_directory = "[.VMS-SUPPORT._BUILD.P64.INCLUDE]," + -
    "[.VMS-SUPPORT._BUILD.P64.LIB.REGEX],[.EXTRAS.REGEX]"
$ IF (P4 .EQS. "REGEX") .AND. -
    (F$SEARCH("[.VMS-SUPPORT._BUILD.P64.INCLUDE]REGEX.H") .EQS. "") THEN GOTO done
$compile:
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
$ IF .NOT. dps_scoped THEN GOTO done
$ IF F$TRNLNM("DPS") .NES. inherited_dps THEN result = 2
$ IF F$TRNLNM("DPS",,,,,"TABLE_NAME") .NES. inherited_dps_table THEN result = 2
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
$ IF dps_scoped .AND. (F$TRNLNM("DPS","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS DPS
$ IF scoped .AND. (F$TRNLNM("X11","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS X11
$ IF .NOT. result THEN WRITE SYS$OUTPUT "X11 component compile failed: " + P1
$ EXIT result
