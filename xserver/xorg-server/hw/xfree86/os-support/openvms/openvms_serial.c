/* Original bounded native byte stream for upstream GenericPS/2 processing.
 * A real nonblocking socketpair participates in upstream SetNotifyFd.
 * Hardware ownership and polling remain in ordinary server context; no input
 * thread, AST callback, pseudo descriptor or private SIGIO registration is used.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include "xf86.h"
#include "xf86Xinput.h"
#include "xf86_OSproc.h"
#include "openvms_serial.h"

#if defined(INPUTTHREAD) && INPUTTHREAD
#error The native byte stream is owned by the ordinary server input loop.
#endif

#define VMS_MOUSE_CAPACITY 256U

static unsigned int used, discard_pending;
static int descriptors[2] = { -1, -1 };
static int failed, draining;
static InputInfoPtr owner;

extern int OpenVMSInputStop(void);
extern void OpenVMSPointerUnregister(void);

int OpenVMSSerialOwns(int token)
{
    return token >= 0 && token == descriptors[0];
}

int OpenVMSSerialReady(void)
{
    return descriptors[0] >= 0 && !failed && owner && owner->read_input;
}

static int transient_error(void)
{
    return errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK;
}

/* A successful write need not be readable yet. Keep reset's old-byte prefix
 * separate from later input, and never wait or spin to finish discarding it. */
static int discard_once(void)
{
    unsigned char discard[VMS_MOUSE_CAPACITY];
    int count;
    if (!discard_pending) return 1;
    count = (int)read(descriptors[0], discard, discard_pending);
    if (count < 0 && transient_error()) return 0;
    if (count <= 0 || (unsigned)count > discard_pending) {
        failed = 1;
        errno = EIO;
        return -1;
    }
    used -= (unsigned)count;
    discard_pending -= (unsigned)count;
    return discard_pending == 0;
}

void OpenVMSSerialReset(void)
{
    if (descriptors[0] < 0) { used = discard_pending = 0; return; }
    discard_pending = used;
    if (!failed) (void)discard_once();
}

int OpenVMSSerialFeedByte(unsigned char value)
{
    int count, saved_errno;
    if (!OpenVMSSerialReady()) return 0;
    if (discard_once() < 0) return 0;
    if (used == VMS_MOUSE_CAPACITY) {
        OpenVMSSerialReset();
        return 0;
    }
    count = (int)write(descriptors[1], &value, 1);
    if (count != 1) {
        if (count < 0 && transient_error()) {
            saved_errno = errno;
            OpenVMSSerialReset();
            errno = saved_errno;
        } else failed = 1;
        return 0; /* The caller must recover the packet; this byte was rejected. */
    }
    ++used;
    return 1;
}

void OpenVMSSerialDrain(void)
{
    if (draining || !used || !OpenVMSSerialReady()) return;
    draining = 1;
    owner->read_input(owner);
    draining = 0;
}

void OpenVMSSerialAttach(InputInfoPtr info)
{
    if (!info || !OpenVMSSerialOwns(info->fd) || failed ||
        !info->read_input || owner)
        FatalError("VMS Xorg invalid native pointer stream registration\n");
    owner = info;
}

int xf86OpenSerial(XF86OptionPtr options)
{
    const char *device = xf86CheckStrOption(options, "Device", NULL);
    const char *protocol = xf86CheckStrOption(options, "Protocol", NULL);
    int pair[2], i, saved_errno, flags;
    if (!device || !protocol || strcmp(device, "OPENVMS_I8042_POINTER") ||
        strcmp(protocol, "GenericPS/2")) {
        errno = ENOTSUP;
        return -1;
    }
    if (failed || descriptors[0] >= 0) {
        errno = failed ? EIO : EBUSY;
        return -1;
    }
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair) != 0) return -1;
    if (pair[0] >= FD_SETSIZE) {
        (void)close(pair[0]); (void)close(pair[1]);
        errno = EMFILE;
        return -1;
    }
    for (i = 0; i < 2; ++i) {
        flags = fcntl(pair[i], F_GETFL, 0);
        if (flags < 0 || fcntl(pair[i], F_SETFL, flags | O_NONBLOCK) < 0 ||
            fcntl(pair[i], F_SETFD, FD_CLOEXEC) < 0) {
            saved_errno = errno;
            (void)close(pair[0]); (void)close(pair[1]);
            errno = saved_errno;
            return -1;
        }
    }
    descriptors[0] = pair[0]; descriptors[1] = pair[1];
    used = discard_pending = 0;
    return descriptors[0];
}

