$! Run the unchanged upstream generator on both upstream mode tables.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ awk_image = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.TOOLS.GAWK]GAWK.EXE")
$ IF P1 .NES. "" THEN awk_image = F$SEARCH(P1)
$ IF awk_image .EQS. "" THEN GOTO missing_awk
$ AWK = "$" + awk_image
$ source = "[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]"
$ base = "[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XSERVER.HW.XFREE86.COMMON]"
$ IF P2 .NES. "" THEN base = P2
$ generator = source + "MODELINE2C.AWK"
$ vesa = source + "VESAMODES."
$ extra = source + "EXTRAMODES."
$ modes = base + "MODELINES.DAT"
$ raw = base + "XF86DEFMODESET_RAW.C"
$ target = base + "XF86DEFMODESET.C"
$ IF F$SEARCH(generator) .EQS. "" THEN GOTO done
$ IF F$SEARCH(vesa) .EQS. "" THEN GOTO done
$ IF F$SEARCH(extra) .EQS. "" THEN GOTO done
$ COPY/NOLOG/CONCATENATE 'vesa','extra' 'modes'
$ DEFINE/USER_MODE/NOLOG SYS$OUTPUT 'raw'
$ AWK "-f" 'generator' 'modes'
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ result = 2
$ IF F$SEARCH(raw) .EQS. "" THEN GOTO done
$ CONVERT/FDL=SYS$INPUT 'raw' 'target'
RECORD
 FORMAT STREAM_LF
 CARRIAGE_CONTROL CARRIAGE_RETURN
$ result = $STATUS
$ IF F$SEARCH(target) .EQS. "" THEN result = 2
$ GOTO done
$missing_awk:
$ WRITE SYS$OUTPUT "COMMON requires the source-built GAWK target."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ EXIT result
