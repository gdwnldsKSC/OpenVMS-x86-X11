$! Foreground, display-only VMware test. Run from a serial console or SSH.
$! P1 is hexadecimal domain:bus:device.function; P2 must be SERIAL_CONSOLE.
$! P3 is an optional display number, default 0. No input or X listeners.
$ ON ERROR THEN GOTO failed
$ ON CONTROL_Y THEN GOTO interrupted
$ result = 2
$ old_default = F$ENVIRONMENT("DEFAULT")
$ default_changed = 0
$ IF P1 .EQS. "" .OR. P2 .NES. "SERIAL_CONSOLE" THEN GOTO usage
$ IF P4 .NES. "" .OR. P5 .NES. "" .OR. P6 .NES. "" THEN GOTO usage
$ IF P7 .NES. "" .OR. P8 .NES. "" THEN GOTO usage
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
$ SET DEFAULT 'repo'
$ default_changed = 1
$ image_dir = F$PARSE("[.VMS-SUPPORT._BUILD.P64_XORG.IMAGES]",,,"DEVICE") + -
    F$PARSE("[.VMS-SUPPORT._BUILD.P64_XORG.IMAGES]",,,"DIRECTORY")
$ module_dir = image_dir - "]" + ".MODULES]"
$ config_d = image_dir - "]" + ".CONFIG_D]"
$ server = F$SEARCH(image_dir + "XORG.EXE")
$ core = F$SEARCH(image_dir + "XORG_SHR.EXE")
$ vgahw = F$SEARCH(module_dir + "libvgahw.so")
$ shadowfb = F$SEARCH(module_dir + "libshadowfb.so")
$ driver = F$SEARCH(module_dir - "]" + ".DRIVERS]vmware_drv.so")
$ config = F$SEARCH(config_dir + "VIDEO.CONF")
$ xkbcomp = F$SEARCH("[.VMS-SUPPORT._BUILD.P64.LIB.X11.XKB]XKBCOMP.EXE")
$ IF server .EQS. "" .OR. core .EQS. "" THEN GOTO missing_files
$ IF vgahw .EQS. "" .OR. shadowfb .EQS. "" .OR. driver .EQS. "" THEN GOTO missing_files
$ IF config .EQS. "" .OR. xkbcomp .EQS. "" THEN GOTO missing_files
$ IF F$SEARCH(image_dir + "CONFIG_D.DIR") .EQS. "" THEN GOTO missing_files
$ IF F$TRNLNM("SYS$SCRATCH") .EQS. "" THEN GOTO missing_scratch
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
$ MCR 'server' "''display'" "-displayonly" "-retro" "-noreset" -
    "-modulepath" "/XORG_MODULES" "-config" "/XORG_CONFIG/VIDEO.CONF" -
    "-configdir" "/XORG_CONFIG_D" -
    "-xkbdir" "./VMS-SUPPORT/_BUILD/P64/LIB/X11/XKB" -
    "-logfile" "''logfile'" "-fp" "built-ins" "-s" "0"
$ result = $STATUS
$ GOTO done
$usage:
$ WRITE SYS$ERROR "Usage: @RUN_XORG domain:bus:device.function SERIAL_CONSOLE [display]"
$ WRITE SYS$ERROR "SERIAL_CONSOLE confirms serial OPA0 and exclusive ownership of the display."
$ GOTO done
$wrong_location:
$ WRITE SYS$ERROR "RUN_XORG.COM must remain in VMS-SUPPORT/PROGRAMS/XSERVER/HW/XFREE86/OPENVMS."
$ GOTO done
$missing_files:
$ WRITE SYS$ERROR "Build XORG and XKB_DATA before running the display-only server."
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
$ IF default_changed THEN SET DEFAULT 'old_default'
$ IF .NOT. $STATUS THEN result = $STATUS
$ EXIT result
