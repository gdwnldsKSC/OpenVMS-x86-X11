# OpenVMS x86 X11 R7.4 Port Effort  
  
Currently, this is the complete modular X11 R7.4 source tree, with the bundled dependencies required by the native build.  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  

The following R7.4 build profiles have passed native OpenVMS x86-64 builds with 64-bit pointers; the standalone GAWK build tool uses 32-bit pointers. Xvfb runtime operation has also been verified.
  
XAU. Produces `XAU.OLB` - all eight standard Xau modules; optional Kerberos support excluded.  
XDMCP. Produces `XDMCP.OLB` - all 38 standard Xdmcp modules; optional `HASXDMAUTH` wrapping excluded.  
ZLIB. Produces `Z.OLB` - all 12 modules of the bundled zlib 1.2.3.  
FONTENC. Produces `FONTENC.OLB` - `fontenc.c` and `encparse.c`, using `Z.OLB`; upstream reverse-map cleanup and parser-growth defects remain.  
EXPAT. Produces `EXPAT.OLB` - `xmlparse.c`, `xmltok.c`, and `xmlrole.c` from bundled Expat 1.95.8.  
XFONT. Produces `XFONT.OLB` - 69 objects across bitmap, fontfile, fc, util, builtins, fontcache, FreeType, and Speedo; gzip and TCP font-server transport included, with minimal VMS text-read mode and font-module const-declaration adaptations. Builds `Z.OLB` and `FREETYPE.OLB` dependencies; Type1/CID and fontencc excluded. Builtins and fontcache APIs require server-side activation.  
FNTSTUBS. Produces `FNTSTUBS.OLB` - all 18 unchanged libXfont stub modules, for standalone font tools.  
FS. Produces `FS.OLB` - all 25 standard font-server client library modules, with TCP transport.  
ICE. Produces `ICE.OLB` - all 19 standard Inter-Client Exchange library modules, with TCP client and server transport.  
SM. Produces `SM.OLB` - all seven standard Session Management library modules, using `ICE.OLB`.  
ICEAUTH. Produces `ICEAUTH.EXE` - the upstream ICE authority-file tool, using `ICE.OLB`; VMS file-version cleanup and permission-inheritance limitations remain.  
MAKESTRS. Produces `MAKESTRS.EXE` - the upstream libXt string-table generator.  
MAKEDEPEND. Produces `MAKEDEPEND.EXE` - all six upstream C dependency-generator modules, emitting `.obj` targets.  
ATOBM. Produces `ATOBM.EXE` - the upstream bitmap package's ASCII-to-bitmap converter.  
GTF. Produces `GTF.EXE` - the upstream server's Generalized Timing Formula calculator.  
FREETYPE2. Produces `FREETYPE.OLB` - 39 objects from bundled FreeType 2.3.1: the complete 38-object default Unix profile plus retained FTAPI compatibility, with P64 relocation/address-membership changes in `psobjs.c` and a pointer-sized cache-hash conversion in `ftcache.h`.  
MKFONTSCALE. Produces `MKFONTSCALE.EXE` - all four upstream font-index generator modules, using `FONTENC.OLB`, `FREETYPE.OLB`, and `Z.OLB`.  
UCS2ANY. Produces `UCS2ANY.EXE` - the unchanged upstream font-util BDF character-encoding converter; use one mapping per invocation because of an upstream multi-mapping cleanup defect.  
PIXMAN. Produces `PIXMAN.OLB` - all 11 modules of the portable Pixman 0.9.6 profile; native P64 pointers and fixed-width pixels, with a minimal integer-limit fallback-guard fix. MMX and timing instrumentation excluded.  
DAMAGE. Produces `DAMAGE.OLB` - the unchanged internal server damage-tracking library from `xserver/xorg-server/miext/damage`; static profile with pixmap privates, excluding Render, Composite, and rootless hooks.  
MI. Produces `MI.OLB` - 36 machine-independent server modules plus the retained `miinitext` auxiliary; static core profile with pixmap privates and native CRTL cube roots, excluding optional extensions and loadable hooks; pointer-width-safe private-value casts; upstream bank-separation limits remain.  
FB. Produces `FB.OLB` - 30 upstream framebuffer modules plus the retained generic colormap provider, with minimal P64 alignment and framebuffer/Pixman buffer-type adaptations; static Render-enabled profile including 24-bit and 24/32 conversion paths, using `PIXMAN.OLB` for pixel compositing.  
RENDER. Produces `RENDER.OLB` - all 11 unchanged upstream Render extension modules from `xserver/xorg-server/render`; static profile excluding optional extension hooks, using `PIXMAN.OLB` and VSI SSL3 for SHA-1.  
DIX. Produces `DIX.OLB` - 28 selected upstream device-independent server modules from `xserver/xorg-server/dix`, with minimal optional-Shape and optional-XKB guards, plus a native entry adapter for the 64-bit environment-vector boundary; native CRTL supplies `ffs` and `strcasecmp`. Static Render-enabled profile excluding optional extension hooks.  
CONFIG. Produces `CONFIG.OLB` - the unchanged portable server configuration initialization/shutdown module; optional D-Bus and HAL backends excluded.  
XPSTUBS. Produces `XPSTUBS.OLB` - the separate unchanged upstream non-Xprint server auxiliary from `xserver/xorg-server/dix`.  
SHADOW. Produces `SHADOW.OLB` - all 20 unchanged upstream shadow-framebuffer modules from `xserver/xorg-server/miext/shadow`; static Render-enabled profile with packed, planar, and rotation paths, excluding loadable hooks.  
RANDR. Produces `RANDR.OLB` - all 11 selected upstream Resize and Rotate extension modules; static Render-enabled profile excluding Xinerama; the MI fallback does not change display modes.  
DBE. Produces `DBE.OLB` - both unchanged upstream Double Buffer Extension modules from `xserver/xorg-server/dbe`; static profile with the machine-independent implementation, excluding loadable hooks.  
RECORD. Produces `RECORD.OLB` - both unchanged upstream protocol-recording extension modules from `xserver/xorg-server/record`; static profile with the range-set implementation, excluding loadable hooks.  
XFIXES. Produces `XFIXES.OLB` - all five unchanged upstream X Fixes extension modules from `xserver/xorg-server/xfixes`; static Render-enabled profile; linking requires an XFIXES-enabled server core.  
COMPOSITE. Produces `COMPOSITE.OLB` - all four upstream Composite extension modules from `xserver/xorg-server/composite`; static Render-enabled profile with a pointer-typed sentinel; linking requires matching COMPOSITE/XFIXES server layouts and Damage extension support.  
CW. Produces `CW.OLB` - all three unchanged upstream Composite wrapper modules from `xserver/xorg-server/miext/cw`; static Render-enabled profile with native C99 assertion-name mapping; requires matching Composite server layouts and screen initialization.  
DAMAGEEXT. Produces `DAMAGE.OLB` in the separate damageext output directory - the unchanged upstream Damage protocol extension module; requires internal Damage tracking and matching server layouts.  
EXT. Produces `EXT.OLB` - all 19 modules of the selected upstream static Xext profile, including ScreenSaver, FontCache, DPMS, XVideo/XvMC, X Resource, Multibuffer, EVI, CUP, and XEvIE; Xprint is excluded because its protocol package is absent from R7.4. Remaining optional and loadable extensions excluded. Runtime use requires matching core feature flags, `XFONT.OLB` for FontCache, and device-specific DPMS/video hooks. XEvIE requires matching XKB/XEVIE core event support; its upstream swapped SelectInput request-size defect remains.  
XINPUT. Produces `XINPUT.OLB` - all 37 upstream server X Input extension modules; requires an XINPUT-enabled server core and device-dependent hooks; upstream lint-only stubs excluded.  
OS. Produces `OS.OLB` - all 15 modules of the upstream TCP/XDMCP, MIT-cookie and built-in RGB profile, with minimal VMS/P64 fixes and one native process adapter; process helpers use DCL and reject images installed with added privileges or rights.  
MFB. Produces `MFB.OLB` - all 45 standard monochrome framebuffer objects, with corrected upstream weak-callback declarations, including specialized drawing variants; static profile excluding banked and loadable hooks.  
VFB. Produces `VFB.OLB` - all four upstream virtual-framebuffer DDx objects from unchanged sources; malloc framebuffer with Render, excluding SHM, MMAP, and DPMS; not a standalone Xvfb executable.  
FBCMAP. Produces `FBCMAP.OLB` - the unchanged MI-based framebuffer colormap provider used by Xvfb, separate from the retained generic `FB` colormap.  
XKB. Produces `XKB.OLB` - all 31 upstream keyboard-extension and generic DDX stub objects, with a native string-function declaration adaptation and explicit keymap include path, including X Input event support; requires matching XKB/XINPUT server layouts, keymaps, and xkbcomp for runtime use.  
CFB. Produces `CFB.OLB` - all 55 standard 8-bit color-framebuffer objects, with minimal P64 address-alignment fixes; static profile excluding banked, assembly, and loadable hooks; native support supplies 32-bit unaligned loads and RTL declarations.  
CFB16. Produces `CFB16.OLB` - all 50 standard 16-bit color-framebuffer objects, with minimal P64 address-alignment fixes; the same static profile and native support as CFB.  
CFB32. Produces `CFB32.OLB` - all 50 standard 32-bit color-framebuffer objects, with minimal P64 address-alignment fixes; the same static profile and native support as CFB.  
XKBFILE. Produces `XKBFILE.OLB` - all 13 unchanged upstream keyboard-file library modules; client profile requiring native Xlib when linked.  
ROOTLESS. Produces `ROOTLESS.OLB` - all five upstream rootless modules, with minimal C99/P64 and platform-guard fixes; generic opaque Render/SHAPE profile without acceleration or XQuartz/Xplugin integration; requires matching SHAPE server layouts and device-dependent frame hooks.  
XTRAP. Produces `XTRAP.OLB` - all four upstream XTrap server extension modules, with a typed drawable-lookup adaptation; static profile excluding PC, X Input, and loadable hooks; requires server initialization and device-dependent hooks.  
XEXT. Produces `XEXT.OLB` - all 14 upstream ordinary X Extension client library modules, excluding XShm; one callback const-compatibility cast; requires matching native 64-bit Xlib when linked.  
XRENDER. Produces `XRENDER.OLB` - all 13 unchanged upstream Render client library modules; requires matching native 64-bit Xlib when linked.  
XRANDR. Produces `XRANDR.OLB` - all seven upstream Resize and Rotate client library modules, with a return-value guard adaptation; requires matching native 64-bit Xlib, `XEXT.OLB`, and `XRENDER.OLB` when linked.  
XINERAMA. Produces `XINERAMA.OLB` - the unchanged upstream Xinerama client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XI. Produces `XI.OLB` - all 37 upstream X Input client library modules; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XTST. Produces `XTST.OLB` - both upstream X Test and Record client library modules; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XV. Produces `XV.OLB` - the upstream X Video client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked; shared-memory requests still require a compatible transport and server.  
XRES. Produces `XRES.OLB` - the upstream X Resource client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XFIXESLIB. Produces `XFIXES.OLB` in the client-library output directory - all five unchanged upstream X Fixes client modules; requires matching native 64-bit Xlib when linked; distinct from the `XFIXES` server target.  
XDAMAGE. Produces `XDAMAGE.OLB` - the unchanged upstream Damage client library module; requires matching native 64-bit Xlib and the Xfixes client library when linked.  
XCOMPOSITE. Produces `XCOMPOSITE.OLB` - the unchanged upstream Composite client library module; uses the upstream Xlib, Xfixes, and Xdamage client dependency profile.  
XCURSOR. Produces `XCURSOR.OLB` - all five unchanged upstream cursor library modules; requires matching native 64-bit Xlib and `XRENDER.OLB` when linked; optional Xfixes naming hooks excluded, Unix-style theme paths and upstream cursor/theme-parser defects remain.  
XSS. Produces `XSS.OLB` - the unchanged upstream ScreenSaver client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked; upstream event root/window assignment defect remains.  
XVMC. Produces `XVMC.OLB` - the unchanged upstream hardware-independent X Video Motion Compensation client library module, excluding the Linux/DRI i810 backend; requires matching native 64-bit Xlib and `XEXT.OLB` when linked; hardware rendering needs a separate backend.  
XXF86VM. Produces `XXF86VM.OLB` - the unchanged upstream XFree86 Video Mode client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked, plus corresponding server support; upstream allocation-failure lock defects remain.  
XXF86MISC. Produces `XXF86MISC.OLB` - the unchanged upstream XFree86 Misc client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked, plus corresponding server support; upstream allocation-failure lock and null-device defects remain.  
XFONTCACHE. Produces `XFONTCACHE.OLB` - the unchanged upstream font-cache client library module; requires matching native 64-bit Xlib, `XEXT.OLB`, and corresponding server support.  
DMX. Produces `DMX.OLB` - the unchanged upstream distributed multihead client library module; requires matching native 64-bit Xlib, `XEXT.OLB`, and a DMX server.  
XMUU. Produces `XMUU.OLB` - all five unchanged upstream mini-Xmu library modules, using native snprintf; requires matching native 64-bit Xlib when linked; remains a separate upstream library from Xmu.  
XPM. Produces `XPM.OLB` - all 28 unchanged upstream X PixMap library modules, using native snprintf, upstream string helpers, and plain-file I/O without compressed-file subprocesses; retains upstream 32-bit count fields; requires matching native 64-bit Xlib when linked.  
X11. Produces `X11.OLB` - static Xlib with XKB and TCP, including P64 Compose-table relocation and an input-method pointer cast.  
XAUTH. Produces `XAUTH.EXE`.  
XDPYINFO. Produces `XDPYINFO.EXE`.  
XWININFO. Produces `XWININFO.EXE`.  
XSET. Produces `XSET.EXE`.  
XMODMAP. Produces `XMODMAP.EXE`.  
XEV. Produces `XEV.EXE`.  
BISON. Produces `BISON.EXE` from bundled, unchanged GNU Bison 1.35.  
GAWK. Produces `GAWK.EXE` from bundled GNU Gawk 5.4.1 for upstream build generators; standalone build tool using 32-bit pointers.  
XKBCOMP. Produces `XKBCOMP.EXE`; builds and uses bundled native Bison automatically.  
SETXKBMAP. Produces `SETXKBMAP.EXE`.

