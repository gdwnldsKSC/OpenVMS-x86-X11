$! Launch the installed DECwindows login application on an authenticated Xorg.
$! Run from a fresh dedicated SYSTEM control login. Invocation asserts that an
$! authorized client connected and a client without a cookie was rejected by
$! this display. This bridge does not establish or replace X server policy.
$! P1: display number. P2: exact native authority version. P3: fresh .LOG path.
$! No system logicals, startup files, cookie contents or installed files change.
$! Public interfaces: VSI DCL Dictionary SET DISPLAY/F$TRNLNM, and the
$! VSI DECwindows Motif Management Guide DECW$STARTLOGIN/LOGIN_XAUTHORITY.
$ operation_status = 2
$ changed = 0
$ launched = 0
$ created = 0
$ restore_failed = 0
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ IF P1 .EQS. "" .OR. P2 .EQS. "" .OR. P3 .EQS. "" THEN GOTO usage
$ IF P4 .NES. "" .OR. P5 .NES. "" .OR. P6 .NES. "" .OR. P7 .NES. "" .OR. P8 .NES. "" THEN GOTO usage
$ IF F$EDIT(F$GETJPI("","USERNAME"),"TRIM,UPCASE") .NES. "SYSTEM" THEN GOTO refused
$ IF .NOT. F$PRIVILEGE("CMEXEC,SYSNAM,SECURITY") THEN GOTO refused
$ IF F$TYPE(DIRECTORY) .NES. "" THEN GOTO refused
$ IF F$LENGTH(P1) .GT. 5 THEN GOTO refused
$ index = 0
$display_character:
$ IF index .GE. F$LENGTH(P1) THEN GOTO display_ready
$ IF F$LOCATE(F$EXTRACT(index,1,P1),"0123456789") .EQ. 10 THEN GOTO refused
$ index = index + 1
$ GOTO display_character
$display_ready:
$ IF F$INTEGER(P1) .GT. 59535 THEN GOTO refused
$ display = F$STRING(F$INTEGER(P1))
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ control_dir = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY")
$ checker = F$SEARCH(control_dir + "CHECK_AUTHORITY.COM")
$ IF checker .EQS. "" THEN GOTO refused
$! Check before substituting a parameter into another command procedure.
$ characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:.[]-;"
$ IF F$LENGTH(P2) .GT. 255 THEN GOTO refused
$ index = 0
$authority_character:
$ IF index .GE. F$LENGTH(P2) THEN GOTO snapshot_begin
$ IF F$LOCATE(F$EXTRACT(index,1,P2),characters) .EQ. F$LENGTH(characters) THEN GOTO refused
$ index = index + 1
$ GOTO authority_character
$snapshot_begin:
$ IF F$SEARCH("SYS$SYSTEM:DECW$STARTLOGIN.EXE") .EQS. "" THEN GOTO refused
$ IF F$TRNLNM("XORG_DECWINDOWS_DISPLAY",,,,,"ACCESS_MODE") .NES. "" THEN GOTO refused
$! Refuse unsupported user-mode definitions before running any image.
$! Do not run this concurrently with other logical changes in the same job.
$ names = "DECW$DISPLAY,DECW$DISPLAY,DECW$LOGIN_DISPLAY,DECW$LOGIN_DISPLAY,DECW$LOGIN_LOG,DECW$LOGIN_LOG,DECW$LOGIN_XAUTHORITY,DECW$LOGIN_XAUTHORITY"
$ scopes = "PROCESS,JOB,PROCESS,JOB,PROCESS,JOB,PROCESS,JOB"
$ attributes = "CONCEALED,CONFINE,CRELOG,NO_ALIAS,TERMINAL,TABLE"
$ i = 0
$snapshot:
$ IF i .EQ. 8 THEN GOTO snapshots_done
$ logical_name = F$ELEMENT(i,",",names)
$ table = "LNM$" + F$ELEMENT(i,",",scopes)
$ mode = F$TRNLNM(logical_name,table,,,,"ACCESS_MODE")
$ IF table .EQS. "LNM$JOB" .AND. mode .NES. "" THEN GOTO occupied_job
$ present_'i' = mode .NES. ""
$ saved_'i' = F$TRNLNM(logical_name,table)
$ IF mode .EQS. "" THEN GOTO next_snapshot
$ IF mode .NES. "SUPERVISOR" THEN GOTO complex_logical
$ IF F$TRNLNM(logical_name,table,,,,"MAX_INDEX") .NE. 0 THEN GOTO complex_logical
$ j = 0
$check_attribute:
$ attribute = F$ELEMENT(j,",",attributes)
$ IF attribute .EQS. "," THEN GOTO check_value
$ IF F$TRNLNM(logical_name,table,,,,attribute) .NES. "FALSE" THEN GOTO complex_logical
$ j = j + 1
$ GOTO check_attribute
$check_value:
$ value = saved_'i'
$ IF F$LOCATE(F$CHAR(34),value) .LT. F$LENGTH(value) THEN GOTO complex_logical
$ IF F$LOCATE(F$CHAR(39),value) .LT. F$LENGTH(value) THEN GOTO complex_logical
$ IF F$LOCATE(F$CHAR(10),value) .LT. F$LENGTH(value) THEN GOTO complex_logical
$ IF F$LOCATE(F$CHAR(13),value) .LT. F$LENGTH(value) THEN GOTO complex_logical
$next_snapshot:
$ i = i + 1
$ GOTO snapshot
$snapshots_done:
$ @'checker' "''P2'"
$ operation_status = $STATUS
$ IF .NOT. operation_status THEN GOTO restore
$ authority = F$SEARCH(P2)
$ display_authority = F$ELEMENT(0,";",authority)
$ IF authority .EQS. "" .OR. F$SEARCH(display_authority) .NES. authority THEN GOTO refused
$! Ordinary absolute native log paths only. Never reuse any existing version.
$ characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:.[]-"
$ IF F$LENGTH(P3) .GT. 255 THEN GOTO refused
$ index = 0
$log_character:
$ IF index .GE. F$LENGTH(P3) THEN GOTO log_ready
$ IF F$LOCATE(F$EXTRACT(index,1,P3),characters) .EQ. F$LENGTH(characters) THEN GOTO refused
$ index = index + 1
$ GOTO log_character
$log_ready:
$ IF F$LOCATE(":[",P3) .EQ. F$LENGTH(P3) .OR. F$LOCATE("::",P3) .LT. F$LENGTH(P3) THEN GOTO refused
$ IF F$LOCATE("..",P3) .LT. F$LENGTH(P3) THEN GOTO refused
$ dir_spec = F$PARSE(P3,,,"DIRECTORY")
$ IF F$EXTRACT(0,1,dir_spec) .NES. "[" THEN GOTO refused
$ IF F$EXTRACT(1,1,dir_spec) .EQS. "." .OR. F$EXTRACT(1,1,dir_spec) .EQS. "-" THEN GOTO refused
$ IF F$PARSE(P3,,,"NAME") .EQS. "" .OR. F$EDIT(F$PARSE(P3,,,"TYPE"),"UPCASE") .NES. ".LOG" THEN GOTO refused
$ login_log = P3
$ IF F$SEARCH(login_log + ";*") .NES. "" THEN GOTO refused
$! Check the output directory without creating or changing it.
$ device = F$PARSE(login_log,,,"DEVICE")
$ body = F$EXTRACT(1,F$LENGTH(dir_spec)-2,dir_spec)
$ index = 0
$ separator = -1
$log_parent_component:
$ IF index .GE. F$LENGTH(body) THEN GOTO log_parent_ready
$ IF F$EXTRACT(index,1,body) .EQS. "." THEN separator = index
$ index = index + 1
$ GOTO log_parent_component
$log_parent_ready:
$ parent = "000000"
$ leaf = body
$ IF separator .GE. 0 THEN parent = F$EXTRACT(0,separator,body)
$ IF separator .GE. 0 THEN leaf = F$EXTRACT(separator+1,F$LENGTH(body),body)
$ IF leaf .EQS. "" .OR. leaf .EQS. "000000" THEN GOTO refused
$ file = device + "[" + parent + "]" + leaf + ".DIR;1"
$ IF F$SEARCH(file) .EQS. "" THEN GOTO refused
$ IF F$FILE_ATTRIBUTES(file,"GRP") .NE. F$GETJPI("","GRP") THEN GOTO refused
$ IF F$FILE_ATTRIBUTES(file,"MBM") .NE. F$GETJPI("","MEM") THEN GOTO refused
$ protection = F$EDIT(F$FILE_ATTRIBUTES(file,"PRO"),"COLLAPSE,UPCASE")
$ rights = ""
$ index = 2
$log_rights:
$ IF index .EQ. 4 THEN GOTO log_acl
$ part = F$ELEMENT(index,",",protection)
$ label = "GROUP"
$ IF index .EQ. 3 THEN label = "WORLD"
$ IF F$ELEMENT(0,"=",part) .NES. label THEN GOTO refused
$ rights = rights + F$ELEMENT(1,"=",part)
$ index = index + 1
$ GOTO log_rights
$log_acl:
$ IF F$LOCATE("W",rights) .LT. F$LENGTH(rights) .OR. F$LOCATE("D",rights) .LT. F$LENGTH(rights) THEN GOTO refused
$ DIRECTORY/SECURITY/SELECT=NOACL/OUTPUT=NL: 'file'
$ IF $STATUS .NE. 1 THEN GOTO refused
$! SET DISPLAY cannot use an explicit version; verify the exact file again.
$ IF F$SEARCH(display_authority) .NES. authority THEN GOTO refused
$ IF F$SEARCH(login_log + ";*") .NES. "" THEN GOTO refused
$ created = 1
$ SET DISPLAY/CREATE/EXECUTIVE_MODE/TRANSPORT=TCPIP/NODE=127.0.0.1 -
    /SERVER='display'/SCREEN=0/XAUTHORITY='display_authority' XORG_DECWINDOWS_DISPLAY
