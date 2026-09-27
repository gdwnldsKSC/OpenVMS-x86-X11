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
FONTUTIL. Produces `FONTUTIL.OLB` - all ten unchanged modules from `lib/font/util`; not the complete Xfont library.  
FONTIO. Produces `FONTIO.OLB` - `bufio.c`, `fileio.c`, `filewr.c`, `decompress.c`, and `gunzip.c`, using `FONTUTIL.OLB` and `Z.OLB`.  
FNTSTUBS. Produces `FNTSTUBS.OLB` - all 17 unchanged modules from `lib/font/stubs`, for standalone font tools.  
BDFTOPCF. Produces `BDFTOPCF.EXE` - the upstream BDF-to-PCF font converter, using the required Xfont objects; not the complete Xfont library; upstream `-p8` defect remains.  
  
## Notes  
  
$ @[.VMS-SUPPORT]BUILD HEADERS  
$ @[.VMS-SUPPORT]BUILD XAU  
$ @[.VMS-SUPPORT]BUILD XDMCP  
$ @[.VMS-SUPPORT]BUILD ZLIB  
$ @[.VMS-SUPPORT]BUILD FONTENC  
$ @[.VMS-SUPPORT]BUILD EXPAT  
$ @[.VMS-SUPPORT]BUILD LBXUTIL  
$ @[.VMS-SUPPORT]BUILD FONTUTIL  
$ @[.VMS-SUPPORT]BUILD FONTIO  
$ @[.VMS-SUPPORT]BUILD FNTSTUBS  
$ @[.VMS-SUPPORT]BUILD BDFTOPCF  
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
