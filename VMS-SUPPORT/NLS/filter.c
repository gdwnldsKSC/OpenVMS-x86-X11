/* Build-time implementation of Imake.rules CppSedMagic and nls R63Compat. */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_line(FILE *input, char **buffer, size_t *capacity)
{
    size_t length = 0;
    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (length + 2 > *capacity) {
            size_t next = *capacity ? *capacity * 2 : 1024;
            char *grown;
            if (next <= *capacity)
                return -1;
            grown = realloc(*buffer, next);
            if (!grown)
                return -1;
            *buffer = grown;
            *capacity = next;
        }
        if (ch == '\n')
            break;
        (*buffer)[length++] = (char)ch;
    }
    if (ferror(input))
        return -1;
    if (!length && ch == EOF)
        return 0;
    (*buffer)[length] = '\0';
    return 1;
}

static int clean_line(char *line)
{
    char *start = line;
    size_t length;
    while (*start == ' ' || *start == '\t')
        ++start;
    if (*start == '#') {
        char *number = start + 1;
        if (!strncmp(number, "line", 4))
            number += 4;
        while (*number == ' ' || *number == '\t')
            ++number;
        if (isdigit((unsigned char)*number))
            return 0;
    }
    if ((!strncmp(start, "XCOMM", 5) &&
         !(isalnum((unsigned char)start[5]) || start[5] == '_')) ||
        !strncmp(start, "XHASH", 5)) {
        *start = '#';
        memmove(start + 1, start + 5, strlen(start + 5) + 1);
    }
    length = strlen(line);
    if (length >= 2 && !strcmp(line + length - 2, "@@")) {
        line[length - 2] = '\\';
        line[length - 1] = '\0';
    }
    return 1;
}

static int compatibility_line(char *line)
{
    char *colon, *space;
    if (*line == '#')
        return 1;
    colon = strchr(line, ':');
    if (!colon)
        return 1;
    space = line + strcspn(line, " \t");
    if (colon > space)
        return 0;
    memmove(colon, colon + 1, strlen(colon + 1) + 1);
    return 1;
}

int main(int argc, char **argv)
{
    FILE *input, *output;
    char *buffer = NULL;
    size_t capacity = 0;
    int compatibility, pass, status = 0;
    if (argc < 3 || argc > 4 || (argc == 4 && strcmp(argv[3], "R63")))
        return 1;
    compatibility = argc == 4;
    input = fopen(argv[1], "r");
    if (!input)
        return 1;
    output = fopen(argv[2], "w");
    if (!output) {
        fclose(input);
        return 1;
    }
    for (pass = !compatibility; pass <= 1 && !status; ++pass) {
        int read_status;
        if (fseek(input, 0, SEEK_SET)) {
            status = 1;
            break;
        }
        while ((read_status = read_line(input, &buffer, &capacity)) > 0) {
            if (!clean_line(buffer) || (!pass && !compatibility_line(buffer)))
                continue;
            if (fputs(buffer, output) == EOF || fputc('\n', output) == EOF) {
                status = 1;
                break;
            }
        }
        if (read_status < 0)
            status = 1;
    }
    free(buffer);
    if (fclose(input))
        status = 1;
    if (fclose(output))
        status = 1;
    return status;
}
