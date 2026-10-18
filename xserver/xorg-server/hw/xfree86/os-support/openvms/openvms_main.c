/* Match DIX ENTRY: /POINTER_SIZE=64=ARGV /MAIN=POSIX_EXIT.
 * The compiler supplies P64 argv, while envp retains its native P32 layout. */
#include <stdio.h>
#include <stdlib.h>
#include "openvms_dlopen.h"

#ifdef __VMS
#pragma pointer_size save
#pragma pointer_size 32
#endif
typedef char *VmsXorgNativeString;
typedef VmsXorgNativeString *VmsXorgNativeVector;
#ifdef __VMS
#pragma pointer_size restore
#endif

extern int XorgNativeEntry(int, char **, VmsXorgNativeVector);

int main(int argc, char **argv, char **envp)
{
    const char *core_image = getenv("XORG_CORE");

    if (core_image == NULL || *core_image == '\0') {
        fputs("Xorg: XORG_CORE is not defined\n", stderr);
        return EXIT_FAILURE;
    }
    if (VmsXorgSetCoreImage(core_image) != 0) {
        const char *error = VmsXorgDlerror();
        fprintf(stderr, "Xorg: cannot register XORG_CORE: %s\n",
                error != NULL ? error : "unknown loader error");
        return EXIT_FAILURE;
    }
    return XorgNativeEntry(argc, argv, envp);
}
