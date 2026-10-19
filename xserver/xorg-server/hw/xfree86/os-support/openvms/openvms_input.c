/* Original OpenVMS acquisition boundary for upstream kbd and mouse drivers.
 * Public drivers own maps, scan-code/packet decoding, device creation and events.
 * Hardware ownership and bounded polling remain in ordinary server context.
 */
#define NEED_EVENTS 1
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include "X.h"
#include "Xproto.h"
#include "misc.h"
#include "dix.h"
#include "dixevents.h"
#include "os.h"
#include "opaque.h"
#include "inputstr.h"
#include "scrnintstr.h"
#include "XKBsrv.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86Xinput.h"
#include "xf86_OSproc.h"
#include "xf86OSKbd.h"
#include "mouse.h"
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) < 5
#error The VMS input boundary requires upstream XKB keyboard initialization.
#endif
#if defined(INPUTTHREAD) && INPUTTHREAD
#error Native acquisition requires the upstream non-threaded input profile.
#endif
#include "xisb.h"
#include "openvms_input.h"
#include "openvms_serial.h"

typedef char require_p64[sizeof(void *) == 8 ? 1 : -1];
extern char *ConnectionInfo;
void ProcessInputEvents(void);
void OpenVMSPointerRegister(InputInfoPtr info);

enum { INPUT_COLD, INPUT_ACTIVE, INPUT_STOPPING, INPUT_STOPPED, INPUT_FAILED };
static int phase, registered, queued, attached, service_busy, handlers;
static int acquisition_attempted, backend_stopped;
static InputInfoPtr keyboard, pointer_device;
static int keyboard_enabled, pointer_enabled;
static uint64_t previous_clock, last_poll, last_service, last_activity;
static int screen_width, screen_height;

static int ready(void)
{
    return attached && phase != INPUT_STOPPED && keyboard && pointer_device &&
           keyboard->dev && pointer_device->dev &&
           keyboard->dev->public.devicePrivate == keyboard &&
           pointer_device->dev->public.devicePrivate == pointer_device &&
           keyboard->dev->enabled && pointer_device->dev->enabled &&
           keyboard->dev->coreEvents && pointer_device->dev->coreEvents &&
           keyboard->dev->public.on && pointer_device->dev->public.on &&
           inputInfo.keyboard && inputInfo.pointer &&
           GetMaster(keyboard->dev, MASTER_KEYBOARD) == inputInfo.keyboard &&
           GetMaster(pointer_device->dev, MASTER_POINTER) == inputInfo.pointer &&
           inputInfo.keyboard->enabled && inputInfo.pointer->enabled &&
           inputInfo.keyboard->key && inputInfo.keyboard->key->xkbInfo &&
           inputInfo.pointer->valuator && pointer_device->dev->valuator &&
           screenInfo.numScreens == 1 && screenInfo.screens[0] &&
           screenInfo.screens[0]->width == screen_width &&
           screenInfo.screens[0]->height == screen_height &&
           GetSpriteCursor(pointer_device->dev) != NULL;
}

int OpenVMSKeyboardByte(unsigned char value)
{
    KbdDevPtr kbd;
    if (!ready() || phase != INPUT_ACTIVE) return 0;
    kbd = keyboard->private;
    if (!kbd || !kbd->PostEvent) return 0;
    /* Drain between bytes so upstream repeat filtering sees current DIX state. */
    ProcessInputEvents();
    input_lock();
    kbd->PostEvent(keyboard, value & 0x7fU, (value & 0x80U) == 0);
    input_unlock();
    ProcessInputEvents();
    return 1;
}

int OpenVMSPointerByte(unsigned char value)
{
    if (!ready() || phase != INPUT_ACTIVE || !OpenVMSSerialFeedByte(value)) return 0;
    OpenVMSSerialDrain();
    ProcessInputEvents();
    return 1;
}

