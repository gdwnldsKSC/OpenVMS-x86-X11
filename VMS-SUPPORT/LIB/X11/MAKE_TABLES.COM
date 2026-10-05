$! Run upstream makekeys with native standard-input/output redirection.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") .OR. (P3 .EQS. "") THEN GOTO done
$ IF (P4 .NES. "") .OR. (P5 .NES. "") .OR. (P6 .NES. "") .OR. -
     (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ target = F$PARSE(P1)
$ input = F$SEARCH(P2)
$ generator = F$SEARCH(P3)
$ IF (input .EQS. "") .OR. (generator .EQS. "") THEN GOTO done
$ temporary = F$PARSE(target,,,"DEVICE") + F$PARSE(target,,,"DIRECTORY") + -
    "KS_TABLES.TMP"
$ DEFINE/USER_MODE SYS$INPUT 'input'
$ DEFINE/USER_MODE SYS$OUTPUT 'temporary'
$ RUN 'generator'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ produced = F$SEARCH(temporary)
$ IF produced .EQS. "" THEN GOTO interrupted
$ RENAME/NOLOG 'produced' 'target'
$ result = $STATUS
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ EXIT result