$ operation_status = $STATUS
$ IF .NOT. operation_status THEN GOTO restore
$ display_device = F$TRNLNM("XORG_DECWINDOWS_DISPLAY","LNM$PROCESS")
$ IF display_device .EQS. "" THEN GOTO refused
$ IF .NOT. F$GETDVI(display_device,"EXISTS") THEN GOTO refused
$ i = 0
$define_next:
$ IF i .EQ. 8 THEN GOTO launch
$ logical_name = F$ELEMENT(i,",",names)
$ table = "LNM$" + F$ELEMENT(i,",",scopes)
$ value = display_device
$ IF i .GE. 4 THEN value = login_log
$ IF i .GE. 6 THEN value = authority
$ changed = i + 1
$ DEFINE/NOLOG/TABLE='table'/SUPERVISOR_MODE 'logical_name' "''value'"
$ operation_status = $STATUS
$ IF .NOT. operation_status THEN GOTO restore
$ i = i + 1
$ GOTO define_next
$launch:
$! A child may exist even if RUN reports failure; retain its display device.
$ launched = 1
$ RUN SYS$SYSTEM:DECW$STARTLOGIN.EXE
$ operation_status = $STATUS
$ GOTO restore
$failed:
$ operation_status = $STATUS
$ GOTO restore
$interrupted:
$ operation_status = 44
$ GOTO restore
$complex_logical:
$ WRITE SYS$ERROR "Unsupported existing logical attributes: " + table + ":" + logical_name
$ GOTO refused
$occupied_job:
$ WRITE SYS$ERROR "Target job logical already exists: " + logical_name + "; use a fresh dedicated control login."
$ GOTO refused
$usage:
$ WRITE SYS$ERROR "Usage: @START_DECWINDOWS display device:[directory]authority.type;version device:[directory]fresh.LOG"
$refused:
$ operation_status = 2
$ WRITE SYS$ERROR "DECwindows launch refused; use a fresh SYSTEM control login and protected authority/log directory."
$restore:
$ ON CONTROL_Y THEN CONTINUE
$ SET NOON
$ i = changed - 1
$restore_next:
$ IF i .LT. 0 THEN GOTO restore_done
$ logical_name = F$ELEMENT(i,",",names)
$ table = "LNM$" + F$ELEMENT(i,",",scopes)
$! Job logicals are shared with asynchronous children, not copied by SPAWN.
$ IF launched .AND. table .EQS. "LNM$JOB" THEN GOTO restored_one
$ DEASSIGN/TABLE='table'/SUPERVISOR_MODE 'logical_name'
$ restore_status = $STATUS
$ IF .NOT. restore_status THEN restore_failed = 1
$ IF .NOT. restore_status THEN operation_status = restore_status
$ IF .NOT. present_'i' THEN GOTO restored_one
$ value = saved_'i'
$ DEFINE/NOLOG/TABLE='table'/SUPERVISOR_MODE 'logical_name' "''value'"
$ restore_status = $STATUS
$ IF .NOT. restore_status THEN restore_failed = 1
$ IF .NOT. restore_status THEN operation_status = restore_status
$restored_one:
$ i = i - 1
$ GOTO restore_next
$restore_done:
$ IF launched THEN GOTO retain_display
$ IF .NOT. created THEN EXIT operation_status
$ IF F$TRNLNM("XORG_DECWINDOWS_DISPLAY","LNM$PROCESS") .EQS. "" THEN EXIT operation_status
$ SET DISPLAY/DELETE XORG_DECWINDOWS_DISPLAY
$ restore_status = $STATUS
$ IF .NOT. restore_status THEN operation_status = restore_status
$ EXIT operation_status
$retain_display:
$ WRITE SYS$OUTPUT "DECwindows launch returned; verify the greeter visually. Display retained: " + display_device
$ WRITE SYS$OUTPUT "LOGIN_LOG=" + login_log
$ WRITE SYS$OUTPUT "Job display/login logicals are retained for asynchronous children. Keep this control login."
$ WRITE SYS$OUTPUT "Only after all associated desktop/greeter children exit, use this control login:"
$ WRITE SYS$OUTPUT "DEASSIGN/JOB/SUPERVISOR_MODE DECW$DISPLAY"
$ WRITE SYS$OUTPUT "DEASSIGN/JOB/SUPERVISOR_MODE DECW$LOGIN_DISPLAY"
$ WRITE SYS$OUTPUT "DEASSIGN/JOB/SUPERVISOR_MODE DECW$LOGIN_LOG"
$ WRITE SYS$OUTPUT "DEASSIGN/JOB/SUPERVISOR_MODE DECW$LOGIN_XAUTHORITY"
$ WRITE SYS$OUTPUT "SET DISPLAY/DELETE XORG_DECWINDOWS_DISPLAY"
$ IF .NOT. restore_failed THEN WRITE SYS$OUTPUT "Parent process display and login logicals restored."
$ IF restore_failed THEN WRITE SYS$ERROR "Parent logical restoration failed; inspect this control job before continuing."
$ EXIT operation_status
