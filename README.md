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
- `lib/Xdmcp/Alloc.c` - `Xalloc`, `Xrealloc`, `Xfree`.
- `lib/Xdmcp/A8Eq.c` - `XdmcpARRAY8Equal`.
- `lib/Xdmcp/CA8.c` - `XdmcpCopyARRAY8`.
- `lib/Xdmcp/AA8.c` - `XdmcpAllocARRAY8`.
- `lib/Xdmcp/AA16.c` - `XdmcpAllocARRAY16`.
- `lib/Xdmcp/AA32.c` - `XdmcpAllocARRAY32`.
- `lib/Xdmcp/AofA8.c` - `XdmcpAllocARRAYofARRAY8`.
- `lib/Xdmcp/DA8.c` - `XdmcpDisposeARRAY8`.
- `lib/Xdmcp/DA16.c` - `XdmcpDisposeARRAY16`.
- `lib/Xdmcp/DA32.c` - `XdmcpDisposeARRAY32`.
- `lib/Xdmcp/DAofA8.c` - `XdmcpDisposeARRAYofARRAY8`.
- `lib/Xdmcp/RA8.c` - `XdmcpReadARRAY8`.
- `lib/Xdmcp/RA16.c` - `XdmcpReadARRAY16`.
- `lib/Xdmcp/RA32.c` - `XdmcpReadARRAY32`.
- `lib/Xdmcp/RAofA8.c` - `XdmcpReadARRAYofARRAY8`.
- `lib/Xdmcp/RaA8.c` - `XdmcpReallocARRAY8`.
- `lib/Xdmcp/RaA16.c` - `XdmcpReallocARRAY16`.
- `lib/Xdmcp/RaA32.c` - `XdmcpReallocARRAY32`.
- `lib/Xdmcp/RaAoA8.c` - `XdmcpReallocARRAYofARRAY8`.
- `lib/Xdmcp/WA8.c` - `XdmcpWriteARRAY8`.
- `lib/Xdmcp/WA16.c` - `XdmcpWriteARRAY16`.
- `lib/Xdmcp/WA32.c` - `XdmcpWriteARRAY32`.
- `lib/Xdmcp/WAofA8.c` - `XdmcpWriteARRAYofARRAY8`.
- `XDMCP_CODEC.OLB` - all 31 scalar, array, and allocation modules above; not the full Xdmcp library.
  
## Notes  

$ @[.VMS-SUPPORT]BUILD HEADERS
$ @[.VMS-SUPPORT]BUILD XAU
$ @[.VMS-SUPPORT]BUILD XDMCP_CODEC
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
