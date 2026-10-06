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
#include <string.h>
#include <unistd.h>
#include <unixlib.h>
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
typedef char *VmsCallbackString;
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

/* system() is non-reentrant; the translation callback has no context argument. */
#define VMS_COMMAND_LIMIT 4096
static char VmsImageName[VMS_COMMAND_LIMIT];

static int
VmsImageTranslated(VmsCallbackString name, int kind)
{
    size_t length = strlen(name);
    size_t i;
    if (kind != DECC$K_FILE || length >= sizeof(VmsImageName))
        return 0;
    /* MCR does not accept a quoted image filespec. Limit that one token to
     * ordinary native names; arguments below still support quoted spaces.
     */
    for (i = 0; i < length; ++i)
        if (!((name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= 'a' && name[i] <= 'z') ||
              (name[i] >= '0' && name[i] <= '9') ||
              strchr("_.$-:[]", name[i]) != NULL))
            return 0;
    memcpy(VmsImageName, name, length + 1);
    return 0;
}

/* Decode literal argument quoting, not shell expressions or redirections. */
static int
VmsLiteralArgument(const char **input, char *word, size_t size)
{
    const char *p = *input;
    size_t used = 0;
    int quote = 0, started = 0;
    while (*p == ' ' || *p == '\t')
        ++p;
    while (*p) {
        unsigned char ch = (unsigned char)*p++;
        if (!quote && (ch == ' ' || ch == '\t'))
            break;
        started = 1;
        if (ch < 32 || ch == 127)
            goto invalid;
        if ((ch == '\'' || ch == '"') && (!quote || quote == ch)) {
            quote = quote ? 0 : ch;
            continue;
        }
        if (ch == '\\' && quote != '\'') {
            ch = (unsigned char)*p++;
            if (ch != '\\' && ch != '"' && ch != ' ' && ch != '\t')
                goto invalid;
        }
        /* Apostrophes can request DCL symbol substitution even inside quotes. */
        if (ch == '\'' || ch == '`' ||
            (!quote && strchr(";&|<>()", ch) != NULL))
            goto invalid;
        if (used + 1 >= size) {
            errno = E2BIG;
            return -1;
        }
        word[used++] = (char)ch;
    }
    if (quote)
        goto invalid;
    word[used] = '\0';
    *input = p;
    return started;
invalid:
    errno = EINVAL;
    return -1;
}

static int
VmsQuotedArgument(char *output, size_t *used, const char *word)
{
    size_t need = strlen(word) + 3;
    const char *p;
    for (p = word; *p; ++p)
        if (*p == '"')
            ++need;
    if (*used + need >= VMS_COMMAND_LIMIT) {
        errno = E2BIG;
        return 0;
    }
    output[(*used)++] = ' ';
    output[(*used)++] = '"';
    while (*word) {
        if (*word == '"')
            output[(*used)++] = '"';
        output[(*used)++] = *word++;
    }
    output[(*used)++] = '"';
    output[*used] = '\0';
    return 1;
}

/* Native DCL commands retain their existing behavior. A Unix executable path
 * accepts only literal arguments: no expansion, pipeline, redirection or shell.
 * The C RTL translates the image path; MCR supplies foreign-image activation.
 */
static const char *
VmsProcessCommand(const char *command, char *output)
{
    const char *p = command, *leaf, *path;
    char word[VMS_COMMAND_LIMIT];
    size_t used = 3, length;
    int result;
    if (!p)
        return NULL;
    while (*p == ' ' || *p == '\t')
        ++p;
    path = p;
    if (*path == '"' || *path == '\'')
        ++path;
    if (*path != '/' && !(path[0] == '.' && path[1] == '/') &&
        !(path[0] == '.' && path[1] == '.' && path[2] == '/'))
        return command;
    if (VmsLiteralArgument(&p, word, sizeof(word)) != 1)
        return NULL;
    leaf = strrchr(word, '/');
    if (!leaf || !leaf[1] || strpbrk(word, "*?%\"'") != NULL) {
        errno = EINVAL;
        return NULL;
    }
    if (!strchr(leaf, '.')) {
        length = strlen(word);
        if (length + 4 >= sizeof(word)) {
            errno = E2BIG;
            return NULL;
        }
        memcpy(word + length, ".exe", 5);
    }
    VmsImageName[0] = '\0';
    (void)decc$to_vms(word, VmsImageTranslated, 0, 1);
    if (!VmsImageName[0]) {
        errno = EINVAL;
        return NULL;
    }
    length = strlen(VmsImageName);
    if (length + 5 >= VMS_COMMAND_LIMIT) {
        errno = E2BIG;
        return NULL;
    }
    memcpy(output, "MCR ", 4);
    memcpy(output + 4, VmsImageName, length + 1);
    used = length + 4;
    while ((result = VmsLiteralArgument(&p, word, sizeof(word))) > 0)
        if (!VmsQuotedArgument(output, &used, word))
            return NULL;
    return result < 0 ? NULL : output;
}

int
System(char *command)
{
    char native[VMS_COMMAND_LIMIT];
    const char *translated;
    if (!VmsProcessAllowed())
        return -1;
    if (!command)
        return system(NULL);
    translated = VmsProcessCommand(command, native);
    if (!translated)
        return -1;
    return system(translated);
}

pointer
Popen(char *command, char *type)
{
    char native[VMS_COMMAND_LIMIT];
    const char *translated;
    if (!command || !type) {
        errno = EINVAL;
        return NULL;
    }
    if (!VmsProcessAllowed())
        return NULL;
    translated = VmsProcessCommand(command, native);
    if (!translated)
        return NULL;
    return popen(translated, type);
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
