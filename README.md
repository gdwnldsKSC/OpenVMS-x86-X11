# OpenVMS x86 X11 R7.7 Port Effort  
  
Currently, this is the complete modular X11 R7.7 source baseline with selected individual-component updates and the bundled dependencies required by the native build.  
  
As parts become buildable, they will be updated and documented here.  
  
## Buildable parts  

The R7.7 build baseline and selected individual-component updates were validated natively on OpenVMS x86-64 E9.2-4.  
Xvfb 21.1.25 passed authenticated TCP, unauthenticated-client rejection, RandR, XI2, Sync fences, core drawing, font glyphs and Render checks with Pixman 0.46.4 and libXfont2 2.0.9.  
VMware 13.4.0 passed native loadable-module activation, video initialization and graceful teardown on the development VM.  
Keyboard 2.1.0 and mouse 2.0.0 have loadable-module support with unchanged upstream drivers and original OpenVMS PS/2 acquisition support.  
Supporting builds include libxcvt 0.1.3, libXfont 1.5.4, xorgproto 2026.1 and xtrans 1.6.0. X11 uses 64-bit pointers; the standalone GAWK build tool uses 32-bit pointers.  
The X11 compiler profile preserves external-name case. When changing this profile, use a clean build tree and rebuild all linked objects and libraries together.  
  