XKB_DATA. Stages 273 upstream xkeyboard-config data files, two byte-identical `xorg` rules aliases, and the native compiler together.

XT. Produces `XT.OLB` — complete static Xt, with P64 host types and generated string tables.  
XMU. Produces `XMU.OLB` — complete static Xmu.  
XVFB. Produces `XVFB.EXE` — headless core/Render/XKB/X Input server, linked with VSI SSL3's P64 libcrypto.  
XF86_VIDEO_VMWARE. Produces `VMWARE_DRV.OLB` — all six xf86-video-vmware 10.16.5 driver modules with 64-bit pointers and module metadata; native packing for Xv command structures. The consuming server supplies XFree86 and loader services. Does not link or activate the driver.  
SHADOWFB. Produces `SHADOWFB.OLB` — both unchanged upstream XFree86 shadowfb objects, including module metadata and Render before/after drawing callbacks used by VMware. Separate from `SHADOW`.  
VGAHW. Produces `VGAHW.OLB` — both unchanged upstream XFree86 VGA hardware support objects with 64-bit pointers, including module metadata. The consuming server supplies port I/O and XFree86 services. Does not access hardware during the build.  
RAMDAC. Produces `RAMDAC.OLB` — all eight unchanged upstream XFree86 RAMDAC/cursor objects, including IBM, BT, TI, and the upstream bit-order wrapper; optional ARGB cursors excluded. The consuming server supplies XFree86 and cursor services.  
XAA. Produces `XAA.OLB` — all 53 static XFree86 acceleration objects, including bit-order, fixed-base, triple-bit and polysegment variants; Render enabled, with minimal P64 address-alignment fixes. Loadable hooks, Xinerama and architecture assembly excluded.  
XF86CONFIG. Produces `XF86CONFIG.OLB` — all 16 upstream XFree86 configuration-parser C sources unchanged, with VMS-only external-name aliases in headers. Reads, validates and writes configuration data; the consuming program supplies diagnostics. Uses current-process file permissions without Unix saved-ID switching; upstream Vendor-subsection Identifier and built-in scanner-reset defects remain.  
RAC. Produces `RAC.OLB` — the unchanged static XFree86 resource-access control module with Render, framebuffer, cursor, colormap and viewport wrappers; loader-only metadata excluded. The consuming server supplies device-access and core services; upstream CloseScreen omits CreatePixmap restoration.  
COMMON. Produces `COMMON.OLB` — all 29 objects of the selected static XFree86 common profile, including generated default modes, with minimal type corrections for module lists, pixel clocks, resource masks and mode copying, plus a standard variadic debug-macro adaptation; XKB, X Input and Render enabled, dynamic image loading and XVideo excluded. The consuming server supplies OS, PCI and core services.  
INIT. Produces `INIT.OLB` — the unchanged upstream XFree86 initialization object; the consuming server supplies OS and device-dependent services.  
LOADER. Produces `LOADER.OLB` — five upstream loader objects with a minimal static-module platform boundary, retaining upstream module and ABI checks; the consuming server supplies native module discovery and loading. Dynamic image loading and Unix symbol tables excluded.  
BUS. Produces `BUS.OLB` — the unchanged generic XFree86 PCI bus module with 64-bit pointers; the consuming platform supplies PCI access, resource sizes and address translation. Native hardware backends and generic domain/mapping assumptions are not selected.  
MISC. Produces `MISC.OLB` — all five unchanged portable C modules of XFree86 OS-support libmisc; the consuming platform supplies port I/O, delay and interrupt exclusion.  
XORGXKB. Produces `XORGXKB.OLB` — all three unchanged XFree86 keyboard-action modules with 64-bit pointers; the consuming server supplies action dispatch. Replaces, rather than accompanies, the generic XKB DDX stubs at link time.  
XORGFB. Produces `FB.OLB` — the 32-object Xorg framebuffer variant with module metadata and the MI colormap provider, retaining the P64 alignment and framebuffer/Pixman buffer-type adaptations. Generic `FB` and `XSERVER_CORE` builds remain separate.  
XF86_INPUT_KEYBOARD. Produces `KBD_DRV.OLB` — the unchanged upstream keyboard driver and AT scan-code helper with 64-bit pointers and module metadata; the consuming server supplies native keyboard services and key mapping.  
XF86_INPUT_MOUSE. Produces `MOUSE_DRV.OLB` — both unchanged upstream mouse driver sources with 64-bit pointers and module metadata; `COMMON.OLB` supplies input buffering, and the consuming server supplies native mouse and serial services.  
MKFONTDIR. Produces the native font-index launcher.  
FONTS. Stages 1,006 BDF fonts and scalable-font/encoding indexes.  
NLS. Builds locale and Compose data for 58 locales.  
X11_DATA. Copies XErrorDB, XKeysymDB, and Xcms.txt.

