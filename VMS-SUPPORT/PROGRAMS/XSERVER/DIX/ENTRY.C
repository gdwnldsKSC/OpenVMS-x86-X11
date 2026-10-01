/* Bridge the native 32-bit environment vector to the pointer64 server. */
#include <stdio.h>
#include <stdlib.h>

#pragma pointer_size save
#pragma pointer_size 32
typedef char *VmsDixNativeString;
typedef VmsDixNativeString *VmsDixNativeEnvironment;
#pragma pointer_size restore

extern int VmsDixMain(int argc, char **argv, char **envp);
extern void CheckUserParameters(int argc, char **argv, char **envp);

static VmsDixNativeEnvironment native_environment;
static char **server_environment;
static size_t environment_count;

static void environment_error(void)
{
    fputs("Xserver: cannot prepare the native environment\n", stderr);
    exit(EXIT_FAILURE);
}

/* Upstream removes unsafe entries by compacting envp in place. */
void VmsDixCheckUserParameters(int argc, char **argv, char **envp)
{
    size_t source = 0;
    size_t destination = 0;

    if (envp != server_environment)
        environment_error();

    CheckUserParameters(argc, argv, envp);

    /* Preserve those removals in the original native vector, without
       narrowing a pointer or copying/replacing the environment strings. */
    while (envp[destination] != NULL) {
        while (source < environment_count &&
               envp[destination] != (char *)native_environment[source])
            ++source;
        if (source == environment_count)
            environment_error();
        native_environment[destination++] = native_environment[source++];
    }
    if (native_environment != NULL)
        native_environment[destination] = NULL;
    environment_count = destination;
}

int main(int argc, char **argv, char **envp)
{
    size_t i;
    int status;

    native_environment = envp;
    environment_count = 0;
    while (envp != NULL && envp[environment_count] != NULL) {
        if (environment_count == (size_t)-1 / sizeof(*server_environment) - 1)
            environment_error();
        ++environment_count;
    }
    server_environment = malloc((environment_count + 1) * sizeof(*server_environment));
    if (server_environment == NULL)
        environment_error();
    for (i = 0; i < environment_count; ++i)
        server_environment[i] = envp[i];
    server_environment[environment_count] = NULL;

    status = VmsDixMain(argc, argv, server_environment);

    free(server_environment);
    server_environment = NULL;
    native_environment = NULL;
    environment_count = 0;
    return status;
}
