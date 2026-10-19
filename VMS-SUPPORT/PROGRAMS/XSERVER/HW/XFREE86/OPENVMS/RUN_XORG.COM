$! Foreground VMware test. Run from a serial console or SSH.
$! P1 is hexadecimal domain:bus:device.function; P2 must be SERIAL_CONSOLE.
$! P3 is an optional display number, default 0. No X listeners by default.
$! P4=I8042 opts into exclusive PS/2 keyboard/mouse input; default is video only.
$! P5=AUTHENTICATED, P6=exact protected authority, P7=public runtime directory
$! explicitly enable ordinary upstream TCP clients. No installation/startup changes.
$ result = 2
$ old_default = F$ENVIRONMENT("DEFAULT")
$ default_changed = 0
$ logicals_checked = 0
$ authenticated = 0
$ ON WARNING THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ IF P1 .EQS. "" .OR. P2 .NES. "SERIAL_CONSOLE" THEN GOTO usage
$ IF P4 .NES. "" .AND. P4 .NES. "I8042" THEN GOTO usage
$ IF P8 .NES. "" THEN GOTO usage
$ IF P5 .EQS. "" THEN GOTO display_only_mode
$ IF P5 .NES. "AUTHENTICATED" THEN GOTO usage
$ IF P6 .EQS. "" .OR. P7 .EQS. "" THEN GOTO usage
$ authenticated = 1
$ GOTO mode_checked
$display_only_mode:
$ IF P6 .NES. "" .OR. P7 .NES. "" THEN GOTO usage
$mode_checked:
$ bdf = F$EDIT(P1,"UPCASE")
$ characters = "0123456789ABCDEF:."
$ index = 0
$bdf_character:
$ IF index .GE. F$LENGTH(bdf) THEN GOTO display_number
$ IF F$LOCATE(F$EXTRACT(index,1,bdf),characters) .EQ. F$LENGTH(characters) THEN GOTO usage
$ index = index + 1
$ GOTO bdf_character
$display_number:
$ display = "0"
$ IF P3 .NES. "" THEN display = P3
$ index = 0
$display_character:
$ IF index .GE. F$LENGTH(display) THEN GOTO display_checked
$ IF F$LOCATE(F$EXTRACT(index,1,display),"0123456789") .EQ. 10 THEN GOTO usage
$ index = index + 1
$ GOTO display_character
$display_checked:
$ IF F$LENGTH(display) .GT. 5 THEN GOTO usage
$ IF F$INTEGER(display) .GT. 59535 THEN GOTO usage
$ display_number = F$STRING(F$INTEGER(display))
$ display = ":" + display_number
$ logfile = "./VMS-SUPPORT/_BUILD/P64_XORG/IMAGES/Xorg." + display_number + ".log"
$ procedure = F$ENVIRONMENT("PROCEDURE")
$ source_dir = F$PARSE(procedure,,,"DIRECTORY")
$ device = F$PARSE(procedure,,,"DEVICE")
$ repo = device + "[000000]"
$ IF F$EDIT(source_dir,"UPCASE") .EQS. -
    "[VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]" THEN GOTO directory_ready
$ suffix = ".VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]"
$ prefix_length = F$LENGTH(source_dir) - F$LENGTH(suffix)
$ IF prefix_length .LT. 1 THEN GOTO wrong_location
$ IF F$EDIT(F$EXTRACT(prefix_length,F$LENGTH(suffix),source_dir),"UPCASE") .NES. suffix THEN GOTO wrong_location
$ repo = device + F$EXTRACT(0,prefix_length,source_dir) + "]"
$directory_ready:
$ config_dir = device + source_dir
$ default_changed = 1
$ SET DEFAULT 'repo'
$ image_dir = F$PARSE("[.VMS-SUPPORT._BUILD.P64_XORG.IMAGES]",,,"DEVICE") + -
    F$PARSE("[.VMS-SUPPORT._BUILD.P64_XORG.IMAGES]",,,"DIRECTORY")
