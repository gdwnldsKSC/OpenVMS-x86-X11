$! Generate one upstream core XCB protocol with native Python 3.
$ SET NOON
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ result = 2
$ module = F$EDIT(P1,"LOWERCASE")
$ IF (module .NES. "xproto") .AND. (module .NES. "bigreq") .AND. -
     (module .NES. "xc_misc") .AND. (module .NES. "shape") THEN GOTO done
$ IF (P2 .NES. "") .OR. (P3 .NES. "") .OR. (P4 .NES. "") .OR. -
     (P5 .NES. "") .OR. (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO done
$ image = F$SEARCH("PYTHON$ROOT:[BIN]PYTHON.EXE")
$ IF image .EQS. "" THEN GOTO missing
$ native_python = "$" + image
$ device = F$PARSE(saved_default,,,"DEVICE")
$ directory = F$PARSE(saved_default,,,"DIRECTORY")
$ unix_root = "/" + F$EXTRACT(0,F$LENGTH(device)-1,device)
$ directory = F$EXTRACT(1,F$LENGTH(directory)-2,directory)
$ index = 0
$path_loop:
$ part = F$ELEMENT(index,".",directory)
$ IF part .EQS. "." THEN GOTO path_done
$ unix_root = unix_root + "/" + part
$ index = index + 1
$ GOTO path_loop
$path_done:
$ generator = unix_root + "/xcb/libxcb/src/c_client.py"
$ protocol_root = unix_root + "/xcb/xcb-proto"
$ protocol = protocol_root + "/src/" + module + ".xml"
$ SET DEFAULT [.VMS-SUPPORT._BUILD.P64.LIB.XCB]
$ DEFINE/USER_MODE PYTHONCASEOK 1
$ native_python "-B" 'generator' "-p" 'protocol_root' 'protocol'
$ result = $STATUS
$ IF .NOT. (result .AND. 1) THEN GOTO done
$ IF F$SEARCH(module + ".C") .EQS. "" THEN result = 2
$ IF F$SEARCH(module + ".H") .EQS. "" THEN result = 2
$ GOTO done
$missing:
$ WRITE SYS$OUTPUT "XCB requires native Python 3 and the PYTHON$ROOT logical name."
$ GOTO done
$interrupted:
$ result = 2
$ GOTO done
$failed:
$ result = $STATUS
$ IF result .AND. 1 THEN result = 2
$done:
$ SET NOON
$ SET DEFAULT 'saved_default'
$ EXIT result
