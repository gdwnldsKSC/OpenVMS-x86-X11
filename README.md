# OpenVMS x86 X11 R6.8.2 Port Effort  
  
Currently, this is just the straight X11 R6.8.2 tree  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  
  
XAU. Produces `XAU.OLB` - all eight standard Xau modules; optional Kerberos support excluded.  
XDMCP. Produces `XDMCP.OLB` - all 38 standard Xdmcp modules; optional `HASXDMAUTH` wrapping excluded.  
ZLIB. Produces `Z.OLB` - all 14 modules of the bundled zlib 1.1.4.  
FONTENC. Produces `FONTENC.OLB` - `fontenc.c` and `encparse.c`, using `Z.OLB`; upstream reverse-map cleanup and parser-growth defects remain.  
EXPAT. Produces `EXPAT.OLB` - `xmlparse.c`, `xmltok.c`, and `xmlrole.c` from bundled Expat 1.95.6.  
LBXUTIL. Produces `LBXUTIL.OLB` - all nine standard LBX utility modules, using `Z.OLB`; consumers supply `Xalloc`/`Xfree`.  
XFONT. Produces `XFONT.OLB` - all 41 standard bitmap, fontfile, fc, and util modules, with gzip and TCP font-server transport; optional renderers and caches excluded.  
FNTSTUBS. Produces `FNTSTUBS.OLB` - all 17 unchanged modules from `lib/font/stubs`, for standalone font tools.  
FS. Produces `FS.OLB` - all 25 standard font-server client library modules, with TCP transport.  
PSRES. Produces `PSRES.OLB` - the complete upstream PostScript resource library; requires case-preserving Unix-style filenames (`DECC$EFS_CASE_PRESERVE` and `DECC$FILENAME_UNIX_REPORT` enabled); upstream cache defects remain.  
REGEX. Produces `REGEX.OLB` - all four upstream regular-expression modules by Henry Spencer, with native generation of their public and private headers.  
BDFTOPCF. Produces `BDFTOPCF.EXE` - the upstream BDF-to-PCF font converter, using `XFONT.OLB` and `Z.OLB`; upstream `-p8` defect remains.  
FSLSFONTS. Produces `FSLSFONTS.EXE` - the upstream font-server font listing tool, using `FS.OLB`.  
SHOWFONT. Produces `SHOWFONT.EXE` - the upstream font-server glyph and metrics viewer, using `FS.OLB`.  
XFSINFO. Produces `XFSINFO.EXE` - the upstream font-server information tool, using `FS.OLB`.  
  
## Notes  
  
$ @[.VMS-SUPPORT]BUILD HEADERS  
$ @[.VMS-SUPPORT]BUILD XAU  
$ @[.VMS-SUPPORT]BUILD XDMCP  
$ @[.VMS-SUPPORT]BUILD ZLIB  
$ @[.VMS-SUPPORT]BUILD FONTENC  
$ @[.VMS-SUPPORT]BUILD EXPAT  
$ @[.VMS-SUPPORT]BUILD LBXUTIL  
$ @[.VMS-SUPPORT]BUILD XFONT  
$ @[.VMS-SUPPORT]BUILD FNTSTUBS  
$ @[.VMS-SUPPORT]BUILD FS  
$ @[.VMS-SUPPORT]BUILD PSRES  
$ @[.VMS-SUPPORT]BUILD REGEX  
$ @[.VMS-SUPPORT]BUILD BDFTOPCF  
$ @[.VMS-SUPPORT]BUILD FSLSFONTS  
$ @[.VMS-SUPPORT]BUILD SHOWFONT  
$ @[.VMS-SUPPORT]BUILD XFSINFO  
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
