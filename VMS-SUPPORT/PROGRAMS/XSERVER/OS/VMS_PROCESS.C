/* Original OpenVMS process helpers using the public C RTL and system services. */

#if !defined(__VMS)
#error "VMS_PROCESS requires OpenVMS"
#endif
#if !defined(__INITIAL_POINTER_SIZE) || __INITIAL_POINTER_SIZE != 64
#error "VMS_PROCESS requires /POINTER_SIZE=64"
#endif
#ifdef _VMS_WAIT
#error "VMS_PROCESS requires POSIX wait status, not _VMS_WAIT"
#endif
#ifndef _POSIX_EXIT
#define _POSIX_EXIT 1
#endif
#ifndef __NEW_STARLET
#define __NEW_STARLET 1
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <jpidef.h>
#include <iosbdef.h>
#include <starlet.h>
#include "os.h"

#pragma __required_pointer_size __save
#pragma __required_pointer_size 32
typedef struct {
    unsigned short length;
    unsigned short code;
    void *buffer;
    unsigned short *returned;
} VmsProcessItem;
#pragma __required_pointer_size __restore

typedef char VmsProcessItemSize[(sizeof(VmsProcessItem) == 12) ? 1 : -1];
typedef char VmsProcessIosbSize[(sizeof(IOSB) == 8) ? 1 : -1];

static int
VmsProcessAllowed(void)
{
    unsigned int image_privileges[2] = {0, 0};
    unsigned int image_rights[2] = {0, 0};
    unsigned short privileges_length = 0;
    unsigned short rights_length = 0;
    VmsProcessItem items[3] = {
        {sizeof(image_privileges), JPI$_IMAGPRIV,
         image_privileges, &privileges_length},
        {sizeof(image_rights), JPI$_IMAGE_RIGHTS,
         image_rights, &rights_length},
        {0, 0, 0, 0}
    };
    IOSB iosb = {0};
    unsigned int status;

    if (getuid() != geteuid() || getgid() != getegid()) {
        errno = EPERM;
        return 0;
    }

    status = sys$getjpiw(0, 0, 0, items, &iosb, 0, 0);
    if (!(status & 1) || !(iosb.iosb$w_status & 1)) {
        vaxc$errno = !(status & 1) ? status : iosb.iosb$w_status;
        errno = EVMSERR;
        return 0;
    }
    if (privileges_length != sizeof(image_privileges) ||
        image_privileges[0] || image_privileges[1] || rights_length) {
        errno = EPERM;
        return 0;
    }
    return 1;
}

int
System(char *command)
{
    if (!VmsProcessAllowed())
        return -1;
    return system(command);
}

pointer
Popen(char *command, char *type)
{
    if (!command || !type) {
        errno = EINVAL;
        return NULL;
    }
    if (!VmsProcessAllowed())
        return NULL;
    return popen(command, type);
}

pointer
Fopen(char *file, char *type)
{
    if (!file || !type) {
        errno = EINVAL;
        return NULL;
    }
    if (!VmsProcessAllowed())
        return NULL;
    return fopen(file, type);
}

int
Pclose(pointer stream)
{
    return pclose((FILE *)stream);
}

int
Fclose(pointer stream)
{
    return fclose((FILE *)stream);
}