XAU - 1.0.12. Produces `XAU.OLB` - all eight upstream Xau modules with native P64 pointers.  
XDMCP. Produces `XDMCP.OLB` - all eight standard Xdmcp modules; optional `HASXDMAUTH` wrapping excluded.  
XCB. Produces `XCB.OLB` - the core TCP client transport and generated core, BIG-REQUESTS, and XC-MISC protocols, using native pthreads and `XAU.OLB`.  
XCB_SHAPE. Produces `XCB_SHAPE.OLB` - the generated Shape protocol client library used by xwininfo.  
ZLIB. Produces `Z.OLB` - all 12 modules of the bundled zlib 1.2.3.  
REGEX. Produces `REGEX.OLB` - all four unchanged standalone POSIX regex modules, with native public/private header generation; P64 case-preserving build and module-discovery matching validated.  
FONTENC. Produces `FONTENC.OLB` - `fontenc.c` and `encparse.c`, using `Z.OLB`; upstream reverse-map cleanup and parser-growth defects remain.  
EXPAT. Produces `EXPAT.OLB` - `xmlparse.c`, `xmltok.c`, and `xmlrole.c` from bundled Expat 1.95.8.  
XFONT - 1.5.4. Produces `XFONT.OLB` - 47 objects across bitmap, fontfile, fc, util, builtins, and FreeType; gzip and TCP font-server transport included, with native text reads and regular-file/identity checks. Builds `Z.OLB`, `FONTENC.OLB`, and `FREETYPE.OLB` dependencies; optional fontencc excluded. Builtin fonts require server-side activation.  
XFONT2 - 2.0.9. Produces `XFONT2.OLB` - all 51 objects of the current server font-library profile with bitmap, builtin, FreeType, gzip and TCP font-server support; native font/glyph, font-index and symlink-rejection checks passed. Retains `XFONT` separately for existing standalone tools.  
FNTSTUBS - 1.5.4. Produces `FNTSTUBS.OLB` - all 16 unchanged libXfont stub modules, for standalone font tools.  
FS. Produces `FS.OLB` - all 25 standard font-server client library modules, with TCP transport.  
ICE. Produces `ICE.OLB` - all 19 standard Inter-Client Exchange library modules, with TCP client and server transport.  
SM. Produces `SM.OLB` - all seven standard Session Management library modules, using `ICE.OLB`.  
ICEAUTH. Produces `ICEAUTH.EXE` - the upstream ICE authority-file tool, using `ICE.OLB`; VMS file-version cleanup and permission-inheritance limitations remain.  
MAKESTRS. Produces `MAKESTRS.EXE` - the upstream libXt string-table generator.  
MAKEDEPEND. Produces `MAKEDEPEND.EXE` - all six upstream C dependency-generator modules, emitting `.obj` targets.  
GTF - 21.1.25. Produces `GTF.EXE` - the upstream server's Generalized Timing Formula calculator.  
XCVT - 0.1.3. Produces `XCVT.OLB` - the unchanged upstream CVT timing generator, with native P64 pointers.  
FREETYPE2. Produces `FREETYPE.OLB` - 39 objects from bundled FreeType 2.3.1: the complete 38-object default Unix profile plus retained FTAPI compatibility, with P64 relocation/address-membership changes in `psobjs.c` and a pointer-sized cache-hash conversion in `ftcache.h`.  
MKFONTSCALE. Produces `MKFONTSCALE.EXE` - all four upstream font-index generator modules, using `FONTENC.OLB`, `FREETYPE.OLB`, and `Z.OLB`.  
UCS2ANY. Produces `UCS2ANY.EXE` - the unchanged upstream font-util BDF character-encoding converter; use one mapping per invocation because of an upstream multi-mapping cleanup defect.  
PIXMAN - 0.46.4. Produces `PIXMAN.OLB` - all 32 unchanged upstream modules of the portable Pixman profile; native P64 pointers and fixed-width pixels. Assembly, MMX, SIMD, and timing instrumentation excluded. Only the floating-point combiner requires optimization disabled; all other modules remain optimized.  
DAMAGE - 21.1.25. Produces `DAMAGE.OLB` - the unchanged internal server damage-tracking library from `xserver/xorg-server/miext/damage`; static profile with pixmap privates, excluding Render, Composite, and rootless hooks.  
MI - 21.1.25. Produces `MI.OLB` - 28 machine-independent server modules plus the retained `miinitext` auxiliary; static core profile with native CRTL cube roots, excluding optional extensions and loadable hooks.  
FB - 21.1.25. Produces `FB.OLB` - all 27 upstream framebuffer objects, including the MI colormap provider; static Render-enabled profile using `PIXMAN.OLB` for pixel compositing.  
RENDER - 21.1.25. Produces `RENDER.OLB` - all 11 upstream Render extension modules; static profile excluding optional extension hooks, using `PIXMAN.OLB` and the OS library's SHA-1 support.  
DIX - 21.1.25. Produces `DIX.OLB` - 33 upstream device-independent server modules plus a native entry adapter for the 64-bit environment-vector boundary; static Render-enabled profile excluding optional extension hooks.  
CONFIG - 21.1.25. Produces `CONFIG.OLB` - the unchanged portable server configuration initialization/shutdown module; optional D-Bus, HAL, and udev backends excluded.  
SHADOW - 21.1.25. Produces `SHADOW.OLB` - all 24 upstream shadow-framebuffer modules; packed, planar, 24/32 conversion and rotation paths, with a pointer-width-safe alignment check.  
SYNC - 21.1.25. Produces `SYNC.OLB` - both upstream machine-independent synchronization objects; shared-memory fence support excluded.  
RANDR - 21.1.25. Produces `RANDR.OLB` - all 15 selected upstream Resize and Rotate extension modules; static Render-enabled profile excluding Xinerama.  
DBE - 21.1.25. Produces `DBE.OLB` - both unchanged upstream Double Buffer Extension modules from `xserver/xorg-server/dbe`; static profile with the machine-independent implementation, excluding loadable hooks.  
RECORD - 21.1.25. Produces `RECORD.OLB` - both unchanged upstream protocol-recording extension modules from `xserver/xorg-server/record`; static profile with the range-set implementation, excluding loadable hooks.  
XFIXES - 21.1.25. Produces `XFIXES.OLB` - all six unchanged upstream X Fixes extension modules; linking requires an XFIXES-enabled server core.  
COMPOSITE - 21.1.25. Produces `COMPOSITE.OLB` - all five upstream Composite extension modules; static Render-enabled profile; linking requires matching COMPOSITE/XFIXES server layouts and Damage extension support.  
DAMAGEEXT - 21.1.25. Produces `DAMAGE.OLB` in the separate damageext output directory - the unchanged upstream Damage protocol extension module; requires internal Damage tracking and matching server layouts.  
EXT - 21.1.25. Produces `EXT.OLB` - all 14 modules of the selected upstream static Xext profile, including Generic Events, ScreenSaver, DPMS, XVideo/XvMC, X Resource and the shared hash table. Runtime use requires matching core feature flags and device-specific DPMS/video hooks.  
XINPUT - 21.1.25. Produces `XINPUT.OLB` - all 52 upstream server X Input/XI2 extension modules; requires matching server layouts and device-dependent hooks; DDX stubs remain separate.  
OS - 21.1.25. Produces `OS.OLB` - 24 upstream TCP/XDMCP, MIT-cookie, polling and utility modules plus one native process adapter; built-in RGB and VSI SSL3 cryptography, with minimal VMS/P64 fixes. Process helpers use DCL and reject images installed with added privileges or rights. Existing display locks require manual verification and stale-lock cleanup on VMS.  
VFB - 21.1.25. Produces `VFB.OLB` - all four upstream virtual-framebuffer DDX objects, including generic input stubs; malloc framebuffer with Render and RandR, excluding SHM and MMAP; not a standalone Xvfb executable.  
XKB - 21.1.25. Produces `XKB.OLB` - all 25 upstream keyboard-extension and generic DDX stub objects, with a native string-function declaration adaptation and explicit keymap include path; requires keymaps and xkbcomp for runtime use.  
XKBFILE. Produces `XKBFILE.OLB` - all 13 unchanged upstream keyboard-file library modules; client profile requiring native Xlib when linked.  
ROOTLESS - 21.1.25. Produces `ROOTLESS.OLB` - all five upstream rootless modules, with minimal C99/P64 fixes; generic opaque Render/SHAPE profile without acceleration or XQuartz/Xplugin integration; requires matching SHAPE server layouts and device-dependent frame hooks.  
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
XVFB - 21.1.25. Produces `XVFB.EXE` - headless Render/XKB/X Input/XI2 server with RandR, Shape, XFixes, Damage, XTest, Sync, XC-MISC, BIG-REQUESTS and Generic Events extensions, linked with VSI SSL3's P64 libcrypto.  
XORG - 21.1.25. Builds the native server as `XORG.EXE` and `XORG_SHR.EXE`, with `vmware_drv.so`, `libvgahw.so`, `libshadowfb.so`, `kbd_drv.so`, and `mouse_drv.so` loadable images. Uses the upstream module loader with native image support and a separate P64_XORG build profile; builds its XKB data/compiler dependencies automatically. Optional native PS/2 acquisition supplies the unchanged upstream input drivers, with mouse acceleration disabled in `INPUT.CONF` and explicit 1:1 pointer defaults in `RUN_XORG.COM`. Includes original P64 USB boot-keyboard/mouse decoding, bounded input polling and a public-interface UG transport; XKB and the upstream mouse driver retain event processing. Native device-free decoder, bridge and UG service-mock tests pass; the server and its loadable modules relink with USB support. Live USB remains unavailable until a working OS USB stack and its verified completion-status contract are supplied; no USB drivers are included or enabled. Native module activation, authenticated TCP, unauthenticated-client rejection, core drawing, glyphs and Render checks pass; fixed-speed pointer movement was confirmed, while full physical keyboard/CDE validation remains incomplete. No installation or boot changes.  
XORG_MODULES. Builds the same seven-image set together, keeping the module imports matched to the shared core.  
XF86_VIDEO_VMWARE - 13.4.0. Produces `VMWARE_DRV.OLB` — all nine objects of the non-DRM profile from xf86-video-vmware, with 64-bit pointers and module metadata; native packing for Xv command structures. The consuming server supplies XFree86 and loader services. Does not link or activate the driver.  
SHADOWFB - 21.1.25. Produces `SHADOWFB.OLB` — both unchanged upstream XFree86 shadowfb objects, including module metadata and Render before/after drawing callbacks used by VMware. Separate from `SHADOW`.  
VGAHW - 21.1.25. Produces `VGAHW.OLB` — both unchanged upstream XFree86 VGA hardware support objects with 64-bit pointers, including module metadata. The consuming server supplies port I/O and XFree86 services. Does not access hardware during the build.  
RAMDAC - 21.1.25. Produces `RAMDAC.OLB` - both unchanged upstream XFree86 cursor objects; the consuming server supplies XFree86 and cursor services.  
XF86CONFIG - 21.1.25. Produces `XF86CONFIG.OLB` - all 18 upstream internal XFree86 configuration-parser modules, including InputClass and OutputClass, with native directory scanning. Reads, validates and writes configuration data; the consuming server supplies diagnostics and allocation/formatting helpers. Uses current-process file permissions without Unix saved-ID switching.  
COMMON - 21.1.25. Produces `COMMON.OLB` - all 29 modules of the selected upstream XFree86 common profile, including server initialization, default-mode generation, VGA-arbiter support, output classification and XVideo; native P64 pointers, XKB, X Input, DGA, RandR and Render. Linking requires matching core feature flags and native OS services.  
XF86MODES - 21.1.25. Produces `XF86MODES.OLB` - all eight upstream mode-setting modules, including DGA, with 64-bit pointers and `XCVT.OLB`; the consuming server supplies XFree86, RandR, framebuffer and OS services.  
DDC - 21.1.25. Produces `DDC.OLB` — all four unchanged upstream display-identification modules with 64-bit pointers; native bus access remains a consumer input.  
I2C - 21.1.25. Produces `I2C.OLB` — the unchanged upstream core I2C bus module with 64-bit pointers; optional multimedia-device libraries are separate.  
LOADER - 21.1.25. Produces `LOADER.OLB` - both upstream loader objects, with filesystem module discovery and dynamic-loading entry points. The consuming server supplies image activation, global symbol lookup and exported symbols.  
PCIACCESS - 0.19. Produces `PCIACCESS.OLB` — nine portable common modules plus the native OpenVMS x86-64 backend and port-I/O leaf, with 64-bit pointers and the VGA-arbitration stub. Enumeration/configuration reads remain read-only by default. Explicit selection and exclusive ownership of a VMware SVGA2 device enable bounded BAR probing, mappings, port I/O and restricted configuration writes; only PCI segment zero is supported and ROM execution is unavailable. Requires installed OS headers, `LINK/SYSEXE` and the corresponding native privileges. The video paths were exercised through Xorg initialization and graceful teardown on the development VMware SVGA2 VM. Define `PCIACCESS_EXTERNAL_BACKEND` for a consumer-supplied provider. OpenVMS uses the native MMS mixed CC/clang build; Meson deliberately rejects that unsupported toolchain combination.  
BUS - 21.1.25. Produces `BUS.OLB` - both objects of the upstream generic XFree86 bus library; the consuming platform supplies PCI access, resource sizes and address translation.  
MISC - 21.1.25. Produces `MISC.OLB` - the unchanged upstream portable `SlowBcopy.c` module; OS services remain consumer inputs.  
STUB - 21.1.25. Produces `STUB.OLB` - four unchanged upstream XFree86 OS-support objects: `vidmem`, `VTsw_noop`, `stub_bell`, and `pm_noop`, with 64-bit pointers. Native hardware and OS services remain consumer inputs.  
XORGXKB - 21.1.25. Produces `XORGXKB.OLB` — all three unchanged XFree86 keyboard-action modules with 64-bit pointers; the consuming server supplies action dispatch. Replaces, rather than accompanies, the generic XKB DDX stubs at link time.  
XF86_INPUT_KEYBOARD - 2.1.0. Produces `KBD_DRV.OLB` — the unchanged upstream keyboard driver and AT scan-code helper with 64-bit pointers and module metadata; the consuming server supplies native keyboard services and key mapping.  
XF86_INPUT_MOUSE - 2.0.0. Produces `MOUSE_DRV.OLB` — both unchanged upstream mouse driver sources with 64-bit pointers and module metadata; `COMMON.OLB` supplies input buffering, and the consuming server supplies native mouse and serial services.  
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
$ @[.VMS-SUPPORT]BUILD REGEX  
$ @[.VMS-SUPPORT]BUILD FONTENC  
$ @[.VMS-SUPPORT]BUILD EXPAT  
$ @[.VMS-SUPPORT]BUILD XFONT  
$ @[.VMS-SUPPORT]BUILD XFONT2  
$ @[.VMS-SUPPORT]BUILD FNTSTUBS  
$ @[.VMS-SUPPORT]BUILD FS  
$ @[.VMS-SUPPORT]BUILD ICE  
$ @[.VMS-SUPPORT]BUILD SM  
$ @[.VMS-SUPPORT]BUILD ICEAUTH  
$ @[.VMS-SUPPORT]BUILD MAKESTRS  
$ @[.VMS-SUPPORT]BUILD MAKEDEPEND  
$ @[.VMS-SUPPORT]BUILD GTF  
$ @[.VMS-SUPPORT]BUILD XCVT  
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
$ @[.VMS-SUPPORT]BUILD DAMAGEEXT  
$ @[.VMS-SUPPORT]BUILD EXT  
$ @[.VMS-SUPPORT]BUILD XINPUT  
$ @[.VMS-SUPPORT]BUILD OS  
$ @[.VMS-SUPPORT]BUILD VFB  
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
$ @[.VMS-SUPPORT]BUILD XORG  
$ @[.VMS-SUPPORT]BUILD XORG_MODULES  
$ @[.VMS-SUPPORT]BUILD XF86_VIDEO_VMWARE  
$ @[.VMS-SUPPORT]BUILD SHADOWFB  
$ @[.VMS-SUPPORT]BUILD VGAHW  
$ @[.VMS-SUPPORT]BUILD RAMDAC  
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
XORG also requires VSI clang for the small port-I/O leaves, compiled with the kernel-stack red zone disabled. The native MMS build generates its own image exports and links all selected video and input modules; no host-side generator is needed.  
For a foreground display-only test, run `@[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]RUN_XORG domain:bus:device.function SERIAL_CONSOLE` from SSH or serial console. The PCI address is hexadecimal. This requires CMKRNL, PFNMAP and SYSLCK, serial OPA0, and exclusive use of the selected VMware SVGA2 display; stop any existing graphics server first. It shows the root window without X listeners or physical input. Append `0 I8042` to select display 0 with PS/2 keyboard and mouse, additionally reserving the i8042 controller exclusively. Omitting `I8042` keeps video-only mode. Neither mode enables X listeners by default.  
The USB source is built by the same `BUILD XORG` invocation and uses the installed `SYS$COMMON:[SYSHLP.EXAMPLES.USB]UGDEF.H`, not a bundled private header. For a future device-ready build, `0 USB` selects explicit process logicals `XORG_OPENVMS_USB_KEYBOARD` and/or `XORG_OPENVMS_USB_MOUSE`, each naming an exclusively available `UGAunit:` device. The current transport refuses live acquisition unless `OPENVMS_UG_USB_SUCCESS` is defined at compile time from a verified driver/SDK contract; the public manual does not specify that value. The initial profile handles boot keyboards and three-button relative mice, not wheel/consumer reports or automatic hotplug. It does not fall back to PS/2 or load/configure USB drivers.  
Authenticated startup passed native client checks and launched the installed DECwindows greeter; fixed-speed pointer movement was confirmed, while full physical keyboard/CDE validation remains incomplete. After building XAU, XORG and RUNTIME, build the standalone read-only authority checker with `@[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]BUILD_XORG_AUTH`. Append `AUTHENTICATED device:[private]server.auth;version device:[repo.VMS-SUPPORT._BUILD.P64.RUNTIME]` after RUN_XORG's display and input arguments. Supply an existing current-version, SYSTEM-owned MIT-MAGIC-COOKIE-1 authority file for that display, with no group/world access or ACL. This mode requires SYSTEM with SECURITY, CMKRNL, PFNMAP and SYSLCK already enabled and uses ordinary upstream TCP listeners, not a loopback-only listener. It does not generate or repair authority files.  
After confirming an authenticated client connects and a client without the cookie is rejected, use a fresh dedicated SYSTEM control login to run `@[.VMS-SUPPORT.PROGRAMS.XSERVER.HW.XFREE86.OPENVMS]START_DECWINDOWS display device:[private]server.auth;version device:[private]fresh.log`. This requires installed DECwindows, existing CMEXEC/SYSNAM/SECURITY privileges, and a protected log directory. Keep that control login open while its greeter/desktop children run; its previously absent job display/login logicals remain defined until the printed cleanup commands are run after all those children exit. No system startup or installation files are changed.  
