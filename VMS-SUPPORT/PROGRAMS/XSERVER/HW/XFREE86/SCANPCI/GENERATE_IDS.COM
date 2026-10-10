$! Run the unchanged upstream generator with both upstream PCI ID tables.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ input_scoped = 0
$ output_scoped = 0
$ IF (P1 .NES. "") .OR. (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ IF F$TYPE(PERL) .NES. "STRING" THEN GOTO missing_perl
$ IF F$TRNLNM("SYS$INPUT","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ IF F$TRNLNM("SYS$OUTPUT","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO done
$ source = "[.XSERVER.XORG-SERVER.HW.XFREE86.SCANPCI]"
$ base = "[.VMS-SUPPORT._BUILD.P64.PROGRAMS.XSERVER.HW.XFREE86.SCANPCI]"
$ generator = source + "PCIID2C.PL"
$ standard = source + "PCI.IDS"
$ extra = source + "EXTRAPCI.IDS"
$ info = "[.XSERVER.XORG-SERVER.HW.XFREE86.COMMON]XF86PCIINFO.H"
$ ids = base + "PCI_IDS.DAT"
$ raw = base + "XF86PCIIDS_RAW.H"
$ target = base + "XF86PCIIDS.H"
$ IF F$SEARCH(generator) .EQS. "" THEN GOTO done
$ IF F$SEARCH(standard) .EQS. "" THEN GOTO done
$ IF F$SEARCH(extra) .EQS. "" THEN GOTO done
$ IF F$SEARCH(info) .EQS. "" THEN GOTO done
$ COPY/NOLOG/CONCATENATE 'standard','extra' 'ids'
$ DEFINE/USER_MODE/NOLOG SYS$INPUT 'ids'
$ input_scoped = 1
$ DEFINE/USER_MODE/NOLOG SYS$OUTPUT 'raw'
$ output_scoped = 1
$ PERL 'generator' 'info'
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
$missing_perl:
$ WRITE SYS$OUTPUT "SCANPCI requires the installed Perl setup to define the PERL foreign command."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ IF input_scoped .AND. (F$TRNLNM("SYS$INPUT","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS SYS$INPUT
$ IF output_scoped .AND. (F$TRNLNM("SYS$OUTPUT","LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER") -
     THEN DEASSIGN/USER_MODE/TABLE=LNM$PROCESS SYS$OUTPUT
$ EXIT result