$ module_dir = image_dir - "]" + ".MODULES]"
$ config_d = image_dir - "]" + ".CONFIG_D]"
$ server = F$SEARCH(image_dir + "XORG.EXE")
$ core = F$SEARCH(image_dir + "XORG_SHR.EXE")
$ authority_tool = ""
$ IF authenticated THEN authority_tool = F$SEARCH(image_dir + "XORG_AUTH.EXE")
$ IF authenticated .AND. authority_tool .EQS. "" THEN GOTO missing_authority_tool
$ vgahw = F$SEARCH(module_dir + "libvgahw.so")
$ shadowfb = F$SEARCH(module_dir + "libshadowfb.so")
$ driver = F$SEARCH(module_dir - "]" + ".DRIVERS]vmware_drv.so")
$ config_name = "VIDEO.CONF"
$ input_mode = "DISABLED"
$ keyboard = ""
$ mouse = ""
$ IF P4 .EQS. "I8042" THEN config_name = "INPUT.CONF"
$ IF P4 .EQS. "I8042" THEN input_mode = "I8042_SERIAL_CONSOLE"
$ IF (P4 .EQS. "I8042") .OR. authenticated THEN keyboard = F$SEARCH(module_dir - "]" + ".INPUT]kbd_drv.so")
$ IF (P4 .EQS. "I8042") .OR. authenticated THEN mouse = F$SEARCH(module_dir - "]" + ".INPUT]mouse_drv.so")
$ IF ((P4 .EQS. "I8042") .OR. authenticated) .AND. (keyboard .EQS. "") THEN GOTO missing_files
$ IF ((P4 .EQS. "I8042") .OR. authenticated) .AND. (mouse .EQS. "") THEN GOTO missing_files
$ config = F$SEARCH(config_dir + config_name)
$ xkbcomp = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.LIB.X11.XKB]XKBCOMP.EXE")
$ IF server .EQS. "" .OR. core .EQS. "" THEN GOTO missing_files
$ IF vgahw .EQS. "" .OR. shadowfb .EQS. "" .OR. driver .EQS. "" THEN GOTO missing_files
$ IF config .EQS. "" .OR. xkbcomp .EQS. "" THEN GOTO missing_files
$ IF F$SEARCH(image_dir + "CONFIG_D.DIR") .EQS. "" THEN GOTO missing_files
$ IF F$TRNLNM("SYS$SCRATCH") .EQS. "" THEN GOTO missing_scratch
$! Refuse pending user-mode definitions before the metadata checker runs any
$! native image. Supervisor-mode caller definitions are never replaced.
$ logical_names = "DECC$EFS_CHARSET,DECC$EFS_CASE_PRESERVE,XORG_SHR,XORG_CORE,LIBVGAHW,LIBSHADOWFB,XORG_MODULES,XORG_CONFIG,XORG_CONFIG_D,PCIACCESS_OPENVMS_DEVICE,PCIACCESS_OPENVMS_EXCLUSIVE,XORG_OPENVMS_INPUT"
$ IF authenticated THEN logical_names = logical_names + ",XLOCALEDIR,XERRORDB,XCMSDB"
$ index = 0
$check_logicals:
$ logical_name = F$ELEMENT(index,",",logical_names)
$ IF logical_name .EQS. "," THEN GOTO logicals_ready
$ IF F$TRNLNM(logical_name,"LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN GOTO occupied_logical
$ index = index + 1
$ GOTO check_logicals
$logicals_ready:
$ logicals_checked = 1
$ font_path = "built-ins"
$ xkb_path = "./VMS-SUPPORT/_BUILD/P64/LIB/X11/XKB"
$ IF .NOT. authenticated THEN GOTO launch_ready
$ IF F$EDIT(F$GETJPI("","USERNAME"),"TRIM,UPCASE") .NES. "SYSTEM" THEN GOTO missing_privileges
$ IF .NOT. F$PRIVILEGE("SECURITY,CMKRNL,PFNMAP,SYSLCK") THEN GOTO missing_privileges
$ allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:-.[];"
$ index = 0
$authority_character:
$ IF index .GE. F$LENGTH(P6) THEN GOTO authority_checked
$ IF F$LOCATE(F$EXTRACT(index,1,P6),allowed) .EQ. F$LENGTH(allowed) THEN GOTO usage
$ index = index + 1
$ GOTO authority_character
$authority_checked:
$! Authority validation is read-only and must precede image logicals. The helper
$! requires an exact positive file version, restrictive protection and no ACL.
$ checker = F$SEARCH(config_dir + "CHECK_AUTHORITY.COM")
$ IF checker .EQS. "" THEN GOTO missing_files
$ @'checker' "''P6'"
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ result = 2
$ authority = F$SEARCH(P6)
$ IF authority .EQS. "" THEN GOTO missing_files
$! Accept ordinary absolute native runtime directories, not wildcard/search-list
$! expressions or text which could become DCL syntax when used below.
$ allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:-.[]"
$ index = 0
$runtime_character:
$ IF index .GE. F$LENGTH(P7) THEN GOTO runtime_parsed
$ IF F$LOCATE(F$EXTRACT(index,1,P7),allowed) .EQ. F$LENGTH(allowed) THEN GOTO usage
$ index = index + 1
$ GOTO runtime_character
$runtime_parsed:
$ IF F$LOCATE(":",P7) .EQ. F$LENGTH(P7) THEN GOTO usage
$ IF F$EXTRACT(F$LENGTH(P7)-1,1,P7) .NES. "]" THEN GOTO usage
$ IF F$PARSE(P7,,,"NODE","SYNTAX_ONLY") .NES. "" THEN GOTO usage
$ runtime_device = F$PARSE(P7,,,"DEVICE","SYNTAX_ONLY")
$ runtime_directory = F$PARSE(P7,,,"DIRECTORY","SYNTAX_ONLY")
$ IF F$EXTRACT(1,1,runtime_directory) .EQS. "." THEN GOTO usage
$ IF F$EXTRACT(1,1,runtime_directory) .EQS. "-" THEN GOTO usage
$ IF runtime_directory .EQS. "[000000]" THEN GOTO usage
$ runtime = runtime_device + runtime_directory
$ runtime_prefix = F$EXTRACT(0,F$LENGTH(runtime)-1,runtime)
$ runtime_unix = "/" + F$EXTRACT(0,F$LENGTH(runtime_device)-1,runtime_device)
$ directory = F$EXTRACT(1,F$LENGTH(runtime_directory)-2,runtime_directory)
$ index = 0
$runtime_unix_directory:
$ part = F$ELEMENT(index,".",directory)
$ IF part .EQS. "." THEN GOTO runtime_unix_ready
$ runtime_unix = runtime_unix + "/" + part
$ index = index + 1
$ GOTO runtime_unix_directory
$runtime_unix_ready:
$ sections = "MISC|75DPI|100DPI|CYRILLIC"
$ index = 0
$ font_path = ""
$runtime_font:
$ section = F$ELEMENT(index,"|",sections)
$ IF section .EQS. "|" THEN GOTO runtime_fonts_ready
$ IF F$SEARCH(runtime_prefix + ".FONTS." + section + "]FONTS.DIR") .EQS. "" THEN GOTO missing_runtime
$ IF font_path .NES. "" THEN font_path = font_path + ","
$ font_path = font_path + runtime_unix + "/FONTS/" + section
$ index = index + 1
$ GOTO runtime_font
$runtime_fonts_ready:
$ IF F$SEARCH(runtime_prefix + ".NLS]LOCALE.DIR") .EQS. "" THEN GOTO missing_runtime
$ IF F$SEARCH(runtime_prefix + ".NLS]LOCALE.ALIAS") .EQS. "" THEN GOTO missing_runtime
$ IF F$SEARCH(runtime_prefix + ".NLS]COMPOSE.DIR") .EQS. "" THEN GOTO missing_runtime
$ IF F$SEARCH(runtime_prefix + ".LIB.X11]XERRORDB.") .EQS. "" THEN GOTO missing_runtime
$ IF F$SEARCH(runtime_prefix + ".LIB.X11]XCMS.TXT") .EQS. "" THEN GOTO missing_runtime
$ IF F$SEARCH(runtime_prefix + ".LIB.X11.XKB]XKBCOMP.EXE") .EQS. "" THEN GOTO missing_runtime
$ IF F$SEARCH(runtime_prefix + ".LIB.X11.XKB.RULES]BASE.") .EQS. "" THEN GOTO missing_runtime
$ xkb_path = runtime_unix + "/LIB/X11/XKB"
$ locale_path = runtime_unix + "/NLS"
$ error_database = runtime_unix + "/LIB/X11/XErrorDB"
$ color_database = runtime_unix + "/LIB/X11/Xcms.txt"
$! A protected but malformed/empty authority database would let upstream fall
$! back to host authorization. Require a valid cookie for this display first.
$ MCR 'authority_tool' "CHECK" "''authority'" "''display_number'"
$ result = $STATUS
$ IF .NOT. result THEN GOTO done
$ result = 2
$! Keep the source-root default: XKB_BIN_DIRECTORY in this source-tree build
$! selects its built compiler; -xkbdir below selects the staged runtime data.
$launch_ready:
$ IF authenticated THEN WRITE SYS$OUTPUT "Authenticated Xorg uses ordinary upstream TCP listeners; no loopback restriction is applied."
$! User-mode definitions shadow existing supervisor entries without replacing
$! them; image/procedure rundown removes these temporary entries. Do not run
$! another image between these definitions and Xorg.
$ DEFINE/USER_MODE/PROCESS DECC$EFS_CHARSET ENABLE
$ DEFINE/USER_MODE/PROCESS DECC$EFS_CASE_PRESERVE ENABLE
$ DEFINE/USER_MODE/PROCESS XORG_SHR "''core'"
$ DEFINE/USER_MODE/PROCESS XORG_CORE "''core'"
$ DEFINE/USER_MODE/PROCESS LIBVGAHW "''vgahw'"
$ DEFINE/USER_MODE/PROCESS LIBSHADOWFB "''shadowfb'"
$ DEFINE/USER_MODE/PROCESS XORG_MODULES "''module_dir'"
$ DEFINE/USER_MODE/PROCESS XORG_CONFIG "''config_dir'"
$ DEFINE/USER_MODE/PROCESS XORG_CONFIG_D "''config_d'"
$ DEFINE/USER_MODE/PROCESS PCIACCESS_OPENVMS_DEVICE "''bdf'"
$ DEFINE/USER_MODE/PROCESS PCIACCESS_OPENVMS_EXCLUSIVE "1"
$ DEFINE/USER_MODE/PROCESS XORG_OPENVMS_INPUT "''input_mode'"
$ IF authenticated THEN GOTO authenticated_launch
$ MCR 'server' "''display'" "-displayonly" "-retro" "-noreset" -
    "-modulepath" "/XORG_MODULES" "-config" "/XORG_CONFIG/''config_name'" -
    "-configdir" "/XORG_CONFIG_D" -
    "-xkbdir" "./VMS-SUPPORT/_BUILD/P64/LIB/X11/XKB" -
    "-logfile" "''logfile'" "-fp" "built-ins" "-a" "1" "-t" "1" "-s" "0"
$ result = $STATUS
$ GOTO done
$authenticated_launch:
$ DEFINE/USER_MODE/PROCESS XLOCALEDIR "''locale_path'"
$ DEFINE/USER_MODE/PROCESS XERRORDB "''error_database'"
$ DEFINE/USER_MODE/PROCESS XCMSDB "''color_database'"
$ MCR 'server' "''display'" "-listen" "tcp" "-auth" "''authority'" "-noreset" -
    "-modulepath" "/XORG_MODULES" "-config" "/XORG_CONFIG/''config_name'" -
    "-configdir" "/XORG_CONFIG_D" "-xkbdir" "''xkb_path'" -
    "-logfile" "''logfile'" "-fp" "''font_path'" "-a" "1" "-t" "1" "-s" "0"
$ result = $STATUS
$ GOTO done
$usage:
$ WRITE SYS$ERROR "Usage: @RUN_XORG domain:bus:device.function SERIAL_CONSOLE [display] [I8042]"
$ WRITE SYS$ERROR "Optional client mode: append AUTHENTICATED exact-authority-file;version native-runtime-directory"
$ WRITE SYS$ERROR "SERIAL_CONSOLE confirms serial OPA0 and exclusive ownership of the display."
$ WRITE SYS$ERROR "I8042 additionally confirms exclusive ownership of the PS/2 keyboard and mouse."
$ GOTO done
$wrong_location:
$ WRITE SYS$ERROR "RUN_XORG.COM must remain in VMS-SUPPORT/PROGRAMS/XSERVER/HW/XFREE86/OPENVMS."
$ GOTO done
$missing_files:
$ WRITE SYS$ERROR "Build XORG and XKB_DATA, and keep the launch support files together."
$ GOTO done
$missing_runtime:
$ WRITE SYS$ERROR "Build RUNTIME and supply its complete native runtime directory."
$ GOTO done
$missing_authority_tool:
$ WRITE SYS$ERROR "Build XAU, then run BUILD_XORG_AUTH.COM before authenticated launch."
$ GOTO done
$missing_privileges:
$ WRITE SYS$ERROR "Authenticated hardware launch requires SYSTEM with SECURITY, CMKRNL, PFNMAP and SYSLCK already enabled."
$ GOTO done
$occupied_logical:
$ WRITE SYS$ERROR "Refusing a preexisting user-mode process logical: ",logical_name
$ GOTO done
$missing_scratch:
$ WRITE SYS$ERROR "SYS$SCRATCH must name a writable directory for compiled XKB keymaps."
$ GOTO done
$interrupted:
$ result = 44
$ GOTO done
$failed:
$ result = $STATUS
$done:
$ SET NOON
$ IF .NOT. logicals_checked THEN GOTO restore_default
$ index = 0
$cleanup_logicals:
$ logical_name = F$ELEMENT(index,",",logical_names)
$ IF logical_name .EQS. "," THEN GOTO restore_default
$ IF F$TRNLNM(logical_name,"LNM$PROCESS",0,"USER",,"ACCESS_MODE") .EQS. "USER" THEN -
    DEASSIGN/USER_MODE/PROCESS 'logical_name'
$ index = index + 1
$ GOTO cleanup_logicals
$restore_default:
$ IF default_changed THEN SET DEFAULT 'old_default'
$ IF .NOT. $STATUS THEN result = $STATUS
$ EXIT result
