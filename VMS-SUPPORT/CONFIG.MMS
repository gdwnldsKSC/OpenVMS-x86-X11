! Native OpenVMS x86-64 build with 64-bit pointers.
OUT = [.VMS-SUPPORT._BUILD.P64
CFLAGS = /STANDARD=C99/POINTER_SIZE=64/NAMES=(UPPERCASE,SHORTENED)/PREFIX=ALL_ENTRIES/NODEBUG/WARNINGS=(ENABLE=LEVEL4)
RTL = SYS$LIBRARY:DECC$RTLDEF.TLB
! Override these Unix-style paths with MMS /MACRO for an installed Xlib.
X11_LIBDIR = lib/X11
X11_LOCALEDIR = nls
! Native staging preserves compose.dir bytes under a non-directory filename.
NLS_COMPOSE_INDEX = [.NLS]COMPOSE_DIR.IN
