! Upstream xkeyboard-config distributed-data layout and xorg rules aliases.
! Copies only declared data and the native compiler; no system startup changes.
XKB_DATA : $(OUT).PROGRAMS.XKBCOMP]XKBCOMP.EXE, -
    [.VMS-SUPPORT.PROGRAMS.XKBCOMP]DATA.LIST, -
    [.EXTRAS.XKEYBOARD-CONFIG.RULES]MAKEFILE.AM, -
    [.VMS-SUPPORT.PROGRAMS.XKBCOMP]STAGE_DATA.COM
	@[.VMS-SUPPORT.PROGRAMS.XKBCOMP]STAGE_DATA.COM "$(XKB_DATA_DEST)" "$(OUT).PROGRAMS.XKBCOMP]XKBCOMP.EXE"