int OpenVMSInputReleaseAll(void)
{
    KbdDevPtr kbd;
    MouseDevPtr mouse;
    XISBuffer *buffer;
    XkbEventCauseRec cause;
    unsigned int code;
    if (!attached) return 1;
    if (phase == INPUT_STOPPING ||
        (dispatchException & (DE_RESET | DE_TERMINATE))) {
        /* Fatal cleanup can detach devices without setting dispatchException.
         * Stop acquisition without delivering events into retiring devices. */
        OpenVMSSerialReset();
        return 1;
    }
    if (!ready() || !keyboard->dev->key || !keyboard->dev->key->xkbInfo ||
        !keyboard->private || !pointer_device->private) return 0;
    kbd = keyboard->private;
    mouse = pointer_device->private;
    if (!mouse->PostEvent) return 0;
    OpenVMSSerialReset();
    ProcessInputEvents();
    for (code = 8; code < 256; ++code) {
        if (!((keyboard->dev->key->down[code >> 3] |
               keyboard->dev->key->postdown[code >> 3]) & (1U << (code & 7)))) continue;
        input_lock();
        xf86PostKeyboardEvent(keyboard->dev, code, FALSE);
        input_unlock();
        ProcessInputEvents();
    }
    input_lock();
    mouse->PostEvent(pointer_device, 0, 0, 0, 0, 0);
    input_unlock();
    ProcessInputEvents();
    XkbSetCauseUnknown(&cause);
    XkbClearAllLatchesAndLocks(keyboard->dev, keyboard->dev->key->xkbInfo, TRUE, &cause);
    XkbClearAllLatchesAndLocks(inputInfo.keyboard, inputInfo.keyboard->key->xkbInfo,
                              TRUE, &cause);
    kbd->scanPrefix = 0;
    mouse->protoBufTail = 0;
    memset(mouse->protoBuf, 0, sizeof(mouse->protoBuf));
    mouse->inSync = FALSE;
    mouse->emulateState = 0;
    mouse->emulate3Pending = FALSE;
    mouse->wheelXDistance = mouse->wheelYDistance = 0;
    buffer = mouse->buffer;
    if (buffer) {
        if (buffer->buf && buffer->buffer_size > 0)
            memset(buffer->buf, 0, (size_t)buffer->buffer_size);
        buffer->current = buffer->end = 0;
        buffer->block_duration = 0;
    }
    return 1;
}

int OpenVMSInputReady(void)
{ return keyboard_enabled && pointer_enabled; }

static uint64_t interval(uint64_t now)
{
    return now - last_activity < UINT64_C(100000000) ?
           UINT64_C(1000000) : UINT64_C(8000000);
}

static void service(void)
{
    uint64_t now;
    int result;
    if (phase != INPUT_ACTIVE || dispatchException) return;
    if (service_busy || !ready() || !OpenVMSMonotonicClock(&now) ||
        now < previous_clock || !OpenVMSInputBackendActive() || !OpenVMSSerialReady())
        FatalError("VMS input service violated lifecycle or clock bounds\n");
    service_busy = 1;
    previous_clock = now;
    if (now - last_service >= UINT64_C(1000000)) {
        last_service = now;
        TimerCheck();
        if (phase == INPUT_ACTIVE && !dispatchException &&
            now - last_poll >= interval(now)) {
            last_poll = now;
            result = OpenVMSInputBackendPoll();
            if (result < 0) {
                service_busy = 0;
                phase = INPUT_FAILED;
                FatalError("VMS input physical polling/recovery failed\n");
            }
            if (result > 0) last_activity = now;
        }
    }
    service_busy = 0;
}

static void block(void *data, void *timeout)
{
    uint64_t now, elapsed, wait_ns;
    int wait_ms;
    (void)data;
    if (phase != INPUT_ACTIVE) return;
    service();
    if (dispatchException) { AdjustWaitForDelay(timeout, 0); return; }
    if (!OpenVMSMonotonicClock(&now) || now < previous_clock)
        FatalError("VMS input wait clock failed\n");
    elapsed = now - last_poll;
    wait_ns = interval(now);
    wait_ms = elapsed >= wait_ns ? 1 :
              (int)((wait_ns - elapsed + UINT64_C(999999)) / UINT64_C(1000000));
    AdjustWaitForDelay(timeout, wait_ms);
}

static void wakeup(void *data, int result)
{ (void)data; (void)result; service(); }

