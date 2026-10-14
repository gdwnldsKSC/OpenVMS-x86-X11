# OpenVMS x86 X11 R7.7 Port Effort  
  
Currently, this is the complete modular X11 R7.7 source tree, with the bundled dependencies required by the native build.  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  

The listed targets build natively on OpenVMS x86-64 E9.2-4 with the R7.7 sources. X11 uses 64-bit pointers; the standalone GAWK build tool uses 32-bit pointers.
  
XAU. Produces `XAU.OLB` - all eight standard Xau modules; optional Kerberos support excluded.  
XDMCP. Produces `XDMCP.OLB` - all eight standard Xdmcp modules; optional `HASXDMAUTH` wrapping excluded.  
XCB. Produces `XCB.OLB` - the core TCP client transport and generated core, BIG-REQUESTS, and XC-MISC protocols, using native pthreads and `XAU.OLB`.  
XCB_SHAPE. Produces `XCB_SHAPE.OLB` - the generated Shape protocol client library used by xwininfo.  
ZLIB. Produces `Z.OLB` - all 12 modules of the bundled zlib 1.2.3.  
FONTENC. Produces `FONTENC.OLB` - `fontenc.c` and `encparse.c`, using `Z.OLB`; upstream reverse-map cleanup and parser-growth defects remain.  
EXPAT. Produces `EXPAT.OLB` - `xmlparse.c`, `xmltok.c`, and `xmlrole.c` from bundled Expat 1.95.8.  
XFONT. Produces `XFONT.OLB` - 47 objects across bitmap, fontfile, fc, util, builtins, and FreeType; gzip and TCP font-server transport included, with minimal VMS text-read adaptations. Builds `Z.OLB`, `FONTENC.OLB`, and `FREETYPE.OLB` dependencies; optional fontencc excluded. Builtin fonts require server-side activation.  
FNTSTUBS. Produces `FNTSTUBS.OLB` - all 16 unchanged libXfont stub modules, for standalone font tools.  
FS. Produces `FS.OLB` - all 25 standard font-server client library modules, with TCP transport.  
ICE. Produces `ICE.OLB` - all 19 standard Inter-Client Exchange library modules, with TCP client and server transport.  
SM. Produces `SM.OLB` - all seven standard Session Management library modules, using `ICE.OLB`.  
ICEAUTH. Produces `ICEAUTH.EXE` - the upstream ICE authority-file tool, using `ICE.OLB`; VMS file-version cleanup and permission-inheritance limitations remain.  
MAKESTRS. Produces `MAKESTRS.EXE` - the upstream libXt string-table generator.  
MAKEDEPEND. Produces `MAKEDEPEND.EXE` - all six upstream C dependency-generator modules, emitting `.obj` targets.  
GTF. Produces `GTF.EXE` - the upstream server's Generalized Timing Formula calculator.  
FREETYPE2. Produces `FREETYPE.OLB` - 39 objects from bundled FreeType 2.3.1: the complete 38-object default Unix profile plus retained FTAPI compatibility, with P64 relocation/address-membership changes in `psobjs.c` and a pointer-sized cache-hash conversion in `ftcache.h`.  
MKFONTSCALE. Produces `MKFONTSCALE.EXE` - all four upstream font-index generator modules, using `FONTENC.OLB`, `FREETYPE.OLB`, and `Z.OLB`.  
UCS2ANY. Produces `UCS2ANY.EXE` - the unchanged upstream font-util BDF character-encoding converter; use one mapping per invocation because of an upstream multi-mapping cleanup defect.  
PIXMAN. Produces `PIXMAN.OLB` - all 25 modules of the portable Pixman 0.24.4 profile; native P64 pointers and fixed-width pixels. Assembly, MMX, SIMD, and timing instrumentation excluded.  
DAMAGE. Produces `DAMAGE.OLB` - the unchanged internal server damage-tracking library from `xserver/xorg-server/miext/damage`; static profile with pixmap privates, excluding Render, Composite, and rootless hooks.  
MI. Produces `MI.OLB` - 34 machine-independent server modules plus the retained `miinitext` auxiliary; static core profile with native CRTL cube roots, excluding optional extensions and loadable hooks; pointer-width-safe private-value casts.  
FB. Produces `FB.OLB` - 29 upstream framebuffer modules, with minimal P64 alignment adaptations; static Render-enabled profile including 24-bit and 24/32 conversion paths, using `PIXMAN.OLB` for pixel compositing.  
RENDER. Produces `RENDER.OLB` - all 11 upstream Render extension modules; static profile excluding optional extension hooks, using `PIXMAN.OLB` and the OS library's SHA-1 support.  
DIX. Produces `DIX.OLB` - 32 selected upstream device-independent server modules, plus a native entry adapter for the 64-bit environment-vector boundary; native CRTL supplies `ffs` and `strcasecmp`. Static Render-enabled profile excluding optional extension hooks.  
CONFIG. Produces `CONFIG.OLB` - the unchanged portable server configuration initialization/shutdown module; optional D-Bus, HAL, and udev backends excluded.  
SHADOW. Produces `SHADOW.OLB` - all 20 unchanged upstream shadow-framebuffer modules from `xserver/xorg-server/miext/shadow`; static Render-enabled profile with packed, planar, and rotation paths, excluding loadable hooks.  
SYNC. Produces `SYNC.OLB` - the upstream machine-independent synchronization module required by the server's Sync extension.  
RANDR. Produces `RANDR.OLB` - all 11 selected upstream Resize and Rotate extension modules; static Render-enabled profile excluding Xinerama.  
DBE. Produces `DBE.OLB` - both unchanged upstream Double Buffer Extension modules from `xserver/xorg-server/dbe`; static profile with the machine-independent implementation, excluding loadable hooks.  
RECORD. Produces `RECORD.OLB` - both unchanged upstream protocol-recording extension modules from `xserver/xorg-server/record`; static profile with the range-set implementation, excluding loadable hooks.  
XFIXES. Produces `XFIXES.OLB` - all five unchanged upstream X Fixes extension modules from `xserver/xorg-server/xfixes`; static Render-enabled profile; linking requires an XFIXES-enabled server core.  
COMPOSITE. Produces `COMPOSITE.OLB` - all five upstream Composite extension modules; static Render-enabled profile; linking requires matching COMPOSITE/XFIXES server layouts and Damage extension support.  
CW. Produces `CW.OLB` - all three unchanged upstream Composite wrapper modules from `xserver/xorg-server/miext/cw`; static Render-enabled profile with native C99 assertion-name mapping; requires matching Composite server layouts and screen initialization.  
DAMAGEEXT. Produces `DAMAGE.OLB` in the separate damageext output directory - the unchanged upstream Damage protocol extension module; requires internal Damage tracking and matching server layouts.  
EXT. Produces `EXT.OLB` - all 13 modules of the selected upstream static Xext profile, including Generic Events, ScreenSaver, DPMS, XVideo/XvMC, and X Resource. Remaining optional and loadable extensions excluded. Runtime use requires matching core feature flags and device-specific DPMS/video hooks.  
XINPUT. Produces `XINPUT.OLB` - all 51 upstream server X Input/XI2 extension modules; requires matching server layouts and device-dependent hooks; DDX stubs remain separate.  
OS. Produces `OS.OLB` - all 20 modules of the upstream TCP/XDMCP, MIT-cookie and built-in RGB profile, including the upstream Xtrans wrapper and SHA-1 support using VSI SSL3, with minimal VMS/P64 fixes and one native process adapter; process helpers use DCL and reject images installed with added privileges or rights. Existing display locks require manual verification and stale-lock cleanup on VMS.  
VFB. Produces `VFB.OLB` - all five upstream virtual-framebuffer DDx objects, including generic input and DPMS stubs; malloc framebuffer with Render, excluding SHM and MMAP; not a standalone Xvfb executable.  
FBCMAP. Produces `FBCMAP.OLB` - the unchanged MI-based framebuffer colormap provider used by Xvfb.  
XKB. Produces `XKB.OLB` - all 26 upstream keyboard-extension and generic DDX stub objects, with a native string-function declaration adaptation and explicit keymap include path; requires keymaps and xkbcomp for runtime use.  
XKBFILE. Produces `XKBFILE.OLB` - all 13 unchanged upstream keyboard-file library modules; client profile requiring native Xlib when linked.  
ROOTLESS. Produces `ROOTLESS.OLB` - all five upstream rootless modules, with minimal C99/P64 fixes; generic opaque Render/SHAPE profile without acceleration or XQuartz/Xplugin integration; requires matching SHAPE server layouts and device-dependent frame hooks.  
XEXT. Produces `XEXT.OLB` - all 15 upstream ordinary X Extension client library modules, excluding XShm; one callback const-compatibility cast; requires matching native 64-bit Xlib when linked.  
XRENDER. Produces `XRENDER.OLB` - all 13 unchanged upstream Render client library modules; requires matching native 64-bit Xlib when linked.  
XRANDR. Produces `XRANDR.OLB` - all seven upstream Resize and Rotate client library modules; requires matching native 64-bit Xlib, `XEXT.OLB`, and `XRENDER.OLB` when linked.  
XINERAMA. Produces `XINERAMA.OLB` - the unchanged upstream Xinerama client library module; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
XI. Produces `XI.OLB` - all 56 upstream X Input client library modules, including XI2; requires matching native 64-bit Xlib and `XEXT.OLB` when linked.  
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
DMX. Produces `DMX.OLB` - the unchanged upstream distributed multihead client library module; requires matching native 64-bit Xlib, `XEXT.OLB`, and a DMX server.  
XMUU. Produces `XMUU.OLB` - all five unchanged upstream mini-Xmu library modules, using native snprintf; requires matching native 64-bit Xlib when linked; remains a separate upstream library from Xmu.  
XPM. Produces `XPM.OLB` - all 28 unchanged upstream X PixMap library modules, using native snprintf, upstream string helpers, and plain-file I/O without compressed-file subprocesses; retains upstream 32-bit count fields; requires matching native 64-bit Xlib when linked.  
X11. Produces `X11.OLB` - static Xlib with XKB, Generic Events, and XCB-backed TCP, including P64 Compose-table relocation and built-in extended keysym tables.  
X11_XCB. Produces `X11_XCB.OLB` - the separate upstream libX11-xcb connection and event-queue bridge, built without source changes.  
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

