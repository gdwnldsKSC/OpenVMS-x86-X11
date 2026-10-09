! Native OpenVMS x86-64 build with 64-bit pointers.
OUT = [.VMS-SUPPORT._BUILD.P64
CFLAGS = /STANDARD=C99/POINTER_SIZE=64/NAMES=(UPPERCASE,SHORTENED)/PREFIX=ALL_ENTRIES/NODEBUG/WARNINGS=(ENABLE=LEVEL4)
RTL = SYS$LIBRARY:DECC$RTLDEF.TLB
! Override these Unix-style paths with MMS /MACRO for an installed Xlib.
X11_LIBDIR = lib/X11
X11_LOCALEDIR = nls
NLS_COMPOSE_INDEX = [.LIB.LIBX11.NLS]COMPOSE^.DIR.PRE
! Upstream uses separate compiled paths for XKB data and xkbcomp.
! RUN_SERVER sets the default to the runtime root; -xkbdir changes only data.
! Direct build-tree launches override XKB_BIN_DIRECTORY with XKB_CONFIG_ROOT.
! Use absolute Unix paths for a fixed install; XKM_OUTPUT_DIR ends in /.
XKB_CONFIG_ROOT = ./VMS-SUPPORT/_BUILD/P64/LIB/X11/XKB
XKB_BIN_DIRECTORY = ./LIB/X11/XKB
XKBCOMP_CONFIG_ROOT = $(XKB_CONFIG_ROOT)
XKB_DATA_DEST = $(OUT).LIB.X11.XKB]
XKB_RULES = xorg
XKB_MODEL = pc105
XKB_LAYOUT = us
XKM_OUTPUT_DIR = compiled/
! Optional comma-prefixed upstream keyboard policy, e.g. ,XKB_ALWAYS_USES_SOFT_REPEAT.
XKB_DDX_DEFINES =
! Optional comma-prefixed DDX definitions, e.g. ,DDXBEFORERESET or ,DDXOSINIT.
! Build each selected feature/policy profile in its own fresh source tree.
DIX_DDX_DEFINES =
XSERVER_DIX_DEFINES = $(DIX_DDX_DEFINES)
XSERVER_OS_DEFINES =
! Transport-only additions can select an independently supplied bind function.
XSERVER_TRANSPORT_DEFINES =
! XSERVER links these caller-built DDX inputs; defaults require explicit input.
XSERVER_DDX_INPUT =
XSERVER_IMAGE = $(OUT).PROGRAMS.XSERVER.HW.VFB]XSERVER.EXE
XSERVER_MAP = $(OUT).PROGRAMS.XSERVER.HW.VFB]XSERVER.MAP
! RUNTIME stages a payload only when explicitly requested; it is not part of ALL.
RUNTIME_DEST = $(OUT).RUNTIME]