static Bool work(ClientPtr client, pointer closure)
{
    if (client || closure || !queued)
        FatalError("VMS input work item violated ownership\n");
    if (phase == INPUT_STOPPED) { queued = 0; return TRUE; }
    if (phase == INPUT_COLD && OpenVMSInputStart() != 1)
        FatalError("VMS input could not start after root/input initialization\n");
    service();
    return FALSE;
}

static void queue_start(void)
{
    if (queued || !keyboard_enabled) return;
    if (!QueueWorkProc(work, NULL, NULL)) FatalError("VMS input cannot queue startup\n");
    queued = 1;
}

int OpenVMSInputStart(void)
{
    InputInfoPtr info;
    uint64_t now, started;
    /* InitInput finishes DEVICE_ON before queued work runs. The upstream
     * server owns notification registration for this real stream descriptor. */
    if (phase == INPUT_COLD && !pointer_device) {
        for (info = xf86InputDevs; info; info = info->next)
            if (OpenVMSSerialOwns(info->fd)) {
                OpenVMSPointerRegister(info);
                OpenVMSSerialAttach(info);
            }
    }
    if (phase != INPUT_COLD || attached || serverGeneration != 1 || InputThreadEnable ||
        dispatchException || !ConnectionInfo || !keyboard || !pointer_device ||
        !keyboard->dev || !pointer_device->dev || !keyboard->dev->inited ||
        !pointer_device->dev->inited || !OpenVMSInputReady() ||
        !OpenVMSSerialReady() ||
        !keyboard->dev->key || !keyboard->dev->key->xkbInfo ||
        !XkbDDXUsesSoftRepeat(keyboard->dev) ||
        screenInfo.numScreens != 1 || !screenInfo.screens[0] ||
        screenInfo.screens[0]->width < 1 || screenInfo.screens[0]->width > 2048 ||
        screenInfo.screens[0]->height < 1 || screenInfo.screens[0]->height > 1536) return 0;
    screen_width = screenInfo.screens[0]->width;
    screen_height = screenInfo.screens[0]->height;
    attached = 1;
    if (!ready() || !OpenVMSMonotonicClock(&started)) { phase = INPUT_FAILED; return 0; }
    acquisition_attempted = 1;
    if (OpenVMSI8042Start() != 1 || !OpenVMSInputBackendActive() ||
        !OpenVMSMonotonicClock(&now) || now < started) { phase = INPUT_FAILED; return 0; }
    previous_clock = last_poll = last_service = last_activity = now;
    if (!RegisterBlockAndWakeupHandlers(block, wakeup, NULL)) {
        phase = INPUT_FAILED;
        return 0;
    }
    handlers = 1;
    phase = INPUT_ACTIVE;
    ErrorF("OPENVMS_INPUT_READY upstream kbd/mouse, native i8042 recovery\n");
    return 1;
}

int OpenVMSInputStop(void)
{
    if (service_busy) return 0;
    if (phase == INPUT_STOPPED) return 1;
    if (handlers) { RemoveBlockAndWakeupHandlers(block, wakeup, NULL); handlers = 0; }
    if (!attached) { phase = INPUT_STOPPED; return 1; }
    if (acquisition_attempted && !backend_stopped && !OpenVMSInputBackendActive()) return 0;
    phase = INPUT_STOPPING;
    if (!backend_stopped) {
        if (!OpenVMSInputBackendStop()) { phase = INPUT_FAILED; return 0; }
        backend_stopped = 1;
    }
    if (!OpenVMSInputReleaseAll()) { phase = INPUT_FAILED; return 0; }
    attached = 0;
    phase = INPUT_STOPPED;
    return 1;
}

