/* Image-local permission semantics required by upstream xauth's umask. */
#include <stdio.h>
#include <stdlib.h>
#include <unixlib.h>

extern int VMS_XAUTH_MAIN(int argc, char **argv);

int main(int argc, char **argv)
{
    int feature = decc$feature_get_index("DECC$FILE_PERMISSION_UNIX");

    if (feature < 0 || decc$feature_set_value(feature, 1, 1) < 0 ||
        decc$feature_get_value(feature, 1) != 1) {
        fputs("xauth: cannot enable private authority-file permissions\n",
              stderr);
        return EXIT_FAILURE;
    }

    return VMS_XAUTH_MAIN(argc, argv);
}
