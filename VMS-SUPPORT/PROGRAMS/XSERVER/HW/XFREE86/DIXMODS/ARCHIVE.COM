$! Compose dixmods/libfb from the coherent generic library and Xorg objects.
$ SET NOON
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ IF (P4 .EQS. "") .OR. (P5 .EQS. "") THEN GOTO done
$ IF (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ base_library = F$SEARCH(P1)
$ IF base_library .EQS. "" THEN GOTO done
$ IF F$SEARCH(P3) .EQS. "" THEN GOTO done
$ IF F$SEARCH(P4) .EQS. "" THEN GOTO done
$ IF F$PARSE(P1,,,"DIRECTORY") .EQS. F$PARSE(P2,,,"DIRECTORY") THEN GOTO done
$! A fresh version prevents stale members from surviving repeated builds.
$ COPY/NOLOG 'base_library' 'P2'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ output_library = F$SEARCH(P2)
$ IF output_library .EQS. "" THEN GOTO missing_output
$ LIBRARY/REPLACE/OBJECT 'output_library' 'P3','P4'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ LIBRARY/LIST='P5' 'output_library'
$ result = $STATUS
$ GOTO done
$missing_output:
$ result = 2
$ GOTO done
$interrupted:
$ result = 44
$done:
$ EXIT result
