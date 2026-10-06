$! Build separately acquired GNU Bison 1.35; P1 source directory, P2 output directory.
$ SET NOON
$ ON CONTROL_Y THEN GOTO interrupted
$ saved_default = F$ENVIRONMENT("DEFAULT")
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ recipe = F$PARSE(procedure,,,"DEVICE") + F$PARSE(procedure,,,"DIRECTORY")
$ result = 2
$ file_open = 0
$ options_open = 0
$ IF (P1 .EQS. "") .OR. (P2 .EQS. "") THEN GOTO usage
$ IF (P3 .NES. "") .OR. (P4 .NES. "") .OR. (P5 .NES. "") .OR. -
     (P6 .NES. "") .OR. (P7 .NES. "") .OR. (P8 .NES. "") THEN GOTO usage
$ SET DEFAULT 'P1'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ source = F$ENVIRONMENT("DEFAULT")
$ src = source - "]" + ".SRC]"
$ lib = source - "]" + ".LIB]"
$ result = 2
$ IF F$SEARCH("CONFIGURE.") .EQS. "" THEN GOTO done
$ OPEN/READ input CONFIGURE.
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ file_open = 1
$version_loop:
$ READ/END=bad_version input line
$ IF F$EDIT(line,"TRIM") .EQS. "PACKAGE_VERSION='1.35'" THEN GOTO version_ok
$ GOTO version_loop
$bad_version:
$ result = 2
$ GOTO done
$version_ok:
$ CLOSE input
$ file_open = 0
$ SET DEFAULT 'saved_default'
$ CREATE/DIRECTORY 'P2'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ SET DEFAULT 'P2'
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ output = F$ENVIRONMENT("DEFAULT")
$ template = recipe + "CONFIG.H"
$ COPY/NOLOG 'template' CONFIG.H;0
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ OPEN/APPEND input CONFIG.H
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ file_open = 1
$ WRITE input "#define BISON_SIMPLE """,output,"BISON.SIMPLE"""
$ WRITE input "#define BISON_HAIRY """,output,"BISON.HAIRY"""
$ CLOSE input
$ file_open = 0
$ file = src + "BISON.SIMPLE"
$ COPY/NOLOG 'file' BISON.SIMPLE;0
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ file = src + "BISON.HAIRY"
$ COPY/NOLOG 'file' BISON.HAIRY;0
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ file = source + "COPYING."
$ COPY/NOLOG 'file' COPYING.;0
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ flags = "/STANDARD=C99/POINTER_SIZE=64/PREFIX=ALL_ENTRIES/NAMES=(UPPERCASE,SHORTENED)"
$ flags = flags + "/FIRST_INCLUDE=CONFIG.H/NESTED_INCLUDE_DIRECTORY=NONE"
$ flags = flags + "/INCLUDE_DIRECTORY=([]," + src + "," + lib + ")"
$ flags = flags + "/DEFINE=(HAVE_CONFIG_H,_XOPEN_SOURCE=600,_XOPEN_SOURCE_EXTENDED=1)"
$ flags = flags + "/WARNINGS=ENABLE=LEVEL4"
$ src_units = "LR0,CLOSURE,COMPLAIN,CONFLICTS,DERIVES,FILES,GETARGS,GRAM,LALR,LEX,MAIN," + -
              "NULLABLE,OUTPUT,STATE,PRINT,READER,REDUCE,SYMTAB,WARSHALL,VCG,PRINT_GRAPH"
$ lib_units = "BASENAME,DIRNAME,GETOPT,GETOPT1,QUOTE,QUOTEARG,XMALLOC,XSTRDUP,XSTRNDUP," + -
              "OBSTACK,ERROR,MEMRCHR"
$ OPEN/WRITE options BISON.OPT
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ options_open = 1
$ group = "SRC"
$ units = src_units
$ directory = src
$ index = 0
$compile_loop:
$ unit = F$ELEMENT(index,",",units)
$ IF unit .EQS. "," THEN GOTO group_done
$ object = group + "_" + unit
$ file = directory + unit + ".C"
$ compile_flags = flags
$ IF (group .EQS. "SRC") .AND. (unit .EQS. "MAIN") THEN -
     compile_flags = flags + "/POINTER_SIZE=64=ARGV/MAIN=POSIX_EXIT"
$ CC 'compile_flags' /OBJECT='object'.OBJ /LIST='object'.LIS/SHOW=(INCLUDE,EXPANSION) -
     /MMS_DEPENDENCIES=(FILE='object'.MMS,SYSTEM_INCLUDE_FILES) 'file'
$ result = $STATUS
$ IF (result .AND. 7) .EQ. 2 THEN GOTO done
$ IF (result .AND. 7) .EQ. 4 THEN GOTO done
$ IF F$SEARCH(object+".OBJ") .EQS. "" THEN GOTO missing_object
$ WRITE options object,".OBJ"
$ index = index + 1
$ GOTO compile_loop
$group_done:
$ IF group .EQS. "LIB" THEN GOTO link
$ group = "LIB"
$ units = lib_units
$ directory = lib
$ index = 0
$ GOTO compile_loop
$missing_object:
$ result = 2
$ GOTO done
$link:
$ CLOSE options
$ options_open = 0
$ OPEN/WRITE input BISON_LINK.LOG
$ CLOSE input
$ DEFINE/USER_MODE/NOLOG SYS$OUTPUT BISON_LINK.LOG
$ DEFINE/USER_MODE/NOLOG SYS$ERROR BISON_LINK.LOG
$ LINK/NOTRACEBACK/MAP=BISON.MAP/FULL/EXE=BISON.EXE BISON.OPT/OPTIONS
$ result = $STATUS
$ IF (result .AND. 7) .EQ. 2 THEN GOTO done
$ IF (result .AND. 7) .EQ. 4 THEN GOTO done
$ OPEN/READ input BISON_LINK.LOG
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ file_open = 1
$link_check:
$ READ/END=linked input line
$ IF F$EXTRACT(0,1,line) .NES. "%" THEN GOTO link_check
$ IF F$LOCATE("%ILINK-W-COMPWARN,",line) .EQ. 0 THEN GOTO link_check
$ WRITE SYS$OUTPUT line
$ result = 2
$ GOTO done
$linked:
$ CLOSE input
$ file_open = 0
$ result = 2
$ image = F$SEARCH("BISON.EXE")
$ IF image .EQS. "" THEN GOTO done
$ WRITE SYS$OUTPUT "BISON :== $",image
$ result = 1
$ GOTO done
$usage:
$ WRITE SYS$OUTPUT "Supply the GNU Bison 1.35 source directory and an isolated output directory."
$ GOTO done
$interrupted:
$ result = 2
$done:
$ IF file_open THEN CLOSE input
$ IF options_open THEN CLOSE options
$ SET DEFAULT 'saved_default'
$ EXIT result