RUNTIME. Stages BDF fonts/indexes, locale/Xlib/XKB data, and native utilities; no system installation or automatic startup.
  
## Notes  
  
$ @[.VMS-SUPPORT]BUILD HEADERS  
$ @[.VMS-SUPPORT]BUILD XAU  
$ @[.VMS-SUPPORT]BUILD XDMCP  
$ @[.VMS-SUPPORT]BUILD ZLIB  
$ @[.VMS-SUPPORT]BUILD FONTENC  
$ @[.VMS-SUPPORT]BUILD EXPAT  
$ @[.VMS-SUPPORT]BUILD XFONT  
$ @[.VMS-SUPPORT]BUILD FNTSTUBS  
$ @[.VMS-SUPPORT]BUILD FS  
$ @[.VMS-SUPPORT]BUILD ICE  
$ @[.VMS-SUPPORT]BUILD SM  
$ @[.VMS-SUPPORT]BUILD ICEAUTH  
$ @[.VMS-SUPPORT]BUILD MAKESTRS  
$ @[.VMS-SUPPORT]BUILD MAKEDEPEND  
$ @[.VMS-SUPPORT]BUILD ATOBM  
$ @[.VMS-SUPPORT]BUILD GTF  
$ @[.VMS-SUPPORT]BUILD FREETYPE2  
$ @[.VMS-SUPPORT]BUILD MKFONTSCALE  
$ @[.VMS-SUPPORT]BUILD UCS2ANY  
$ @[.VMS-SUPPORT]BUILD PIXMAN  
$ @[.VMS-SUPPORT]BUILD DAMAGE  
$ @[.VMS-SUPPORT]BUILD MI  
$ @[.VMS-SUPPORT]BUILD FB  
$ @[.VMS-SUPPORT]BUILD RENDER  
$ @[.VMS-SUPPORT]BUILD DIX  
$ @[.VMS-SUPPORT]BUILD CONFIG  
$ @[.VMS-SUPPORT]BUILD XPSTUBS  
$ @[.VMS-SUPPORT]BUILD SHADOW  
$ @[.VMS-SUPPORT]BUILD RANDR  
$ @[.VMS-SUPPORT]BUILD DBE  
$ @[.VMS-SUPPORT]BUILD RECORD  
$ @[.VMS-SUPPORT]BUILD XFIXES  
$ @[.VMS-SUPPORT]BUILD COMPOSITE  
$ @[.VMS-SUPPORT]BUILD CW  
$ @[.VMS-SUPPORT]BUILD DAMAGEEXT  
$ @[.VMS-SUPPORT]BUILD EXT  
$ @[.VMS-SUPPORT]BUILD XINPUT  
$ @[.VMS-SUPPORT]BUILD OS  
$ @[.VMS-SUPPORT]BUILD MFB  
$ @[.VMS-SUPPORT]BUILD VFB  
$ @[.VMS-SUPPORT]BUILD FBCMAP  
$ @[.VMS-SUPPORT]BUILD XKB  
$ @[.VMS-SUPPORT]BUILD CFB  
$ @[.VMS-SUPPORT]BUILD CFB16  
$ @[.VMS-SUPPORT]BUILD CFB32  
$ @[.VMS-SUPPORT]BUILD XKBFILE  
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
$ @[.VMS-SUPPORT]BUILD XFIXESLIB  
$ @[.VMS-SUPPORT]BUILD XDAMAGE  
$ @[.VMS-SUPPORT]BUILD XCOMPOSITE  
$ @[.VMS-SUPPORT]BUILD XCURSOR  
$ @[.VMS-SUPPORT]BUILD XSS  
$ @[.VMS-SUPPORT]BUILD XVMC  
$ @[.VMS-SUPPORT]BUILD XXF86VM  
$ @[.VMS-SUPPORT]BUILD XXF86MISC  
$ @[.VMS-SUPPORT]BUILD XFONTCACHE  
$ @[.VMS-SUPPORT]BUILD DMX  
$ @[.VMS-SUPPORT]BUILD XMUU  
$ @[.VMS-SUPPORT]BUILD XPM  
$ @[.VMS-SUPPORT]BUILD X11  
$ @[.VMS-SUPPORT]BUILD XAUTH  
$ @[.VMS-SUPPORT]BUILD XDPYINFO  
$ @[.VMS-SUPPORT]BUILD XWININFO  
$ @[.VMS-SUPPORT]BUILD XSET  
$ @[.VMS-SUPPORT]BUILD XMODMAP  
$ @[.VMS-SUPPORT]BUILD XEV  
$ @[.VMS-SUPPORT]BUILD BISON  
$ @[.VMS-SUPPORT]BUILD GAWK  
$ @[.VMS-SUPPORT]BUILD XKBCOMP  
$ @[.VMS-SUPPORT]BUILD SETXKBMAP  
$ @[.VMS-SUPPORT]BUILD XKB_DATA  
$ @[.VMS-SUPPORT]BUILD RUNTIME  
$ @[.VMS-SUPPORT]BUILD XT  
$ @[.VMS-SUPPORT]BUILD XMU  
$ @[.VMS-SUPPORT]BUILD XVFB  
$ @[.VMS-SUPPORT]BUILD XF86_VIDEO_VMWARE  
$ @[.VMS-SUPPORT]BUILD SHADOWFB  
$ @[.VMS-SUPPORT]BUILD VGAHW  
$ @[.VMS-SUPPORT]BUILD RAMDAC  
$ @[.VMS-SUPPORT]BUILD XAA  
$ @[.VMS-SUPPORT]BUILD XF86CONFIG  
$ @[.VMS-SUPPORT]BUILD RAC  
$ @[.VMS-SUPPORT]BUILD COMMON  
$ @[.VMS-SUPPORT]BUILD INIT  
$ @[.VMS-SUPPORT]BUILD LOADER  
$ @[.VMS-SUPPORT]BUILD BUS  
$ @[.VMS-SUPPORT]BUILD MISC  
$ @[.VMS-SUPPORT]BUILD XORGXKB  
$ @[.VMS-SUPPORT]BUILD XORGFB  
$ @[.VMS-SUPPORT]BUILD XF86_INPUT_KEYBOARD  
$ @[.VMS-SUPPORT]BUILD XF86_INPUT_MOUSE  
$ @[.VMS-SUPPORT]BUILD MKFONTDIR  
$ @[.VMS-SUPPORT]BUILD FONTS  
$ @[.VMS-SUPPORT]BUILD NLS  
$ @[.VMS-SUPPORT]BUILD X11_DATA  
  
Native x86-64 X11 libraries and servers with 64-bit pointers; the standalone GAWK build tool uses 32-bit pointers. Requires VSI C, DECset MMS, and VSI SSL3 headers/P64 libcrypto.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
COMMON builds bundled GNU Gawk 5.4.1 automatically to run the upstream default-mode generator.  
XKBCOMP builds bundled GNU Bison 1.35 automatically; no separate download or BISON command setup is required.  
