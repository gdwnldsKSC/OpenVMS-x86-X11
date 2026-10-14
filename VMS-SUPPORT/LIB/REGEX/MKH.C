/* Native implementation of the -p and -i modes used by extras/regex/mkh. */
#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

static FILE *temporary_output;
static char *temporary_name;
static int temporary_owned;

static void fail(const char *message)
{
    fprintf(stderr, "MKH: %s\n", message);
    if (temporary_output != NULL)
        fclose(temporary_output);
    if (temporary_owned && remove(temporary_name) != 0)
        fprintf(stderr, "MKH: cannot remove temporary %s\n", temporary_name);
    exit(EXIT_FAILURE);
}

static void *resize(void *memory, size_t size)
{
    void *result = realloc(memory, size);
    if (result == NULL)
        fail("out of memory");
    return result;
}

static char *read_line(FILE *input)
{
    size_t size = 128, used = 0;
    char *line = resize(NULL, size);
    int ch;
    while ((ch = fgetc(input)) != EOF && ch != '\n') {
        if (used == size - 1) {
            if (size > (size_t)-1 / 2)
                fail("input line too long");
            size *= 2;
            line = resize(line, size);
        }
        line[used++] = (char)ch;
    }
    if (ferror(input))
        fail("input read failed");
    if (ch == EOF && used == 0) {
        free(line);
        return NULL;
    }
    if (used != 0 && line[used - 1] == '\r')
        used--;
    line[used] = '\0';
    return line;
}

static char *read_marked(FILE *input, int private_mode)
{
    const char *marker = private_mode ? " ==" : " =";
    size_t length = strlen(marker);
    char *line;
    while ((line = read_line(input)) != NULL) {
        if (strncmp(line, marker, length) == 0 &&
            (line[length] == '\0' || line[length] == ' ' || line[length] == '\t')) {
            size_t offset = length + (line[length] != '\0');
            memmove(line, line + offset, strlen(line + offset) + 1);
            return line;
        }
        free(line);
    }
    return NULL;
}

static void emit_line(FILE *output, const char *line)
{
    size_t length, used = 0;
    char *altered, *comment;
    if (strstr(line, "//") == NULL) {
        fprintf(output, "%s\n", line);
        return;
    }
    length = strlen(line);
    if (length > ((size_t)-1 - 1) / 2)
        fail("comment line too long");
    altered = resize(NULL, length * 2 + 1);
    while (*line != '\0') {
        if (line[0] == '*' && line[1] == '/') {
            memcpy(altered + used, "* /", 3);
            used += 3;
            line += 2;
        } else {
            altered[used++] = *line++;
        }
    }
    altered[used] = '\0';
    comment = strstr(altered, "//");
    if (comment != NULL) {
        *comment = '\0';
        fprintf(output, "%s/*%s */\n", altered, comment + 2);
    } else {
        fprintf(output, "%s\n", altered);
    }
    free(altered);
}

static void emit_source_name(FILE *output, const char *path)
{
    const char *base = path, *cursor;
    for (cursor = path; *cursor != '\0'; cursor++)
        if (*cursor == ']' || *cursor == '>' || *cursor == ':' ||
            *cursor == '/' || *cursor == '\\')
            base = cursor + 1;
    fputs("\n/* === extras/regex/", output);
    while (*base != '\0' && *base != ';')
        fputc(tolower((unsigned char)*base++), output);
    fputs(" === */\n", output);
}

static FILE *open_temporary(const char *target)
{
    const char *cursor;
    size_t directory_length = 0, offset = 0;
    int descriptor;
    if (strpbrk(target, ";*?%") != NULL)
        fail("output must have no version or wildcards");
    for (cursor = target; *cursor != '\0'; cursor++) {
        if (offset == (size_t)-1)
            fail("output path too long");
        offset++;
        if (*cursor == ']' || *cursor == '>' || *cursor == ':' ||
            *cursor == '/' || *cursor == '\\')
            directory_length = offset;
    }
    if (directory_length > (size_t)-1 - 48)
        fail("output path too long");
    temporary_name = resize(NULL, directory_length + 48);
    memcpy(temporary_name, target, directory_length);
    snprintf(temporary_name + directory_length, 48, "MKH_%08lx.TMP;1",
             (unsigned long)getpid());
    descriptor = open(temporary_name, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (descriptor < 0)
        fail("cannot exclusively create temporary output");
    temporary_owned = 1;
    temporary_output = fdopen(descriptor, "w");
    if (temporary_output == NULL) {
        close(descriptor);
        fail("cannot open temporary output stream");
    }
    return temporary_output;
}

static void publish(const char *target)
{
    size_t length = strlen(target);
    char *new_version;
    int status;
    if (ferror(temporary_output))
        fail("output write failed");
    status = fclose(temporary_output);
    temporary_output = NULL;
    if (status != 0)
        fail("output close failed");
    if (length > (size_t)-1 - 3)
        fail("output path too long");
    new_version = resize(NULL, length + 3);
    memcpy(new_version, target, length);
    memcpy(new_version + length, ";0", 3);
    if (rename(temporary_name, new_version) != 0)
        fail("cannot publish complete output as a new version");
    temporary_owned = 0;
    free(new_version);
    free(temporary_name);
}

int main(int argc, char **argv)
{
    int private_mode, output_index, i;
    const char *guard = NULL;
    FILE *output;
    if (argc >= 4 && strcmp(argv[1], "-p") == 0) {
        private_mode = 1;
        output_index = 2;
    } else if (argc >= 5 && strcmp(argv[1], "-i") == 0) {
        private_mode = 0;
        guard = argv[2];
        output_index = 3;
    } else {
        fail("usage: MKH -p output input... | MKH -i guard output input...");
        return EXIT_FAILURE;
    }
    output = open_temporary(argv[output_index]);
    if (guard != NULL)
        fprintf(output, "#ifndef %s\n#define\t%s\t/* never again */\n", guard, guard);
    fputs("/* ========= begin header generated by extras/regex/mkh ========= */\n"
          "#ifdef __cplusplus\nextern \"C\" {\n#endif\n", output);
    for (i = output_index + 1; i < argc; i++) {
        FILE *input = fopen(argv[i], "r");
        char *line;
        if (input == NULL)
            fail("cannot open input");
        emit_source_name(output, argv[i]);
        while ((line = read_marked(input, private_mode)) != NULL) {
            size_t length = strlen(line);
            if (length != 0 && line[length - 1] == '\\') {
                char *next = read_marked(input, private_mode);
                if (next != NULL) {
                    char *text = next;
                    size_t extra;
                    while (*text == ' ' || *text == '\t')
                        text++;
                    extra = strlen(text);
                    if (extra > (size_t)-1 - length)
                        fail("joined line too long");
                    line = resize(line, length + extra);
                    memcpy(line + length - 1, text, extra + 1);
                    free(next);
                }
            }
            emit_line(output, line);
            free(line);
        }
        if (fclose(input) != 0)
            fail("input close failed");
        fputc('\n', output);
    }
    fputs("#ifdef __cplusplus\n}\n#endif\n"
          "/* ========= end header generated by extras/regex/mkh ========= */\n", output);
    if (guard != NULL)
        fputs("#endif\n", output);
    publish(argv[output_index]);
    return EXIT_SUCCESS;
}
