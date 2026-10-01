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
ICE. Produces `ICE.OLB` - all 19 standard Inter-Client Exchange library modules, with TCP client and server transport.  
SM. Produces `SM.OLB` - all seven standard Session Management library modules, using `ICE.OLB`.  
BDFTOPCF. Produces `BDFTOPCF.EXE` - the upstream BDF-to-PCF font converter, using `XFONT.OLB` and `Z.OLB`; upstream `-p8` defect remains.  
FSLSFONTS. Produces `FSLSFONTS.EXE` - the upstream font-server font listing tool, using `FS.OLB`.  
SHOWFONT. Produces `SHOWFONT.EXE` - the upstream font-server glyph and metrics viewer, using `FS.OLB`.  
XFSINFO. Produces `XFSINFO.EXE` - the upstream font-server information tool, using `FS.OLB`.  
ICEAUTH. Produces `ICEAUTH.EXE` - the upstream ICE authority-file tool, using `ICE.OLB`; VMS file-version cleanup and permission-inheritance limitations remain.  
MAKEPSRES. Produces `MAKEPSRES.EXE` - the upstream PostScript resource-directory generator; use Unix-style paths with `DECC$EFS_CASE_PRESERVE` enabled; backups require hard links (or `-nb`); upstream `-f -` limitation remains.  
SHOWRGB. Produces `SHOWRGB.EXE` - the upstream text color-database viewer (`USE_RGB_TXT`), reading `programs/rgb/rgb.txt` by default when run from the repository root.  
MAKESTRS. Produces `MAKESTRS.EXE` - the upstream string-table generator from `config/util`.  
MAKEDEPEND. Produces `MAKEDEPEND.EXE` - all six upstream C dependency-generator modules from `config/makedepend`, emitting `.obj` targets.  
LNDIR. Produces `LNDIR.EXE` - the shadow symbolic-link tree generator from `config/util`, with OpenVMS directory traversal and case-preserving names; requires symbolic-link support and Unix-style paths.  
ATOBM. Produces `ATOBM.EXE` - the upstream ASCII-to-bitmap converter from `programs/bitmap`.  
GTF. Produces `GTF.EXE` - the upstream Generalized Timing Formula calculator from `programs/Xserver/hw/xfree86/etc`.  
FREETYPE2. Produces `FREETYPE.OLB` - the complete 34-object upstream Unix profile of bundled FreeType 2.1.8, with an essential P64 PostScript-table relocation adaptation; based in part on the work of the FreeType Team and Catharon Productions, Inc.  
MKFONTSCALE. Produces `MKFONTSCALE.EXE` - all four upstream font-index generator modules, using `FONTENC.OLB`, `FREETYPE.OLB`, and `Z.OLB`.  
FONTTOSFNT. Produces `FONTTOSFNT.EXE` - all five upstream bitmap-to-sfnt converter modules, with essential fixed-field initializer and C99 varargs retry adaptations, using `FREETYPE.OLB`, `FONTENC.OLB`, and `Z.OLB`.  
  
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
$ @[.VMS-SUPPORT]BUILD ICE  
$ @[.VMS-SUPPORT]BUILD SM  
$ @[.VMS-SUPPORT]BUILD BDFTOPCF  
$ @[.VMS-SUPPORT]BUILD FSLSFONTS  
$ @[.VMS-SUPPORT]BUILD SHOWFONT  
$ @[.VMS-SUPPORT]BUILD XFSINFO  
$ @[.VMS-SUPPORT]BUILD ICEAUTH  
$ @[.VMS-SUPPORT]BUILD MAKEPSRES  
$ @[.VMS-SUPPORT]BUILD SHOWRGB  
$ @[.VMS-SUPPORT]BUILD MAKESTRS  
$ @[.VMS-SUPPORT]BUILD MAKEDEPEND  
$ @[.VMS-SUPPORT]BUILD LNDIR  
$ @[.VMS-SUPPORT]BUILD ATOBM  
$ @[.VMS-SUPPORT]BUILD GTF  
$ @[.VMS-SUPPORT]BUILD FREETYPE2  
$ @[.VMS-SUPPORT]BUILD MKFONTSCALE  
$ @[.VMS-SUPPORT]BUILD FONTTOSFNT  
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
