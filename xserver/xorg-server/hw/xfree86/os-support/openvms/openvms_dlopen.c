/* OpenVMS CRTL image namespace for the upstream Xorg loader.
 * XORG.EXE identifies its already linked core image explicitly. Modules
 * are discovered by upstream loadmod.c, not by a fixed provider registry.
 *
 * CRTL image activation is deferred until dlsym and images remain resident.
 * Closing a wrapper handle releases its logical reference and removes its
 * symbols from this namespace; reopening reuses the resident native handle.
 * Calls are serialized by Xorg's loader, not made from input callbacks.
 */
#include "openvms_dlopen.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct VmsXorgImage {
    struct VmsXorgImage *next;
    char *path;
    void *native;
    unsigned int references;
    int core;
} VmsXorgImage;

static VmsXorgImage *images;
static VmsXorgImage *core;
static char *core_path;
static char error_text[512];
static int error_pending;

static void
record_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(error_text, sizeof(error_text), format, args);
    va_end(args);
    error_pending = 1;
}

static int
valid_text(const char *value)
{
    const unsigned char *p = (const unsigned char *) value;
    if (!p || !*p)
        return 0;
    for (; *p; ++p)
        if (*p < 32 || *p == 127)
            return 0;
    return 1;
}

static int
absolute_core_path(const char *path)
{
    const char *colon, *end, *version;
    if (!valid_text(path))
        return 0;
    if (*path == '/')
        return 1;
    colon = strchr(path, ':');
    if (!colon || colon == path || colon[1] != '[' ||
        colon[2] == '.' || colon[2] == '-' || colon[2] == ']')
        return 0;
    end = strchr(colon + 2, ']');
    if (!end || !end[1] || strchr(path, '/'))
        return 0;
    version = strrchr(end + 1, ';');
    if (!version || version == end + 1 || version[1] < '1' || version[1] > '9')
        return 0;
    for (version += 2; *version; ++version)
        if (*version < '0' || *version > '9')
            return 0;
    return 1;
}

int
VmsXorgSetCoreImage(const char *image)
{
    char *copy;
    error_pending = 0;
    if (!absolute_core_path(image)) {
        record_error("Xorg core requires an exact absolute image path");
        return -1;
    }
    if (core_path) {
        if (!strcmp(core_path, image))
            return 0;
        record_error("Xorg core image has already been registered");
        return -1;
    }
    copy = strdup(image);
    if (!copy) {
        record_error("Cannot allocate the Xorg core image path");
        return -1;
    }
    core_path = copy;
    return 0;
}

static VmsXorgImage *
find_handle(void *handle)
{
    VmsXorgImage *image;
    for (image = images; image; image = image->next)
        if (image == handle)
            return image;
    return NULL;
}

static VmsXorgImage *
open_image(const char *path, int flags, int is_core)
{
    VmsXorgImage *image, **tail;
    const char *native_error;
    for (image = images; image; image = image->next)
        if (image->core == is_core && !strcmp(image->path, path))
            return image;
    image = calloc(1, sizeof(*image));
    if (!image) {
        record_error("Cannot allocate Xorg image handle for %s", path);
        return NULL;
    }
    image->path = strdup(path);
    if (!image->path) {
        free(image);
        record_error("Cannot allocate Xorg image path for %s", path);
        return NULL;
    }
    (void) dlerror();
    image->native = dlopen(path, flags);
    if (!image->native) {
        native_error = dlerror();
        record_error("Cannot open %s: %s", path,
                     native_error ? native_error : "CRTL dlopen failed");
        free(image->path);
        free(image);
        return NULL;
    }
    image->core = is_core;
    for (tail = &images; *tail; tail = &(*tail)->next)
        ;
    *tail = image;
    return image;
}

static VmsXorgImage *
open_core(int flags)
{
    if (!core_path) {
        record_error("Xorg core image has not been registered by XORG.EXE");
        return NULL;
    }
    if (!core)
        core = open_image(core_path, flags, 1);
    return core;
}

void *
VmsXorgDlopen(const char *path, int flags)
{
    VmsXorgImage *image;
    error_pending = 0;
    if (!core_path) {
        record_error("Xorg core image has not been registered by XORG.EXE");
        return NULL;
    }
    if (!path)
        image = open_core(flags);
    else {
        if (!valid_text(path) || path[0] != '/') {
            record_error("Xorg module requires an absolute Unix-style path");
            return NULL;
        }
        if (!strcmp(path, core_path)) {
            record_error("Xorg core is not a loadable driver module");
            return NULL;
        }
        image = open_image(path, flags, 0);
    }
    if (!image)
        return NULL;
    if (image->references == UINT_MAX) {
        record_error("Xorg image reference count overflow for %s", image->path);
        return NULL;
    }
    ++image->references;
    return image;
}

static void *
image_symbol(VmsXorgImage *image, const char *name)
{
    (void) dlerror();
    return dlsym(image->native, name);
}

void *
VmsXorgDlsym(void *handle, const char *name)
{
    VmsXorgImage *image;
    void *symbol;
    const char *native_error;
    error_pending = 0;
    if (!valid_text(name)) {
        record_error("Xorg symbol name is empty or contains a control character");
        return NULL;
    }
    /* dlopen(NULL) is the same global namespace as RTLD_DEFAULT, not just
     * the core's export table. Upstream LoaderSymbol uses both forms. */
    if (handle == RTLD_DEFAULT || (core && handle == core && core->references)) {
#ifdef RTLD_LAZY
        image = open_core(RTLD_LAZY);
#else
        image = open_core(0);
#endif
        if (!image)
            return NULL;
        symbol = image_symbol(image, name);
        if (symbol)
            return symbol;
        for (image = images; image; image = image->next) {
            if (image->core || !image->references)
                continue;
            symbol = image_symbol(image, name);
            if (symbol)
                return symbol;
        }
        native_error = dlerror();
        record_error("Xorg global symbol %s was not found: %s", name,
                     native_error ? native_error : "no active image exports it");
        return NULL;
    }
    image = find_handle(handle);
    if (!image || !image->references) {
        record_error("Xorg symbol lookup received an inactive or invalid image handle");
        return NULL;
    }
    symbol = image_symbol(image, name);
    if (!symbol) {
        native_error = dlerror();
        record_error("Cannot find %s in %s: %s", name, image->path,
                     native_error ? native_error : "CRTL dlsym failed");
    }
    return symbol;
}

int
VmsXorgDlclose(void *handle)
{
    VmsXorgImage *image;
    error_pending = 0;
    image = find_handle(handle);
    if (!image || !image->references) {
        record_error("Xorg close received an inactive or invalid image handle");
        return -1;
    }
    --image->references;
    return 0;
}

char *
VmsXorgDlerror(void)
{
    if (!error_pending)
        return NULL;
    error_pending = 0;
    return error_text;
}
