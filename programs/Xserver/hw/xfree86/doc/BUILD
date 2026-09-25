       Building the X Window System from the X.org Source Distribution

                Jim Gettys and Keith Packard (for X11R6.8.2)
            David Dawes and Matthieu Herrb (for XFree86 4.4 RC2)

                               9 February 2005

                                  Abstract

     This document describes how to build the X Window System from the
     X.Org monolithic source distribution and is designed to be used in
     conjunction with the operating system (OS) specific README files.

     NOTE:  Refer to the appropriate OS-specific README file before
     attempting to build the X distribution.  These files often contain
     additional information that you need to successfully build for your
     OS.

We highly recommend using gcc to build the X distribution, but X also gener-
ally builds with the native compiler for each OS platform; The build tools
known to be required include: gcc, make, C library development package, bi-
son, flex, zlib (development package), ncurses (development package), font-
config (development package), expat (development package), and Perl.

1.  How to get the X11R6.8.2 distribution source

One way of getting the X11R6.8.2 source is to obtain it directly from the
X.Org CVS repository.  There are several ways of doing that, and they are
described in the CVS section of our wiki <URL:http://wiki.x.org> The CVS tag
for this release is "XORG-6_8_2".   The tag for the maintenance branch for
this release is "XORG-6_8-branch".

Another method of getting the X11R6.8.2 source is to either download the
6.8.2 source tarballs sites from freedesktop.org using either ftp or http.
The procedure for this is as follows:

   o The X11R6.8.2 source is contained in the files:

          X11R6.8.2-src1.tar.gz

          X11R6.8.2-src2.tar.gz

          X11R6.8.2-src3.tar.gz

          X11R6.8.2-src4.tar.gz

          X11R6.8.2-src5.tar.gz

          X11R6.8.2-src6.tar.gz

          X11R6.8.2-src7.tar.gz

     These can be found at ftp://ftp.freedesktop.org/xorg/X11R6.8.2/src/ or
     http://freedesktop.org/~xorg/X11R6.8.2/src/     and similar locations on
     X.org mirror sites.  X11R6.8.2-src4.tgz and X11R6.8.2-src5.tar.gz con-
     tains the fonts.  X11R6.8.2-src6.tar.gz contains the documentation
     source.  X11R6.8.2-src7.tar.gz contains the hardcopy documentation.
     X11R6.8.2-src1.tar.gz, X11R6.8.2-src2.tar.gz and X11R6.8.2-src3.tar.gz
     contains everything else.  If you don't need the docs or fonts you can
     get by with only X11R6.8.2-src1.tar.gz, X11R6.8.2-src2.tar.gz and
     X11R6.8.2-src3.tar.gz.

   o Extract each of these files by running the following from a directory on
     a filesystem containing enough space (the full source requires around
     305MB, and a similar amount is required in addition to this for the com-
     piled binaries):

          gzip -d < X11R6.8.2-src1.tar.gz | tar vxf -

          gzip -d < X11R6.8.2-src2.tar.gz | tar vxf -

          gzip -d < X11R6.8.2-src3.tar.gz | tar vxf -

          gzip -d < X11R6.8.2-src4.tar.gz | tar vxf -

          gzip -d < X11R6.8.2-src5.tar.gz | tar vxf -

          gzip -d < X11R6.8.2-src6.tar.gz | tar vxf -

          gzip -d < X11R6.8.2-src7.tar.gz | tar vxf -

All methods will produce one main source directory called xc.

2.  Configuring the source before building

In most cases it shouldn't be necessary to configure anything before build-
ing.

If you do want to make configuration changes, it is recommended that you
start by going to the xc/config/cf directory, and copying the file
xorgsite.def to host.def.  Then read through the host.def file (which is
heavily commented), and set your configuration parameters. Usually you can
find the default settings by checking the .cf file(s) relevant to your OS.

A good rule to follow is only to change things that you understand as it's
easy to create build problems by changing the default configuration.  Check
the configuration parameters specified in the xc/config/cf/README.

If you are using just the X11R6.8.2-src1.tar.gz, X11R6.8.2-src2.tar.gz and
X11R6.8.2-src3.tar.gz parts of the source dist, you will need to define
BuildFonts to NO.

3.  Using a shadow directory of symbolic links for the build

A recommended practice is to use a shadow directory of symbolic links to do
the build of X11R6.8.2 as this allows you to keep the source directory unmod-
ified during the build.  It has the following benefits:

   o When you are using CVS to maintain your source tree, the update process
     is not disturbed by foreign files not under CVS's control.

   o It is possible to build X11R6.8.2 for several different Operating System
     or architectures from the same sources, shared by read-only NFS mounts.

   o It is possible to build X11R6.8.2 with different configuration options,
     by putting a real copy of the host.def file in each build tree and by
     customizing it separately in each build tree.

To make a shadow directory of symbolic links, use the following steps:

   o create the directory at the top of the build tree. It is often created
     at the same level that the xc directory, but this is not mandatory.

          cd the directory containing the xcdirectory

          mkdir build

   o use the "lndir" command to make the shadow tree:

          lndir ../xc

     Note that you can refer to the xc directory with an absolute path if
     needed.

     See the lndir(1) manual page for details.

