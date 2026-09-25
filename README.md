# OpenVMS x86 X11 R6.8.2 Port Effort  
  
Currently, this is just the straight X11 R6.8.2 tree  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  
  
- `lib/Xau/AuRead.c` - `XauReadAuth`.  
- `lib/Xau/AuDispose.c` - `XauDisposeAuth`.  
- `lib/Xau/AuWrite.c` - `XauWriteAuth`.
- `lib/Xau/AuFileName.c` - `XauFileName`.
- `lib/Xau/AuGetAddr.c` - `XauGetAuthByAddr`.
- `lib/Xau/AuGetBest.c` - `XauGetBestAuthByAddr`.
- `lib/Xau/AuLock.c` - `XauLockAuth`.
- `lib/Xau/AuUnlock.c` - `XauUnlockAuth`.
- `XAU.OLB` - all eight standard Xau modules; optional Kerberos support excluded.
  
## Notes  

$ @[.VMS-SUPPORT]BUILD HEADERS
$ @[.VMS-SUPPORT]BUILD XAU
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
