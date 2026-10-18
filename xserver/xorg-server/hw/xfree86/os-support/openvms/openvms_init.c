/* Original OpenVMS Xorg video-only platform support.
 * The operator must reserve the display and use serial OPA0. libpciaccess
 * owns the device lease and bounds every privileged hardware operation.
 */
#include <stdlib.h>
#include <string.h>
#include <pciaccess.h>
#include "xf86.h"
#include "xf86Priv.h"
#define XF86_OS_PRIVS
#include "xf86_OSproc.h"
#include "opaque.h"

static int console_open;
static int cleanup_registered;
extern void OpenVMSClosePortIO(void);

void
xf86OpenConsole(void)
{
    const char *exclusive = getenv("PCIACCESS_OPENVMS_EXCLUSIVE");
    if (!exclusive || strcmp(exclusive, "1"))
        FatalError("OpenVMS Xorg requires exclusive video ownership and serial OPA0\n");
    if (console_open)
        return;
    console_open = 1;
    xf86Info.consoleFd = -1;
    xf86Info.vtno = -1;
}

void
xf86CloseConsole(void)
{
    OpenVMSClosePortIO();
    pci_system_cleanup();
    console_open = 0;
}

Bool xf86EnableIO(void) { return TRUE; }
void xf86DisableIO(void) { OpenVMSClosePortIO(); }
void
osPciInit(void)
{
    /* PCI probing precedes console opening; clean up early startup failures. */
    if (!cleanup_registered) {
        if (atexit(xf86CloseConsole))
            FatalError("OpenVMS Xorg cannot register PCI cleanup\n");
        cleanup_registered = 1;
    }
}

int
xf86ProcessArgument(int argc, char **argv, int i)
{
    (void)argc;
    if (!strcmp(argv[i], "-displayonly")) {
        /* Upstream's no-listener path: a visible root window without exposing
         * any X connection service. Normal authenticated clients can be used
         * by omitting this option and supplying the usual -auth argument. */
        NoListenAll = TRUE;
        return 1;
    }
    return 0;
}

void
xf86UseMsg(void)
{
    ErrorF("-displayonly          display the root window without X listeners\n");
}