int xf86CloseSerial(int token)
{
    if (!OpenVMSSerialOwns(token)) {
        errno = EBADF;
        return -1;
    }
    if (owner) {
        if (!OpenVMSInputStop()) {
            failed = 1;
            ErrorF("VMS Xorg native pointer stream retains failed hardware ownership\n");
        } else {
            OpenVMSPointerUnregister();
            owner = NULL;
        }
    }
    if (failed) {
        errno = EIO;
        return -1;
    }
    OpenVMSSerialReset();
    if (failed || close(descriptors[0]) != 0 || close(descriptors[1]) != 0) {
        failed = 1;
        errno = EIO;
        return -1;
    }
    descriptors[0] = descriptors[1] = -1;
    used = discard_pending = 0;
    return 0;
}

int xf86ReadSerial(int token, void *buffer, int count)
{
    int copied;
    if (!OpenVMSSerialOwns(token) || failed) {
        errno = failed ? EIO : EBADF;
        return -1;
    }
    if (count < 0 || (!buffer && count)) {
        errno = EINVAL;
        return -1;
    }
    if (!count) return 0;
    copied = discard_once();
    if (copied <= 0) return copied;
    if (!used) return 0;
    if ((unsigned)count > used) count = (int)used;
    copied = (int)read(token, buffer, (unsigned)count);
    if (copied < 0 && transient_error()) return 0;
    if (copied <= 0 || (unsigned)copied > used) { failed = 1; errno = EIO; return -1; }
    used -= (unsigned)copied;
    return copied;
}

int xf86WaitForInput(int token, int timeout)
{
    fd_set input;
    struct timeval wait = { 0, 0 };
    int result;
    if (!OpenVMSSerialOwns(token) || failed) {
        errno = failed ? EIO : EBADF;
        return -1;
    }
    if (timeout < 0) {
        errno = EINVAL;
        return -1;
    }
    /* Polling is driven by the ordinary native scheduler; never block here. */
    result = discard_once();
    if (result <= 0) return result;
    if (!used) return 0;
    FD_ZERO(&input);
    FD_SET(token, &input);
    result = select(token + 1, &input, NULL, NULL, &wait);
    if (result < 0 && transient_error()) return 0;
    if (result < 0) { failed = 1; return -1; }
    return result > 0 && FD_ISSET(token, &input);
}

int xf86FlushInput(int token)
{
    if (!OpenVMSSerialOwns(token) || failed) {
        errno = failed ? EIO : EBADF;
        return -1;
    }
    OpenVMSSerialReset();
    return failed ? -1 : 0;
}

static int unsupported(void)
{
    errno = ENOTSUP;
    return -1;
}

int xf86SetSerial(int token, XF86OptionPtr options)
{ (void)token; (void)options; return unsupported(); }
int xf86SetSerialSpeed(int token, int speed)
{ (void)token; (void)speed; return unsupported(); }
int xf86WriteSerial(int token, const void *buffer, int count)
{ (void)token; (void)buffer; (void)count; return unsupported(); }
int xf86SerialSendBreak(int token, int duration)
{ (void)token; (void)duration; return unsupported(); }
int xf86SetSerialModemState(int token, int state)
{ (void)token; (void)state; return unsupported(); }
int xf86GetSerialModemState(int token)
{ (void)token; return unsupported(); }
int xf86SerialModemSetBits(int token, int bits)
{ (void)token; (void)bits; return unsupported(); }
int xf86SerialModemClearBits(int token, int bits)
{ (void)token; (void)bits; return unsupported(); }
