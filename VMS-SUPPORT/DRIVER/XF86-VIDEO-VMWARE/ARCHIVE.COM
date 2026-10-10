$! Archive the six ordinary driver objects, retaining compiler diagnostics.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF P1 .EQS. "" THEN GOTO done
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ modules = "BITS2PIXELS|OFFSCREEN_MANAGER|VMWARE|VMWARECURS|VMWAREXAA|VMWARECTRL"
$ index = 0
$check_objects:
$ module = F$ELEMENT(index,"|",modules)
$ IF module .EQS. "|" THEN GOTO create_library
$ IF F$SEARCH(P1 + module + ".OBJ") .EQS. "" THEN GOTO done
$ index = index + 1
$ GOTO check_objects
$create_library:
$ work_library = P1 + "VMWARE_DRV_BUILD.OLB"
$ LIBRARY/CREATE/OBJECT 'work_library'
$ work_library = F$SEARCH(work_library)
$ IF work_library .EQS. "" THEN GOTO done
$ index = 0
$insert_object:
$ module = F$ELEMENT(index,"|",modules)
$ IF module .EQS. "|" THEN GOTO publish
$ object = P1 + module + ".OBJ"
$ expected_warning = (module .EQS. "OFFSCREEN_MANAGER") .OR. (module .EQS. "VMWAREXAA")
$ IF expected_warning THEN ON WARNING THEN CONTINUE
$ LIBRARY/INSERT/OBJECT 'work_library' 'object'
$ result = $STATUS
$ ON WARNING THEN GOTO failed
$! COMCOD reports the retained MAYLOSEDATA3 compilation warning in these objects.
$ IF expected_warning .AND. (result .EQ. %X1086A030) THEN result = 1
$ IF .NOT. result THEN GOTO done
$ index = index + 1
$ GOTO insert_object
$publish:
$ listing = P1 + "VMWARE_DRV_LIBRARY.LIS"
$ LIBRARY/LIST='listing' 'work_library'
$ target = P1 + "VMWARE_DRV.OLB"
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
