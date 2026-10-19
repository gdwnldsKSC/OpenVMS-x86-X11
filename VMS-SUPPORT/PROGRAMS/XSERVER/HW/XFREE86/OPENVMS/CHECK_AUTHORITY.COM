$! Read-only metadata checks for an explicitly supplied local X authority file.
$! No cookie contents are read, copied, generated, or printed.
$ ON WARNING THEN GOTO refused
$ ON CONTROL_Y THEN GOTO refused
$ IF P1 .EQS. "" .OR. P2 .NES. "" .OR. P3 .NES. "" .OR. P4 .NES. "" THEN GOTO usage
$ IF P5 .NES. "" .OR. P6 .NES. "" .OR. P7 .NES. "" .OR. P8 .NES. "" THEN GOTO usage
$ IF F$EDIT(F$GETJPI("","USERNAME"),"TRIM,UPCASE") .NES. "SYSTEM" THEN GOTO refused
$ IF .NOT. F$PRIVILEGE("SECURITY") THEN GOTO refused
$ IF F$TYPE(DIRECTORY) .NES. "" THEN GOTO refused
$! Restrict interpolation to ordinary absolute native filespecs, not DCL text.
$ characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:.[]-;"
$ index = 0
$ IF F$LENGTH(P1) .GT. 255 THEN GOTO refused
$check_character:
$ IF index .GE. F$LENGTH(P1) THEN GOTO check_version
$ IF F$LOCATE(F$EXTRACT(index,1,P1),characters) .EQ. F$LENGTH(characters) THEN GOTO refused
$ index = index + 1
$ GOTO check_character
$check_version:
$ authority_base = F$ELEMENT(0,";",P1)
$ version = F$ELEMENT(1,";",P1)
$ IF version .EQS. ";" .OR. version .EQS. "" THEN GOTO refused
$ IF F$ELEMENT(2,";",P1) .NES. ";" THEN GOTO refused
$ IF F$LENGTH(version) .GT. 5 THEN GOTO refused
$ index = 0
$version_character:
$ IF index .GE. F$LENGTH(version) THEN GOTO check_native
$ IF F$LOCATE(F$EXTRACT(index,1,version),"0123456789") .EQ. 10 THEN GOTO refused
$ index = index + 1
$ GOTO version_character
$check_native:
$ IF F$INTEGER(version) .LT. 1 .OR. F$INTEGER(version) .GT. 32767 THEN GOTO refused
$ IF F$LOCATE(":[",authority_base) .EQ. F$LENGTH(authority_base) THEN GOTO refused
$ IF F$LOCATE("::",authority_base) .LT. F$LENGTH(authority_base) THEN GOTO refused
$ IF F$LOCATE("..",authority_base) .LT. F$LENGTH(authority_base) THEN GOTO refused
$ dir_spec = F$PARSE(authority_base,,,"DIRECTORY")
$ IF F$EXTRACT(0,1,dir_spec) .NES. "[" THEN GOTO refused
$ IF F$EXTRACT(1,1,dir_spec) .EQS. "." .OR. F$EXTRACT(1,1,dir_spec) .EQS. "-" THEN GOTO refused
$ IF F$PARSE(authority_base,,,"NAME") .EQS. "" THEN GOTO refused
$ authority = F$SEARCH(P1)
$ IF authority .EQS. "" THEN GOTO refused
$ IF F$SEARCH(authority_base) .NES. authority THEN GOTO refused
$ IF F$FILE_ATTRIBUTES(authority,"DIRECTORY") THEN GOTO refused
$ IF F$FILE_ATTRIBUTES(authority,"EOF") .LT. 1 THEN GOTO refused
$ file = authority
$ strict = 1
$ GOSUB protected
$! The immediate directory must not allow another user to replace the file.
$ device = F$PARSE(authority,,,"DEVICE")
$ dir_spec = F$PARSE(authority,,,"DIRECTORY")
$ body = F$EXTRACT(1,F$LENGTH(dir_spec)-2,dir_spec)
$ index = 0
$ separator = -1
$parent_component:
$ IF index .GE. F$LENGTH(body) THEN GOTO parent_ready
$ IF F$EXTRACT(index,1,body) .EQS. "." THEN separator = index
$ index = index + 1
$ GOTO parent_component
$parent_ready:
$ parent = "000000"
$ leaf = body
$ IF separator .GE. 0 THEN parent = F$EXTRACT(0,separator,body)
$ IF separator .GE. 0 THEN leaf = F$EXTRACT(separator+1,F$LENGTH(body),body)
$ IF leaf .EQS. "" .OR. leaf .EQS. "000000" THEN GOTO refused
$ file = device + "[" + parent + "]" + leaf + ".DIR;1"
$ strict = 0
$ GOSUB protected
$ EXIT 1
$protected:
$ IF F$SEARCH(file) .EQS. "" THEN GOTO refused
$ IF F$FILE_ATTRIBUTES(file,"GRP") .NE. F$GETJPI("","GRP") THEN GOTO refused
$ IF F$FILE_ATTRIBUTES(file,"MBM") .NE. F$GETJPI("","MEM") THEN GOTO refused
$ protection = F$EDIT(F$FILE_ATTRIBUTES(file,"PRO"),"COLLAPSE,UPCASE")
$ part = F$ELEMENT(2,",",protection)
$ IF F$ELEMENT(0,"=",part) .NES. "GROUP" THEN GOTO refused
$ group_rights = F$ELEMENT(1,"=",part)
$ IF part .EQS. "GROUP" THEN group_rights = ""
$ part = F$ELEMENT(3,",",protection)
$ IF F$ELEMENT(0,"=",part) .NES. "WORLD" THEN GOTO refused
$ world_rights = F$ELEMENT(1,"=",part)
$ IF part .EQS. "WORLD" THEN world_rights = ""
$ IF strict .AND. (group_rights .NES. "" .OR. world_rights .NES. "") THEN GOTO refused
$ rights = group_rights + world_rights
$ IF F$LOCATE("W",rights) .LT. F$LENGTH(rights) THEN GOTO refused
$ IF F$LOCATE("D",rights) .LT. F$LENGTH(rights) THEN GOTO refused
$ DIRECTORY/SECURITY/SELECT=NOACL/OUTPUT=NL: 'file'
$ IF $STATUS .NE. 1 THEN GOTO refused
$ RETURN
$usage:
$ WRITE SYS$ERROR "Usage: @CHECK_AUTHORITY device:[directory]file.type;positive-version"
$ EXIT 2
$refused:
$ WRITE SYS$ERROR "Authority refused: use a nonempty current-version file owned by SYSTEM, with no group/world access or ACL."
$ WRITE SYS$ERROR "Its immediate directory must be SYSTEM-owned, without group/world write/delete or ACL; SECURITY must already be enabled."
$ EXIT 2
