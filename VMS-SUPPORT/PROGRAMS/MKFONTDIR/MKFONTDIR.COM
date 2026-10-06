$! Native launcher for programs/mkfontdir/mkfontdir.cpp.
$ SET NOON
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO failed
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ directory = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY")
$ SET DEFAULT 'directory'
$ executable = F$SEARCH("[-.MKFONTSCALE]MKFONTSCALE.EXE")
$ SET DEFAULT 'saved_default'
$ IF executable .EQS. "" THEN EXIT 2
$ mkfontscale = "$" + executable
$ DEFINE/USER_MODE DECC$EFS_CHARSET ENABLE
$ mkfontscale -b -s -l 'P1' 'P2' 'P3' 'P4' 'P5' 'P6' 'P7' 'P8'
$ EXIT $STATUS
$failed:
$ SET NOON
$ SET DEFAULT 'saved_default'
$ EXIT 2
