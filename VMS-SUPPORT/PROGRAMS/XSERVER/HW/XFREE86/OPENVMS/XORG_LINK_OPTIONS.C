/* Native ELF64 link-options generator for OpenVMS x86-64 Xorg.
 * Original project build support. Input objects must be built from this tree.
 *
 * IMAGE|id|SHARE or EXE|image-path|module-metadata-symbol (empty for core/entry)
 * OBJECT|id|SEED or POOL|object-path
 * RUNTIME|id|shareable-image-path
 *
 * Only core has POOL objects. Their order is the library search order. All
 * other objects are explicit. Core and entry are required image identifiers.
 * The caller creates output directories and invokes generated LINK_IMAGES.COM
 * from the same source-root default directory used to read the manifest.
 */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_IMAGES 32
#define MAX_OBJECTS 4096
#define MAX_LINE 4096
#define LIMIT_FILE (256U * 1024U * 1024U)

typedef struct { char *name; unsigned type; } Symbol;
typedef struct {
    char *path;
    int image, selected;
    Symbol *defs;
    size_t ndefs;
    char **refs;
    size_t nrefs;
} Object;
typedef struct {
    char *id, *path, *metadata;
    int executable;
    char **runtime;
    size_t nruntime;
    unsigned char deps[MAX_IMAGES];
} Image;
static Image images[MAX_IMAGES];
static Object objects[MAX_OBJECTS];
static int nimages, nobjects, core = -1, entry = -1;
static const char *context = "manifest";

static void die(const char *format, ...)
{
    va_list ap;
    fprintf(stderr, "XORG_LINK_OPTIONS: %s: ", context);
    va_start(ap, format); vfprintf(stderr, format, ap); va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}
