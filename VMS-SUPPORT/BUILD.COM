$! Original component build entry point. No system installation or driver actions.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ step = "arguments"
$ target = F$EDIT(P1,"UPCASE,TRIM")
$ IF target .EQS. "" THEN target = "ALL"
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. (P5 .NES. "") .OR. -
     (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
$ IF (target .NES. "ALL") .AND. (target .NES. "HEADERS") .AND. -
     (target .NES. "XAU") .AND. (target .NES. "XDMCP_CODEC") .AND. -
     (target .NES. "XDMCP") THEN GOTO usage
$ IF F$EDIT(F$GETSYI("ARCH_NAME"),"TRIM") .NES. "x86_64" THEN GOTO unsupported
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ step = "locate source tree"
$ support_directory = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY")
$ SET DEFAULT 'support_directory'
$ SET DEFAULT [-]
$ IF F$SEARCH("[.INCLUDE]XFUNCPROTO.H") .EQS. "" THEN GOTO missing_source
$ step = "create output directories"
$ IF F$SEARCH("[.VMS-SUPPORT]_BUILD.DIR") .EQS. "" THEN CREATE/DIRECTORY [.VMS-SUPPORT._BUILD]
$ profile = "[.VMS-SUPPORT._BUILD.P64"
$ directory = profile + "]"
$ IF F$SEARCH("[.VMS-SUPPORT._BUILD]P64.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE]"
$ IF F$SEARCH(profile + "]INCLUDE.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".INCLUDE.X11]"
$ IF F$SEARCH(profile + ".INCLUDE]X11.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB]"
$ IF F$SEARCH(profile + "]LIB.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XAU]"
$ IF F$SEARCH(profile + ".LIB]XAU.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ directory = profile + ".LIB.XDMCP]"
$ IF F$SEARCH(profile + ".LIB]XDMCP.DIR") .EQS. "" THEN CREATE/DIRECTORY 'directory'
$ WRITE SYS$OUTPUT "X11 component build: " + target + ", native x86-64, pointer64"
$ step = "MMS " + target
$ MMS/DESCRIPTION=[.VMS-SUPPORT]DESCRIP.MMS 'target'
$ result = $STATUS
$ GOTO done
$usage:
$ WRITE SYS$ERROR "Usage: @BUILD [ALL|HEADERS|XAU|XDMCP_CODEC|XDMCP]"
$ GOTO done
$unsupported:
$ WRITE SYS$ERROR "This initial build profile supports OpenVMS x86-64 only."
$ GOTO done
$missing_source:
$ WRITE SYS$ERROR "Run this procedure from its original VMS-SUPPORT location in the source tree."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$ WRITE SYS$ERROR "X11 build failed at " + step + ": " + F$MESSAGE(result)
$done:
$ SET NOON
$ SET DEFAULT 'saved_default'
$ EXIT result
