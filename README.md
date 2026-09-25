# OpenVMS x86 X11 R6.8.2 Port Effort  
  
Currently, this is just the straight X11 R6.8.2 tree  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  
  
XAU. Produces `XAU.OLB` - all eight standard Xau modules; optional Kerberos support excluded.
XDMCP - in progress, these are done so far:
- `lib/Xdmcp/RC8.c` - `XdmcpReadCARD8`.
- `lib/Xdmcp/RC16.c` - `XdmcpReadCARD16`.
- `lib/Xdmcp/RC32.c` - `XdmcpReadCARD32`.
- `lib/Xdmcp/WC8.c` - `XdmcpWriteCARD8`.
- `lib/Xdmcp/WC16.c` - `XdmcpWriteCARD16`.
- `lib/Xdmcp/WC32.c` - `XdmcpWriteCARD32`.
- `lib/Xdmcp/RHead.c` - `XdmcpReadHeader`.
- `lib/Xdmcp/RR.c` - `XdmcpReadRemaining`.
- `XDMCP_CODEC.OLB` - the eight scalar codec modules above; not the full Xdmcp library.
  
## Notes  

$ @[.VMS-SUPPORT]BUILD HEADERS
$ @[.VMS-SUPPORT]BUILD XAU
$ @[.VMS-SUPPORT]BUILD XDMCP_CODEC
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
