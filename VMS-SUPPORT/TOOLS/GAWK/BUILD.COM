$! Run GNU Gawk's native MMS description in a generated-files directory.
$ SET NOON
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") THEN GOTO done
$ SET DEFAULT 'P1'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ source = F$ENVIRONMENT("DEFAULT")
$ SET DEFAULT 'saved_default'
$ CREATE/DIRECTORY 'P2'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ SET DEFAULT 'P2'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ output = F$ENVIRONMENT("DEFAULT")
$ source_dir = source
$ output_dir = output
$ masks = "*.C|*.H|CONFIGURE.|CONFIGH.IN|COPYING."
$ GOSUB copy_files
$ IF .NOT. result THEN GOTO done
$ groups = "VMS|SUPPORT|SUPPORT.MALLOC|MISSING_D"
$ group_index = 0
$group_loop:
$ group = F$ELEMENT(group_index,"|",groups)
$ IF group .EQS. "|" THEN GOTO build
$ source_dir = source - "]" + "." + group + "]"
$ output_dir = output - "]" + "." + group + "]"
$ CREATE/DIRECTORY 'output_dir'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ masks = "*.C|*.H"
$ IF group .EQS. "VMS" THEN masks = "*.C|*.H|*.COM|*.MMS|*.VMS|*.CLD"
$ GOSUB copy_files
$ IF .NOT. result THEN GOTO done
$ group_index = group_index + 1
$ GOTO group_loop
$build:
$ __x86_64__ = "TRUE"
$ flags = P3
$ IF flags .EQS. "" THEN flags = "/NOPOINTER_SIZE"
$ MMS/DESCRIPTION=[.VMS]DESCRIP.MMS/MACRO=("CCFLAGS=''flags'") gawk
$ result = $STATUS
$ GOTO done
$copy_files:
$ mask_index = 0
$mask_loop:
$ mask = F$ELEMENT(mask_index,"|",masks)
$ IF mask .EQS. "|" THEN RETURN
$ files = source_dir + mask + ";0"
$ destination = output_dir + "*.*;0"
$ COPY/NOLOG 'files' 'destination'
$ result = $STATUS
$ IF .NOT. result THEN RETURN
$ mask_index = mask_index + 1
$ GOTO mask_loop
$interrupted:
$ result = %X1000002C
$done:
$ SET DEFAULT 'saved_default'
$ EXIT 'result'
