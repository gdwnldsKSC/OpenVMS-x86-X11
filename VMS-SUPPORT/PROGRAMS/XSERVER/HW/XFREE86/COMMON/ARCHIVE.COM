$! Archive the ordinary common objects, retaining compiler diagnostics.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF P1 .EQS. "" THEN GOTO done
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ modules = "XF86CONFIGURE|XF86BUS|XF86CONFIG|XF86CURSOR|XF86DGA|XF86DPMS|XF86EVENTS|XF86GLOBALS"
$ modules = modules + "|XF86AUTOCONFIG|XF86OPTION|XF86VIDMODE|XF86FBMAN|XF86CMAP|XF86HELPER|XF86PM|XF86MODE"
$ modules = modules + "|XORGHELPER|XF86PCIBUS|XF86FBBUS|XF86NOBUS|XF86XINPUT|XISB|XF86RANDR|XF86DEFMODESET"
$ modules = modules + "|XF86VGAARBITER|XF86XV|XF86XVMC|XF86INIT|XF86EXTENSIONS"
$ index = 0
$check_objects:
$ module = F$ELEMENT(index,"|",modules)
$ IF module .EQS. "|" THEN GOTO create_library
$ IF F$SEARCH(P1 + module + ".OBJ") .EQS. "" THEN GOTO done
$ index = index + 1
$ GOTO check_objects
$create_library:
$ work_library = P1 + "COMMON_BUILD.OLB"
$ LIBRARY/CREATE/OBJECT 'work_library'
$ work_library = F$SEARCH(work_library)
$ IF work_library .EQS. "" THEN GOTO done
$ index = 0
$insert_object:
$ module = F$ELEMENT(index,"|",modules)
$ IF module .EQS. "|" THEN GOTO publish
$ object = P1 + module + ".OBJ"
$ expected_warning = module .EQS. "XF86CONFIG"
$ IF expected_warning THEN ON WARNING THEN CONTINUE
$ LIBRARY/INSERT/OBJECT 'work_library' 'object'
$ result = $STATUS
$ ON WARNING THEN GOTO failed
$! COMCOD preserves the reviewed substring-count MAYLOSEDATA3 diagnostics.
$ IF expected_warning .AND. (result .EQ. %X1086A030) THEN result = 1
$ IF .NOT. result THEN GOTO done
$ index = index + 1
$ GOTO insert_object
$publish:
$ listing = P1 + "COMMON_LIBRARY.LIS"
$ LIBRARY/LIST='listing' 'work_library'
$ target = P1 + "COMMON.OLB"
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
