/* Read-only checks for a dedicated local Xorg authority file.
 * Original project support using the public Xau API and C run-time library.
 * File ownership/protection is checked separately by CHECK_AUTHORITY.COM.
 * This does not replace authenticated and unauthenticated connection tests.
 */
#if defined(__VMS) && !defined(_XOPEN_SOURCE_EXTENDED)
#define _XOPEN_SOURCE_EXTENDED 1
#endif
#include <X11/X.h>
#include <X11/Xauth.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_AUTH_BYTES 4096
#define MAX_AUTH_RECORDS 16

#ifdef __VMS
typedef char require_p64[(sizeof(void *) == 8) ? 1 : -1];
#endif

static int
decimal(const char *text, unsigned maximum, int positive)
{
    unsigned value = 0;
    size_t n;

    if (!text || !*text || strlen(text) > 5)
        return 0;
    if (text[0] == '0' && text[1])
        return 0;
    for (n = 0; text[n]; ++n) {
        if (text[n] < '0' || text[n] > '9')
            return 0;
        value = value * 10 + (unsigned)(text[n] - '0');
    }
    return value <= maximum && (!positive || value != 0);
}

static int
native_versioned_file(const char *path)
{
    static const char allowed[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:.[]-;";
    const char *colon, *close, *version, *p;

    if (!path || !*path || strlen(path) > 255 ||
        strspn(path, allowed) != strlen(path) || strstr(path, ".."))
        return 0;
    colon = strchr(path, ':');
    if (!colon || colon == path || colon[1] != '[' ||
        strchr(colon + 1, ':') || strchr(path, '[') != colon + 1)
        return 0;
    if (!colon[2] || colon[2] == '.' || colon[2] == '-')
        return 0;
    close = strchr(colon + 2, ']');
    if (!close || close == colon + 2 || strchr(close + 1, ']') ||
        strchr(colon + 2, '['))
        return 0;
    version = strchr(close + 1, ';');
    if (!version || version == close + 1 || strchr(path, ';') != version ||
        !decimal(version + 1, 32767, 1))
        return 0;
    for (p = close + 1; p < version; ++p)
        if (*p == '[' || *p == ']' || *p == ':')
            return 0;
    return 1;
}

static int
local_record(const Xauth *auth, const char *hostname)
{
    static const unsigned char loopback[] = { 127, 0, 0, 1 };

    if (auth->family == FamilyWild)
        return auth->address_length == 0;
    if (auth->family == FamilyInternet)
        return auth->address_length == sizeof(loopback) && auth->address &&
            memcmp(auth->address, loopback, sizeof(loopback)) == 0;
    if (auth->family == FamilyLocal && hostname && *hostname)
        return auth->address_length == strlen(hostname) && auth->address &&
            memcmp(auth->address, hostname, auth->address_length) == 0;
    return 0;
}

static void
dispose(Xauth *auth)
{
    if (auth && auth->data) {
        volatile unsigned char *p = (volatile unsigned char *)auth->data;
        size_t count = auth->data_length;
        while (count--)
            *p++ = 0;
    }
    XauDisposeAuth(auth);
}

static int
check_records(FILE *stream, size_t file_size, const char *display,
              const char *hostname)
{
    static const char protocol[] = "MIT-MAGIC-COOKIE-1";
    size_t total = 0;
    unsigned count = 0;
    int next, valid;
    Xauth *auth;

    if (!file_size || file_size > MAX_AUTH_BYTES ||
        !decimal(display, 59535, 0))
        return 0;
    for (;;) {
        /* XauReadAuth alone cannot distinguish clean EOF from truncation. */
        next = fgetc(stream);
        if (next == EOF)
            return !ferror(stream) && count != 0 && total == file_size;
        if (count == MAX_AUTH_RECORDS || ungetc(next, stream) == EOF)
            return 0;
        auth = XauReadAuth(stream);
        if (!auth)
            return 0;
        ++count;
        total += 10U + auth->address_length + auth->number_length +
            auth->name_length + auth->data_length;
        valid = total <= file_size &&
            auth->number_length == strlen(display) && auth->number &&
            memcmp(auth->number, display, auth->number_length) == 0 &&
            auth->name_length == sizeof(protocol) - 1 && auth->name &&
            memcmp(auth->name, protocol, sizeof(protocol) - 1) == 0 &&
            auth->data_length == 16 && auth->data &&
            local_record(auth, hostname);
        dispose(auth);
        if (!valid)
            return 0;
    }
}

int
main(int argc, char **argv)
{
    FILE *stream = NULL;
    struct stat before, after;
    char hostname[256] = { 0 };
    int valid = 0;

    if (argc != 4 || strcmp(argv[1], "CHECK") != 0 ||
        !native_versioned_file(argv[2]) || !decimal(argv[3], 59535, 0))
        goto done;
    if (gethostname(hostname, sizeof(hostname)) != 0 ||
        !memchr(hostname, 0, sizeof(hostname)))
        hostname[0] = 0;
    stream = fopen(argv[2], "rb");
    if (!stream || fstat(fileno(stream), &before) != 0 ||
        !S_ISREG(before.st_mode) || before.st_size <= 0 ||
        before.st_size > MAX_AUTH_BYTES)
        goto done;
    valid = check_records(stream, (size_t)before.st_size, argv[3], hostname);
    if (fstat(fileno(stream), &after) != 0 ||
        before.st_size != after.st_size || before.st_mtime != after.st_mtime)
        valid = 0;
done:
    if (stream && fclose(stream) != 0)
        valid = 0;
    if (valid) {
        puts("XORG_AUTH_CHECK_PASS");
        return EXIT_SUCCESS;
    }
    fputs("XORG_AUTH_CHECK_FAILED\n", stderr);
    return EXIT_FAILURE;
}