XKB_DATA. Stages 271 files from the release's xkeyboard-config 2.6 data, including three `xorg` rules aliases, with the native compiler; native Perl generates the upstream rules and lists.

XT. Produces `XT.OLB` — complete static Xt, with P64 host types and generated string tables.  
XMU. Produces `XMU.OLB` — complete static Xmu.  
XVFB. Produces `XVFB.EXE` — headless Render/XKB/X Input/XI2 server with Shape, XFixes, Damage, XTest, Sync, XC-MISC, BIG-REQUESTS and Generic Events extensions, linked with VSI SSL3's P64 libcrypto. RandR is built and linked but not initialized by the upstream Xvfb DDx.  
XF86_VIDEO_VMWARE. Produces `VMWARE_DRV.OLB` — all nine objects of the non-DRM profile from xf86-video-vmware 12.0.2, with 64-bit pointers and module metadata; native packing for Xv command structures. The consuming server supplies XFree86 and loader services. Does not link or activate the driver.  
SHADOWFB. Produces `SHADOWFB.OLB` — both unchanged upstream XFree86 shadowfb objects, including module metadata and Render before/after drawing callbacks used by VMware. Separate from `SHADOW`.  
VGAHW. Produces `VGAHW.OLB` — both unchanged upstream XFree86 VGA hardware support objects with 64-bit pointers, including module metadata. The consuming server supplies port I/O and XFree86 services. Does not access hardware during the build.  
RAMDAC. Produces `RAMDAC.OLB` — all eight unchanged upstream XFree86 RAMDAC/cursor objects, including IBM, BT, TI, and the upstream bit-order wrapper; optional ARGB cursors excluded. The consuming server supplies XFree86 and cursor services.  
XAA. Produces `XAA.OLB` — all 52 static XFree86 acceleration objects, including bit-order, fixed-base, triple-bit and polysegment variants; Render enabled, with minimal P64 address-alignment fixes. Loadable hooks, Xinerama and architecture assembly excluded.  
XF86CONFIG. Produces `XF86CONFIG.OLB` — all 17 upstream XFree86 configuration-parser C sources, including InputClass, plus the upstream formatting helpers, with native directory scanning and VMS-only external-name aliases in headers. Reads, validates and writes configuration data; the consuming program supplies diagnostics. Uses current-process file permissions without Unix saved-ID switching; upstream Vendor-subsection Identifier and built-in scanner-reset defects remain.  
COMMON. Produces `COMMON.OLB` - all 28 modules of the selected upstream XFree86 common profile, including server initialization, default-mode generation, VGA-arbiter support, and both XVideo sources; native P64 pointers, XKB, X Input, DGA, RandR, and Render, excluding SPARC bus and debug sources. Linking requires matching core feature flags and native OS services.  
XF86MODES. Produces `XF86MODES.OLB` — all ten unchanged upstream mode-setting modules, including DGA, with 64-bit pointers; the consuming server supplies XFree86, RandR, framebuffer and OS services.  
DDC. Produces `DDC.OLB` — all four unchanged upstream display-identification modules with 64-bit pointers; native bus access remains a consumer input.  
I2C. Produces `I2C.OLB` — the unchanged upstream core I2C bus module with 64-bit pointers; optional multimedia-device libraries are separate.  
LOADER. Produces `LOADER.OLB` - the four ordinary loader objects, with platform-owned static discovery/loading; dynamic image loading and Unix symbol-export tables are excluded.  
PCIACCESS. Produces `PCIACCESS.OLB` — all nine portable libpciaccess common modules with 64-bit pointers, including the VGA-arbitration stub; minimal VMS dispatch/endian adaptations. Linking requires a platform PCI backend.  
BUS. Produces `BUS.OLB` — the unchanged generic XFree86 PCI bus module with 64-bit pointers; the consuming platform supplies PCI access, resource sizes and address translation. Native hardware backends and generic domain/mapping assumptions are not selected.  
MISC. Produces `MISC.OLB` - the unchanged upstream portable `SlowBcopy.c` module; OS services remain consumer inputs.  
STUB. Produces `STUB.OLB` — four unchanged upstream XFree86 OS-support objects: `vidmem`, `VTsw_noop`, `stub_bell`, and `stub_bios`, with 64-bit pointers. Portable subset only; native hardware and OS services remain consumer inputs.  
XORGXKB. Produces `XORGXKB.OLB` — all three unchanged XFree86 keyboard-action modules with 64-bit pointers; the consuming server supplies action dispatch. Replaces, rather than accompanies, the generic XKB DDX stubs at link time.  
XORGFB. Produces `FB.OLB` — the 31-object Xorg framebuffer variant with module metadata and the MI colormap provider, retaining the P64 alignment adaptations. Generic `FB` and `XSERVER_CORE` builds remain separate.  
XF86_INPUT_KEYBOARD. Produces `KBD_DRV.OLB` — the unchanged upstream keyboard driver and AT scan-code helper with 64-bit pointers and module metadata; the consuming server supplies native keyboard services and key mapping.  
XF86_INPUT_MOUSE. Produces `MOUSE_DRV.OLB` — both unchanged upstream mouse driver sources with 64-bit pointers and module metadata; `COMMON.OLB` supplies input buffering, and the consuming server supplies native mouse and serial services.  
MKFONTDIR. Produces the native font-index launcher.  
FONTS. Stages 1,006 BDF fonts and scalable-font/encoding indexes.  
NLS. Builds locale and Compose data for 60 locales.  
X11_DATA. Copies XErrorDB and Xcms.txt.

