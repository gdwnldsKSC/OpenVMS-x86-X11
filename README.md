# OpenVMS x86 X11 R6.8.2 Port Effort  
  
Currently, this is just the straight X11 R6.8.2 tree  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  
  
XAU. Produces `XAU.OLB` - all eight standard Xau modules; optional Kerberos support excluded.
XDMCP. Produces `XDMCP.OLB` - all 38 standard Xdmcp modules; optional `HASXDMAUTH` wrapping excluded.
ZLIB. Produces `Z.OLB` - all 14 modules of the bundled zlib 1.1.4.
FONTENC. Produces `FONTENC.OLB` - `fontenc.c` and `encparse.c`, using `Z.OLB`; upstream reverse-map cleanup and parser-growth defects remain.
EXPAT. Produces `EXPAT.OLB` - `xmlparse.c`, `xmltok.c`, and `xmlrole.c` from bundled Expat 1.95.6.
LBX_IMAGE. Produces `LBX_IMAGE.OLB` - all five image-codec modules from `lbxutil/image`; not the complete LBX utility library.
 
## Notes  

$ @[.VMS-SUPPORT]BUILD HEADERS
$ @[.VMS-SUPPORT]BUILD XAU
$ @[.VMS-SUPPORT]BUILD XDMCP_CODEC
$ @[.VMS-SUPPORT]BUILD XDMCP
$ @[.VMS-SUPPORT]BUILD ZLIB
$ @[.VMS-SUPPORT]BUILD FONTENC
$ @[.VMS-SUPPORT]BUILD EXPAT
$ @[.VMS-SUPPORT]BUILD LBX_IMAGE
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
