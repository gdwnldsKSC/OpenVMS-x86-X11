$! Archive exactly the portable C members selected by upstream libmisc.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF P1 .EQS. "" THEN GOTO done
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ modules = "XF86_UTIL|DELAY|BUSMEMCPY|IODELAY|SLOWBCOPY"
$ index = 0
$check_objects:
$ module = F$ELEMENT(index,"|",modules)
$ IF module .EQS. "|" THEN GOTO create_library
$ IF F$SEARCH(P1 + module + ".OBJ") .EQS. "" THEN GOTO done
$ index = index + 1
$ GOTO check_objects
$create_library:
$ work_library = P1 + "MISC_BUILD.OLB"
$ LIBRARY/CREATE/OBJECT 'work_library'
$ work_library = F$SEARCH(work_library)
$ IF work_library .EQS. "" THEN GOTO done
$ index = 0
$insert_object:
$ module = F$ELEMENT(index,"|",modules)
$ IF module .EQS. "|" THEN GOTO publish
$ object = P1 + module + ".OBJ"
$ LIBRARY/INSERT/OBJECT 'work_library' 'object'
$ index = index + 1
$ GOTO insert_object
$publish:
$ listing = P1 + "MISC_LIBRARY.LIS"
$ LIBRARY/LIST='listing' 'work_library'
$ target = P1 + "MISC.OLB"
$ RENAME/NOLOG 'work_library' 'target'
$ result = $STATUS
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$ GOTO done
$interrupted:
$ result = 2
$done:
$ EXIT result