If lndir is not already installed on your system, you can build it manually
from the X11R6.8.2 sources by running the following commands:

     cd xc/config/util

     make -f Makefile.ini lndir

     cp lndir some directory in your PATH

Occasionally there may be stale links in the build tree, like when files in
the source tree are removed or renamed.  These can be cleaned up by running
the "cleanlinks" script from the build directory (see the cleanlinks(1) man-
ual page).  Rarely there will be changes that will require the build tree to
be re-created from scratch.  A symptom of this can be mysterious build prob-
lems.  The best solution for this is to remove the build tree, and then re-
create it using the steps outlined above.

4.  Building and installing the distribution

Before building the distribution, read through the OS-specific README file in
xc/programs/Xserver/hw/xfree86/doc that is relevant to you.  Once you have
addressed the OS-specific details, go your build directory (either the xc
directory or the shadow tree created before) and run "make World" with the
BOOTSTRAPCFLAGS set as described in the OS-specific README (if necessary, but
most systems supported by X11R6.8.2 don't need BOOTSTRAPCFLAGS).  It is
advisable to redirect stdout and stderr to World.Log so that you can track
down problems that might occur during the build.

With Bourne-like shells (Bash, the Korn shell, zsh, etc.) use a command like:

     make World > World.log 2>&1

Witch C-shell variants (csh, tcsh, etc), use:

     make World >& World.log

You can follow the progress of the build by running:

     tail -f World.log

 in a terminal.

When the build is finished, you should check the World.Log file to see if
there were any problems.  If there weren't any then you can install the bina-
ries.  By default the "make World" process will exit at the first error.  To
restart the build process after correcting the problems, just run 'make'.  If
Imakefiles or part of the build configuration was changed as part of correct-
ing the problem, either re-run "make World", or run "make Everything".

If you would prefer "make World" to ignore errors and build as much as possi-
ble, run it in the following way instead of the way described above:

for Bourne-like shells:

     make WORLDOPTS=-k World > World.log 2>&1

for C-shell variants:

     make WORLDOPTS=-k World >& World.log

To do the install, run "make install" and "make install.man".  Make sure you
have enough space in /usr/X11R6 for the install to succeed.  If you want to
install on a filesystem other than /usr, make a symbolic link to /usr/X11R6
before installing.  To install the tree into a different directory than
/usr/X11R6 you can specify DESTDIR:

     make install DESTDIR=<install_target_dir> make install.man DEST-
     DIR=<install_target_dir>

Cross compiling is supported if the appropriate config files for your target
platforms exist. You must have the compiler toolchain installed for your tar-
get platform and the C-compiler must know where those tools exist. To inform
the build system where your cross compiler is located use BOOTSTRAPCFLAGS to
set the make variable CROSSCOMPILEDIR.

     make World BOOTSTRAPCFLAGS="CROSSCOMPILEDIR=<cross compiler dir>";

5.  Reconfiguring the server (source distribution)

To build a different set of servers or servers with a different set of
drivers installed:

  1.  Make sure the source for any new drivers is in the correct place (e.g.,
      driver source should be in a subdirectory of xc/pro-
      grams/Xserver/hw/xfree86/drivers).

  2.  Change the settings of the server defines in host.def to specify which
      servers you wish to build.  Also, change the driver lists to suit your
      needs.

  3.  From xc/programs/Xserver, run:

                make Makefile
                make Makefiles
                make includes
                make depend
                make

6.  Other useful make targets

There are some other useful targets defined in the top level Makefile of
X11R6.8.2:

   o Everything after a make World, make Everything does everything a make
     World does, except the cleaning of the tree. It is a way to quickly
     rebuild the tree after a source patch, but it is not 100% bullet proof.
     There are cases were it is better to force a full build by using make
     World.

   o clean does a partial cleaning of the source tree. Removes object files
     and generated manual pages, but leaves the Makefiles and the generated
     dependencies files in place. After a make clean you need to re-run

                  make includes
                  make depend
                  make

     to rebuild the X11R6.8.2.

   o distclean does a full cleaning of the source tree, removing all gener-
     ated files. After a make distclean, make World is the only option to
     rebuild X11R6.8.2.

   o includes generates all generated header files and in-tree symbolic links
     needed by the build. These files are removed by a make clean.

   o depend recomputes the dependencies for the various targets in all Make-
     files. Depending on the operating system, the dependencies are stored in
     the Makefile, or as a separate file, called .depend. This target needs
     the generated include files produced by make includes.

   o VerifyOS displays the detected operating system version. If the numbers
     shown do not match your system, you probably need to set them manually
     in host.def and report the problem to <xorg@freedesktop.org>.

     $Id$


$XdotOrg: xc/programs/Xserver/hw/xfree86/doc/BUILD,v 1.4.2.3 2005/02/08 00:44:26 kem Exp $
