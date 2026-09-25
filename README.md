# OpenVMS x86 X11 R6.8.2 Port Effort  
  
Currently, this is just the straight X11 R6.8.2 tree  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  
  
- `lib/Xau/AuRead.c` - `XauReadAuth`.  
- `lib/Xau/AuDispose.c` - `XauDisposeAuth`.  
- `XAU_READ.OLB` - the two modules above, not the complete Xau library.  
  
## Notes  

$ @[.VMS-SUPPORT]BUILD HEADERS
$ @[.VMS-SUPPORT]BUILD XAU_READ
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