static void *resize(void *p, size_t n)
{
    void *q = realloc(p, n ? n : 1);
    if (!q) die("out of memory");
    return q;
}
static char *copy(const char *s)
{
    char *p = resize(NULL, strlen(s) + 1);
    strcpy(p, s); return p;
}
static int identifier(const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    if (!(isalpha(*p) || *p == '_' || *p == '$')) return 0;
    while (*++p) if (!(isalnum(*p) || *p == '_' || *p == '$')) return 0;
    return 1;
}
static int path_ok(const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    if (!*p) return 0;
    for (; *p; ++p)
        if (!(isalnum(*p) || strchr("_-$.:[];/\\", *p))) return 0;
    return 1;
}
static uint16_t u16(const unsigned char *p)
{ return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t u32(const unsigned char *p)
{ return (uint32_t)u16(p) | ((uint32_t)u16(p + 2) << 16); }
static uint64_t u64(const unsigned char *p)
{ return (uint64_t)u32(p) | ((uint64_t)u32(p + 4) << 32); }
static void span(uint64_t off, uint64_t len, size_t size)
{ if (off > size || len > (uint64_t)size - off) die("truncated ELF object"); }

static void read_object(Object *obj)
{
    FILE *f;
    unsigned char *raw;
    long length;
    size_t size, i;
    uint64_t shoff;
    unsigned count, found = 0;
    context = obj->path;
    f = fopen(obj->path, "rb");
    if (!f) die("open failed: %s", strerror(errno));
    if (fseek(f, 0, SEEK_END) || (length = ftell(f)) < 64 ||
        (unsigned long)length > LIMIT_FILE || fseek(f, 0, SEEK_SET))
        die("invalid object size");
    size = (size_t)length;
    raw = resize(NULL, size);
    if (fread(raw, 1, size, f) != size || fclose(f)) die("object read failed");
    if (memcmp(raw, "\177ELF\002\001\001", 7) || u16(raw + 16) != 1 ||
        u16(raw + 18) != 62 || u32(raw + 20) != 1 ||
        u16(raw + 52) != 64 || u16(raw + 58) != 64)
        die("expected ELF64 little-endian x86-64 relocatable object");
    shoff = u64(raw + 40); count = u16(raw + 60);
    if (!count || shoff < 64) die("missing section table");
    span(shoff, (uint64_t)count * 64, size);
    for (i = 0; i < count; ++i) {
        const unsigned char *s = raw + shoff + i * 64;
        uint64_t off = u64(s + 24), len = u64(s + 32), at;
        uint32_t type = u32(s + 4), link = u32(s + 40);
        const unsigned char *strsec, *strings;
        uint64_t strsize;
        if (type != 8) span(off, len, size);
        if (type != 2) continue;
        ++found;
        if (u64(s + 56) != 24 || len % 24 || link >= count)
            die("invalid symbol table");
        strsec = raw + shoff + (size_t)link * 64;
        if (u32(strsec + 4) != 3) die("symbol strings missing");
        strsize = u64(strsec + 32);
        span(u64(strsec + 24), strsize, size);
        strings = raw + u64(strsec + 24);
        for (at = off; at < off + len; at += 24) {
            const unsigned char *sym = raw + at;
            uint32_t name = u32(sym);
            unsigned bind = sym[4] >> 4, kind = sym[4] & 15;
            const char *spelling;
            if ((bind != 1 && bind != 2) || !name) continue;
            if (name >= strsize || !memchr(strings + name, 0, (size_t)(strsize - name)))
                die("invalid symbol spelling");
            spelling = (const char *)strings + name;
            if (!identifier(spelling)) die("unsupported linker symbol %s", spelling);
            if (u16(sym + 6) == 0) {
                obj->refs = resize(obj->refs, (obj->nrefs + 1) * sizeof(*obj->refs));
                obj->refs[obj->nrefs++] = copy(spelling);
            } else {
                size_t k;
                if (kind != 1 && kind != 2 && kind != 5)
                    die("unclassified global definition %s", spelling);
                for (k = 0; k < obj->ndefs; ++k)
                    if (!strcmp(spelling, obj->defs[k].name))
                        die("repeated global definition %s", spelling);
                obj->defs = resize(obj->defs, (obj->ndefs + 1) * sizeof(*obj->defs));
                obj->defs[obj->ndefs].name = copy(spelling);
                obj->defs[obj->ndefs++].type = kind == 2 ? 2 : 1;
            }
        }
    }
    if (!found) die("missing symbol table");
    free(raw);
}

static int image_id(const char *name)
{
    int i;
    for (i = 0; i < nimages; ++i) if (!strcmp(images[i].id, name)) return i;
    return -1;
}
static void read_manifest(const char *name)
{
    FILE *f = fopen(name, "r");
    char line[MAX_LINE];
    unsigned number = 0;
    int id;
    context = name;
    if (!f) die("open failed: %s", strerror(errno));
    while (fgets(line, sizeof line, f)) {
        char *fields[6], *p;
        int nf = 1;
        ++number;
        if (!strchr(line, '\n') && !feof(f)) die("line %u too long", number);
        line[strcspn(line, "\r\n")] = 0;
        if (!*line || line[0] == '#') continue;
        fields[0] = line;
        for (p = line; *p; ++p) if (*p == '|') {
            *p = 0;
            if (nf == 6) die("line %u has too many fields", number);
            fields[nf++] = p + 1;
        }
        if (nf < 3 || !identifier(fields[1]) || strlen(fields[1]) > 63)
            die("invalid line %u", number);
        id = image_id(fields[1]);
        if (!strcmp(fields[0], "IMAGE")) {
            Image *im;
            if (nf != 5 || id >= 0 || nimages == MAX_IMAGES ||
                (strcmp(fields[2], "SHARE") && strcmp(fields[2], "EXE")) ||
                !path_ok(fields[3]) || (*fields[4] && !identifier(fields[4])))
                die("invalid IMAGE line %u", number);
            im = &images[nimages++];
            im->id = copy(fields[1]); im->path = copy(fields[3]);
            im->metadata = copy(fields[4]); im->executable = !strcmp(fields[2], "EXE");
        } else if (!strcmp(fields[0], "OBJECT")) {
            Object *obj;
            int k;
            if (nf != 4 || id < 0 || nobjects == MAX_OBJECTS ||
                (strcmp(fields[2], "SEED") && strcmp(fields[2], "POOL")) ||
                (!strcmp(fields[2], "POOL") && strcmp(fields[1], "core")) ||
                !path_ok(fields[3])) die("invalid OBJECT line %u", number);
            for (k = 0; k < nobjects; ++k)
                if (!strcmp(objects[k].path, fields[3])) die("repeated object %s", fields[3]);
            obj = &objects[nobjects++];
            obj->path = copy(fields[3]); obj->image = id;
            obj->selected = !strcmp(fields[2], "SEED");
        } else if (!strcmp(fields[0], "RUNTIME")) {
            Image *im;
            if (nf != 3 || id < 0 || !path_ok(fields[2])) die("invalid RUNTIME line %u", number);
            im = &images[id];
            im->runtime = resize(im->runtime, (im->nruntime + 1) * sizeof(*im->runtime));
            im->runtime[im->nruntime++] = copy(fields[2]);
        } else die("unknown directive on line %u", number);
    }
    if (ferror(f) || fclose(f)) die("manifest read failed");
    core = image_id("core"); entry = image_id("entry");
    if (core < 0 || entry < 0 || images[core].executable || !images[entry].executable)
        die("SHARE core and EXE entry are required");
    for (id = 0; id < nimages; ++id)
        if ((id != core && id != entry && (images[id].executable || !*images[id].metadata)) ||
            ((id == core || id == entry) && *images[id].metadata))
            die("invalid image role: %s", images[id].id);
}

static int defines(const Object *o, const char *name, unsigned *type)
{
    size_t k;
    for (k = 0; k < o->ndefs; ++k) if (!strcmp(o->defs[k].name, name)) {
        if (type) *type = o->defs[k].type;
        return 1;
    }
    return 0;
}
static int provider(const char *name, int selected_only)
{
    int j;
    for (j = 0; j < nobjects; ++j)
        if (objects[j].image != entry && (!selected_only || objects[j].selected) &&
            defines(&objects[j], name, NULL)) return j;
    return -1;
}
static void resolve(void)
{
    int i, j, changed;
    size_t k;
    context = "symbol resolution";
    do {
        changed = 0;
        for (i = 0; i < nobjects; ++i) if (objects[i].selected) {
            for (k = 0; k < objects[i].nrefs; ++k) {
                const char *name = objects[i].refs[k];
                if (provider(name, 1) >= 0) continue;
                j = provider(name, 0);
                if (j >= 0) { objects[j].selected = 1; changed = 1; }
            }
        }
    } while (changed);
    for (i = 0; i < nobjects; ++i) if (objects[i].selected && objects[i].image != entry) {
        for (k = 0; k < objects[i].ndefs; ++k) {
            const char *name = objects[i].defs[k].name;
            if (provider(name, 1) != i) die("duplicate selected symbol %s in %s", name, objects[i].path);
        }
    }
    j = provider("main", 1);
    {
        unsigned type = 0;
        if (j < 0 || objects[j].image != core || !defines(&objects[j], "main", &type) || type != 2)
            die("core entry PROCEDURE main is missing");
    }
    if (provider("XorgNativeEntry", 1) >= 0)
        die("XorgNativeEntry is reserved for the core main alias");
    for (i = 0; i < nimages; ++i) if (*images[i].metadata) {
        unsigned type = 0;
        j = provider(images[i].metadata, 1);
        if (j < 0 || objects[j].image != i || !defines(&objects[j], images[i].metadata, &type) || type != 1)
            die("missing DATA module metadata %s", images[i].metadata);
    }
    images[entry].deps[core] = 1;
    for (i = 0; i < nobjects; ++i) if (objects[i].selected) {
        for (k = 0; k < objects[i].nrefs; ++k) {
            j = provider(objects[i].refs[k], 1);
            if (j >= 0 && objects[i].image != objects[j].image) {
                int a = objects[i].image, b = objects[j].image;
                if (a == core || (a == entry && b != core))
                    die("core/entry references module symbol %s", objects[i].refs[k]);
                images[a].deps[b] = 1;
            }
        }
    }
}

static FILE *output(const char *prefix, const char *leaf)
{
    char path[MAX_LINE];
    FILE *f;
    if (snprintf(path, sizeof path, "%s%s", prefix, leaf) >= (int)sizeof path) die("output path too long");
    f = fopen(path, "w");
    if (!f) die("cannot create %s: %s", path, strerror(errno));
    return f;
}
static void close_output(FILE *f)
{ if (ferror(f) || fclose(f)) die("output write failed"); }
static void upper(char *s)
{ for (; *s; ++s) *s = (char)toupper((unsigned char)*s); }
static void image_logical(const char *path, char *out, size_t capacity)
{
    const char *p, *leaf = path;
    size_t n;
    for (p = path; *p; ++p) if (*p == ']' || *p == ':' || *p == '/' || *p == '\\') leaf = p + 1;
    n = strcspn(leaf, ".;");
    if (!n || n >= capacity) die("invalid shareable image filename");
    memcpy(out, leaf, n); out[n] = 0; upper(out);
    if (!identifier(out)) die("invalid image logical name");
}
static void generate(const char *prefix)
{
    int i, j, order[MAX_IMAGES], count = 0;
    unsigned char done[MAX_IMAGES] = {0};
    FILE *script;
    context = "link output";
    /* Topological order also rejects dependencies between cyclic modules. */
    while (count < nimages) {
        int progress = 0;
        for (i = 0; i < nimages; ++i) if (!done[i]) {
            for (j = 0; j < nimages; ++j) if (images[i].deps[j] && !done[j]) break;
            if (j == nimages) { order[count++] = i; done[i] = 1; progress = 1; }
        }
        if (!progress) die("cyclic shareable-image dependency");
    }
    for (i = 0; i < nimages; ++i) {
        char leaf[128];
        FILE *f;
        size_t k;
        snprintf(leaf, sizeof leaf, "%s.OPT", images[i].id); upper(leaf);
        f = output(prefix, leaf);
        fprintf(f, "CASE_SENSITIVE=YES\nIDENTIFICATION=\"XORG_NATIVE\"\n");
        fprintf(f, "! Keep the linker default GSMATCH=EQUAL and unique link-time identity.\n");
        for (j = 0; j < nobjects; ++j) if (objects[j].selected && objects[j].image == i) {
            if (!images[i].executable) for (k = 0; k < objects[j].ndefs; ++k) {
                const Symbol *s = &objects[j].defs[k];
                if (strcmp(s->name, "main") && strcmp(s->name, "__main"))
                    fprintf(f, "SYMBOL_VECTOR=(%s=%s)\n", s->name, s->type == 2 ? "PROCEDURE" : "DATA");
            }
        }
        if (i == core) fprintf(f, "SYMBOL_VECTOR=(XorgNativeEntry/main=PROCEDURE)\n");
        for (j = 0; j < nobjects; ++j)
            if (objects[j].selected && objects[j].image == i) fprintf(f, "%s\n", objects[j].path);
        for (j = 0; j < nimages; ++j)
            if (images[i].deps[j]) fprintf(f, "%s/SHAREABLE\n", images[j].path);
        for (k = 0; k < images[i].nruntime; ++k) fprintf(f, "%s/SHAREABLE\n", images[i].runtime[k]);
        close_output(f);
    }
    script = output(prefix, "LINK_IMAGES.COM");
    fprintf(script, "$ SET NOON\n$ ON WARNING THEN GOTO failed\n$ ON CONTROL_Y THEN GOTO failed\n");
    fprintf(script, "$ IF F$TRNLNM(\"LNK$LIBRARY\") .NES. \"\" THEN EXIT 2\n");
    fprintf(script, "$ n = 1\n$library_check:\n$ IF F$TRNLNM(\"LNK$LIBRARY_\" + F$STRING(n)) .NES. \"\" THEN EXIT 2\n");
    fprintf(script, "$ n = n + 1\n$ IF n .LE. 999 THEN GOTO library_check\n");
    for (i = 0; i < count; ++i) {
        int role = order[i];
        char id[128];
        strcpy(id, images[role].id); upper(id);
        for (j = 0; j < i; ++j) if (!images[order[j]].executable) {
            char logical[128];
            image_logical(images[order[j]].path, logical, sizeof logical);
            fprintf(script, "$ DEFINE/USER_MODE %s \"%s\"\n", logical, images[order[j]].path);
        }
        fprintf(script, "$ LINK/NODEBUG/FULL%s/MAP=%s%s.MAP -\n",
                role == core ? "/SYSEXE" : "", prefix, id);
        fprintf(script, "    /%s=%s -\n", images[role].executable ? "EXECUTABLE" : "SHAREABLE", images[role].path);
        fprintf(script, "    %s%s.OPT/OPTIONS\n$ IF .NOT. $STATUS THEN GOTO failed\n", prefix, id);
    }
    fprintf(script, "$ WRITE SYS$OUTPUT \"Xorg and loadable display modules linked\"\n$ EXIT 1\n$failed:\n$ EXIT 2\n");
    close_output(script);
}

int main(int argc, char **argv)
{
    int i, selected = 0;
    if (argc != 3 || !path_ok(argv[2])) {
        fprintf(stderr, "usage: XORG_LINK_OPTIONS manifest.txt output-directory-prefix\n");
        return EXIT_FAILURE;
    }
    read_manifest(argv[1]);
    for (i = 0; i < nobjects; ++i) read_object(&objects[i]);
    resolve(); generate(argv[2]);
    for (i = 0; i < nobjects; ++i) selected += objects[i].selected;
    printf("Xorg link options: %d images, %d of %d input objects selected\n", nimages, selected, nobjects);
    return EXIT_SUCCESS;
}
