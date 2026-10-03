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
REGEX. Produces `REGEX.OLB` - all four upstream regular-expression modules, with native generation of their public and private headers.  
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
FREETYPE2. Produces `FREETYPE.OLB` - the complete 34-object upstream Unix profile of bundled FreeType 2.1.8, with an essential P64 PostScript-table relocation adaptation.  
MKFONTSCALE. Produces `MKFONTSCALE.EXE` - all four upstream font-index generator modules, using `FONTENC.OLB`, `FREETYPE.OLB`, and `Z.OLB`.  
FONTTOSFNT. Produces `FONTTOSFNT.EXE` - all five upstream bitmap-to-sfnt converter modules, with essential fixed-field initializer and C99 varargs retry adaptations, using `FREETYPE.OLB`, `FONTENC.OLB`, and `Z.OLB`.  
REVPATH. Produces `REVPATH.EXE` - the unchanged upstream relative-path helper from `config/util`.  
RMAN. Produces `RMAN.EXE` - the unchanged upstream PolyglotMan manual-page converter from `extras/rman`, using the `config/util` build definitions.  
UCS2ANY. Produces `UCS2ANY.EXE` - the unchanged upstream BDF character-encoding converter from `fonts/util`; use one mapping per invocation because of an upstream multi-mapping cleanup defect.  
DAMAGE. Produces `DAMAGE.OLB` - the unchanged internal server damage-tracking library from `programs/Xserver/miext/damage`; static profile with pixmap privates, excluding Render, Composite, and rootless hooks.  
MI. Produces `MI.OLB` - all 38 unchanged machine-independent server modules from `programs/Xserver/mi`; static core profile with pixmap privates and native CRTL cube roots, excluding optional extensions and loadable hooks; upstream bank-separation limits remain.  
CBRT. Produces `CBRT.OLB` - the separate unchanged upstream cube-root fallback from `programs/Xserver/mi`.  
FB. Produces `FB.OLB` - all 35 unchanged upstream framebuffer modules from `programs/Xserver/fb`; static Render-enabled profile including 24-bit and 24/32 conversion paths, excluding loadable hooks and GCC MMX.  
RENDER. Produces `RENDER.OLB` - all 12 unchanged upstream Render extension modules from `programs/Xserver/render`; static profile excluding optional extension hooks; upstream triangle-strip/fan allocation and sampling/edge-stepping defects remain.  
DIX. Produces `DIX.OLB` - all 23 unchanged upstream device-independent server modules from `programs/Xserver/dix`, plus a native entry adapter for the 64-bit environment-vector boundary; static Render-enabled profile excluding optional extension hooks.  
XPSTUBS. Produces `XPSTUBS.OLB` - the separate unchanged upstream non-Xprint server auxiliary from `programs/Xserver/dix`.  
SHADOW. Produces `SHADOW.OLB` - all 15 unchanged upstream shadow-framebuffer modules from `programs/Xserver/miext/shadow`; static Render-enabled profile with packed, planar, and rotation paths, excluding loadable hooks.  
RANDR. Produces `RANDR.OLB` - both unchanged upstream Resize and Rotate extension modules from `programs/Xserver/randr`; static Render-enabled profile; MI fallback does not change display modes, and the upstream rate-array cleanup defect remains.  
DBE. Produces `DBE.OLB` - both unchanged upstream Double Buffer Extension modules from `programs/Xserver/dbe`; static profile with the machine-independent implementation, excluding loadable hooks.  
RECORD. Produces `RECORD.OLB` - both unchanged upstream protocol-recording extension modules from `programs/Xserver/record`; static profile with the range-set implementation, excluding loadable hooks.  
LAYER. Produces `LAYER.OLB` - all four unchanged upstream screen-layer modules from `programs/Xserver/miext/layer`, with native internal-symbol aliases; static Render-enabled profile excluding loadable hooks; upstream window-loop defects remain.  
XFIXES. Produces `XFIXES.OLB` - all five unchanged upstream X Fixes extension modules from `programs/Xserver/xfixes`; static Render-enabled profile; linking requires an XFIXES-enabled server core.  
COMPOSITE. Produces `COMPOSITE.OLB` - all four unchanged upstream Composite extension modules from `programs/Xserver/composite`; static Render-enabled profile; linking requires matching COMPOSITE/XFIXES server layouts and Damage extension support.  
CW. Produces `CW.OLB` - all three unchanged upstream Composite wrapper modules from `programs/Xserver/miext/cw`; static Render-enabled profile with native C99 assertion-name mapping; requires matching Composite server layouts and screen initialization.  
DAMAGEEXT. Produces `DAMAGE.OLB` in the separate damageext output directory - the unchanged upstream Damage protocol extension module; requires internal Damage tracking and matching server layouts.  
EXT. Produces `EXT.OLB` - all 11 modules of the selected upstream static Xext profile, including ScreenSaver; optional platform and loadable extensions excluded.  
XINPUT. Produces `XINPUT.OLB` - all 37 upstream Unix X Input library modules; requires an XINPUT-enabled server core and device-dependent hooks; upstream lint-only stubs excluded.  
OS. Produces `OS.OLB` - all 15 modules of the upstream TCP/XDMCP, MIT-cookie and text-RGB profile, with minimal VMS/P64 fixes and one native process adapter; process helpers use DCL and reject images installed with added privileges or rights.  
MFB. Produces `MFB.OLB` - all 47 standard monochrome framebuffer objects from unchanged upstream sources, including specialized drawing variants; static profile excluding banked and loadable hooks.  
VFB. Produces `VFB.OLB` - all four upstream virtual-framebuffer DDx objects from unchanged sources; malloc framebuffer with Render, excluding SHM, MMAP, and DPMS; not a standalone Xvfb executable.  
XKB. Produces `XKB.OLB` - all 35 upstream keyboard-extension objects from unchanged sources, including generic DDx hooks and X Input event support; requires matching XKB/XINPUT server layouts, keymaps, and xkbcomp for runtime use.  
LBX. Produces `LBX.OLB` - all 12 upstream Low Bandwidth X server extension objects from unchanged sources; requires matching LBX server layouts, `LBXUTIL.OLB`, and `Z.OLB`.  
CFB. Produces `CFB.OLB` - all 57 standard 8-bit color-framebuffer objects, with minimal P64 address-alignment and glyph-store sequencing fixes; static profile excluding banked, assembly, and loadable hooks; native support supplies 32-bit unaligned loads and RTL declarations.  
CFB16. Produces `CFB16.OLB` - all 52 standard 16-bit color-framebuffer objects, with minimal P64 address-alignment fixes; the same static profile and native support as CFB.  
CFB24. Produces `CFB24.OLB` - all 52 standard 24-bit color-framebuffer objects, with minimal P64 address-alignment fixes; the same static profile and native support as CFB.  
CFB32. Produces `CFB32.OLB` - all 52 standard 32-bit color-framebuffer objects, with minimal P64 address-alignment fixes; the same static profile and native support as CFB.  
XKBFILE. Produces `XKBFILE.OLB` - all 13 unchanged upstream keyboard-file library modules; client profile requiring native Xlib when linked.  
XKBUI. Produces `XKBUI.OLB` - the unchanged upstream keyboard user-interface library module; requires native Xlib and `XKBFILE.OLB` when linked.  
ROOTLESS. Produces `ROOTLESS.OLB` - all five upstream rootless modules, with minimal C99/P64 header fixes; generic opaque Render/SHAPE profile without acceleration; requires matching SHAPE server layouts and device-dependent frame hooks.  
XTRAP. Produces `XTRAP.OLB` - all four unchanged upstream XTrap server extension modules; static profile excluding PC, X Input, and loadable hooks; requires server initialization and device-dependent hooks.  
XEXT. Produces `XEXT.OLB` - all 14 unchanged upstream ordinary X Extension client library modules, excluding XShm; requires matching native 64-bit Xlib when linked.  
XRENDER. Produces `XRENDER.OLB` - all 13 unchanged upstream Render client library modules; requires matching native 64-bit Xlib when linked.  
XRANDR. Produces `XRANDR.OLB` - the unchanged upstream Resize and Rotate client library module; requires matching native 64-bit Xlib, `XEXT.OLB`, and `XRENDER.OLB` when linked.  
XINERAMA. Produces `XINERAMA.OLB` - the unchanged upstream Xinerama client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XI. Produces `XI.OLB` - all 37 upstream X Input client library modules; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XTST. Produces `XTST.OLB` - both upstream X Test and Record client library modules; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XV. Produces `XV.OLB` - the upstream X Video client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked; shared-memory requests still require a compatible transport and server.  
XRES. Produces `XRES.OLB` - the upstream X Resource client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
  
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
$ @[.VMS-SUPPORT]BUILD REVPATH  
$ @[.VMS-SUPPORT]BUILD RMAN  
$ @[.VMS-SUPPORT]BUILD UCS2ANY  
$ @[.VMS-SUPPORT]BUILD DAMAGE  
$ @[.VMS-SUPPORT]BUILD MI  
$ @[.VMS-SUPPORT]BUILD CBRT  
$ @[.VMS-SUPPORT]BUILD FB  
$ @[.VMS-SUPPORT]BUILD RENDER  
$ @[.VMS-SUPPORT]BUILD DIX  
$ @[.VMS-SUPPORT]BUILD XPSTUBS  
$ @[.VMS-SUPPORT]BUILD SHADOW  
$ @[.VMS-SUPPORT]BUILD RANDR  
$ @[.VMS-SUPPORT]BUILD DBE  
$ @[.VMS-SUPPORT]BUILD RECORD  
$ @[.VMS-SUPPORT]BUILD LAYER  
$ @[.VMS-SUPPORT]BUILD XFIXES  
$ @[.VMS-SUPPORT]BUILD COMPOSITE  
$ @[.VMS-SUPPORT]BUILD CW  
$ @[.VMS-SUPPORT]BUILD DAMAGEEXT  
$ @[.VMS-SUPPORT]BUILD EXT  
$ @[.VMS-SUPPORT]BUILD XINPUT  
$ @[.VMS-SUPPORT]BUILD OS  
$ @[.VMS-SUPPORT]BUILD MFB  
$ @[.VMS-SUPPORT]BUILD VFB  
$ @[.VMS-SUPPORT]BUILD XKB  
$ @[.VMS-SUPPORT]BUILD LBX  
$ @[.VMS-SUPPORT]BUILD CFB  
$ @[.VMS-SUPPORT]BUILD CFB16  
$ @[.VMS-SUPPORT]BUILD CFB24  
$ @[.VMS-SUPPORT]BUILD CFB32  
$ @[.VMS-SUPPORT]BUILD XKBFILE  
$ @[.VMS-SUPPORT]BUILD XKBUI  
$ @[.VMS-SUPPORT]BUILD ROOTLESS  
$ @[.VMS-SUPPORT]BUILD XTRAP  
$ @[.VMS-SUPPORT]BUILD XEXT  
$ @[.VMS-SUPPORT]BUILD XRENDER  
$ @[.VMS-SUPPORT]BUILD XRANDR  
$ @[.VMS-SUPPORT]BUILD XINERAMA  
$ @[.VMS-SUPPORT]BUILD XI  
$ @[.VMS-SUPPORT]BUILD XTST  
$ @[.VMS-SUPPORT]BUILD XV  
$ @[.VMS-SUPPORT]BUILD XRES  
  
Native x86-64 with 64-bit pointers. Requires VSI C and DECset MMS.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