static int keyboard_init(InputInfoPtr info, int operation)
{
    return info == keyboard && operation == DEVICE_INIT && phase == INPUT_COLD ?
           Success : BadImplementation;
}
static int keyboard_on(InputInfoPtr info, int operation)
{
    if (info != keyboard || operation != DEVICE_ON || phase != INPUT_COLD) return BadImplementation;
    keyboard_enabled = 1;
    queue_start();
    return Success;
}
static int keyboard_off(InputInfoPtr info, int operation)
{
    if (info != keyboard || (operation != DEVICE_OFF && operation != DEVICE_CLOSE)) return BadImplementation;
    if (!OpenVMSInputStop()) FatalError("VMS input did not stop before keyboard teardown\n");
    keyboard_enabled = 0;
    return Success;
}
static Bool keyboard_open(InputInfoPtr info)
{ return info == keyboard && phase == INPUT_COLD && info->fd == -1; }
static void keyboard_mapping(InputInfoPtr info, KeySymsPtr syms, CARD8 *modifiers)
{
    if (info != keyboard || !syms || !modifiers)
        FatalError("VMS keyboard mapping owner mismatch\n");
    /* ABI >= 5 still calls this legacy callback but ignores its outputs.
     * InitKeyboardDeviceStruct builds the actual map from upstream XKB data. */
    memset(syms, 0, sizeof(*syms));
    memset(modifiers, 0, MAP_LENGTH);
}
static void keyboard_bell(InputInfoPtr info, int percent, int pitch, int duration)
{
    (void)info; (void)percent; (void)pitch; (void)duration;
}
static void keyboard_leds(InputInfoPtr info, int leds)
{ (void)info; (void)leds; }
static int keyboard_get_leds(InputInfoPtr info)
{ (void)info; return 0; }

Bool xf86OSKbdPreInit(InputInfoPtr info)
{
    KbdDevPtr kbd;
    RemapScanCodeProc remap;
    if (!registered || phase != INPUT_COLD || keyboard || !info || !info->private ||
        !info->drv || !info->drv->driverName || strcmp(info->drv->driverName, "kbd") ||
        !info->type_name || strcmp(info->type_name, XI_KEYBOARD) ||
        !(info->flags & XI86_ALWAYS_CORE)) return FALSE;
    remap = (RemapScanCodeProc)LoaderSymbol("ATScancode");
    if (!remap) return FALSE;
    keyboard = info;
    kbd = info->private;
    kbd->KbdInit = keyboard_init;
    kbd->KbdOn = keyboard_on;
    kbd->KbdOff = keyboard_off;
    kbd->OpenKeyboard = keyboard_open;
    kbd->KbdGetMapping = keyboard_mapping;
    kbd->RemapScanCode = remap;
    kbd->Bell = keyboard_bell;
    kbd->SetLeds = keyboard_leds;
    kbd->GetLeds = keyboard_get_leds;
    info->fd = -1;
    xf86Msg(X_INFO, "VMS keyboard: software repeat; physical LEDs and bell unavailable\n");
    return TRUE;
}

static int mouse_interfaces(void) { return MSE_PS2; }
static const char *mouse_default(void) { return "GenericPS/2"; }
OSMouseInfoPtr OSMouseInit(int flags)
{
    static OSMouseInfoRec info;
    (void)flags;
    info.SupportedInterfaces = mouse_interfaces;
    info.DefaultProtocol = mouse_default;
    return &info;
}

void OpenVMSPointerRegister(InputInfoPtr info)
{
    MouseDevPtr mouse = info ? info->private : NULL;
    if (!registered || phase != INPUT_COLD || pointer_device || !info || !info->dev ||
        !info->drv || !info->drv->driverName || strcmp(info->drv->driverName, "mouse") ||
        !info->type_name || strcmp(info->type_name, XI_MOUSE) || !info->dev->coreEvents ||
        !mouse || !mouse->buffer ||
        mouse->protocolID != PROT_GENPS2 || mouse->emulate3Buttons ||
        mouse->emulate3ButtonsSoft || mouse->emulateWheel || mouse->pDragLock)
        FatalError("VMS input requires one ordinary GenericPS/2 core pointer\n");
    pointer_device = info;
    pointer_enabled = 1;
    queue_start();
}
void OpenVMSPointerUnregister(void) { pointer_enabled = 0; }

void OpenVMSInputRegister(void)
{
    if (registered) return;
    if (serverGeneration != 1 || phase != INPUT_COLD)
        FatalError("VMS input supports one physical-owner server generation\n");
    registered = 1;
}

/* Existing upstream DIX hook: stop before clients, roots and devices retire.
 * The first physical input implementation owns one server generation. */
void ddxBeforeReset(void)
{
    if (!registered) return;
    if (OpenVMSInputStop() != 1)
        FatalError("OpenVMS input shutdown failed\n");
    dispatchException |= DE_TERMINATE;
}