RUNTIME. Stages BDF fonts/indexes, locale/Xlib/XKB data, and native utilities; no system installation or automatic startup.
  
## Notes  
  
$ @[.VMS-SUPPORT]BUILD HEADERS  
$ @[.VMS-SUPPORT]BUILD XAU  
$ @[.VMS-SUPPORT]BUILD XDMCP  
$ @[.VMS-SUPPORT]BUILD XCB  
$ @[.VMS-SUPPORT]BUILD XCB_SHAPE  
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
$ @[.VMS-SUPPORT]BUILD SHADOW  
$ @[.VMS-SUPPORT]BUILD SYNC  
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
$ @[.VMS-SUPPORT]BUILD VFB  
$ @[.VMS-SUPPORT]BUILD FBCMAP  
$ @[.VMS-SUPPORT]BUILD XKB  
$ @[.VMS-SUPPORT]BUILD XKBFILE  
$ @[.VMS-SUPPORT]BUILD ROOTLESS  
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
$ @[.VMS-SUPPORT]BUILD DMX  
$ @[.VMS-SUPPORT]BUILD XMUU  
$ @[.VMS-SUPPORT]BUILD XPM  
$ @[.VMS-SUPPORT]BUILD X11  
$ @[.VMS-SUPPORT]BUILD X11_XCB  
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
$ @[.VMS-SUPPORT]BUILD COMMON  
$ @[.VMS-SUPPORT]BUILD XF86MODES  
$ @[.VMS-SUPPORT]BUILD DDC  
$ @[.VMS-SUPPORT]BUILD I2C  
$ @[.VMS-SUPPORT]BUILD LOADER  
$ @[.VMS-SUPPORT]BUILD PCIACCESS  
$ @[.VMS-SUPPORT]BUILD BUS  
$ @[.VMS-SUPPORT]BUILD MISC  
$ @[.VMS-SUPPORT]BUILD STUB  
$ @[.VMS-SUPPORT]BUILD XORGXKB  
$ @[.VMS-SUPPORT]BUILD XORGFB  
$ @[.VMS-SUPPORT]BUILD XF86_INPUT_KEYBOARD  
$ @[.VMS-SUPPORT]BUILD XF86_INPUT_MOUSE  
$ @[.VMS-SUPPORT]BUILD MKFONTDIR  
$ @[.VMS-SUPPORT]BUILD FONTS  
$ @[.VMS-SUPPORT]BUILD NLS  
$ @[.VMS-SUPPORT]BUILD X11_DATA  
  
Native x86-64 X11 libraries and servers with 64-bit pointers; the standalone GAWK build tool uses 32-bit pointers. Requires VSI C, DECset MMS, native Python 3 (`PYTHON$ROOT`) for XCB protocol generation, native Perl for keyboard rules, and VSI SSL3 headers/P64 libcrypto.  
Build from the repository root with `@[.VMS-SUPPORT]BUILD`.  
COMMON builds bundled GNU Gawk 5.4.1 automatically to run the upstream default-mode generator.  
XKBCOMP builds bundled GNU Bison 1.35 automatically; no separate download or BISON command setup is required.  
