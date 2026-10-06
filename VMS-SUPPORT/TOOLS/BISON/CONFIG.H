#ifndef BISON_NATIVE_CONFIG_H
#define BISON_NATIVE_CONFIG_H
#define HAVE_CONFIG_H 1
#define PACKAGE "bison"
#define VERSION "1.35"
#define PACKAGE_NAME "GNU Bison"
#define PACKAGE_VERSION "1.35"
#define PACKAGE_STRING "GNU Bison 1.35"
#define PACKAGE_TARNAME "bison"
#define PACKAGE_BUGREPORT "bug-bison@gnu.org"
#define LOCALEDIR "."
#define STDC_HEADERS 1
#define PROTOTYPES 1
#define HAVE_STDDEF_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_STRINGS_H 1
#define HAVE_CTYPE_H 1
#define HAVE_LIMITS_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_LOCALE_H 1
#define HAVE_SETLOCALE 1
#define HAVE_WCHAR_H 1
#define HAVE_WCTYPE_H 1
#define HAVE_MBRTOWC 0
#define HAVE_MBSINIT 0
#define mbstate_t int
#define HAVE_ISWPRINT 1
#define HAVE_VPRINTF 1
#define HAVE_C_BACKSLASH_A 1
#define HAVE_STRERROR 1
#define HAVE_DECL_STRERROR 1
#define HAVE_DECL_STRERROR_R 1
#define HAVE_DECL_STRCHR 1
#define HAVE_DECL_STRSPN 1
#define HAVE_DECL_MEMCHR 1
#define HAVE_DECL_MEMRCHR 0
#define HAVE_DECL_STPCPY 1
#define HAVE_DECL_STRNLEN 1
#define HAVE_DONE_WORKING_MALLOC_CHECK 1
#define HAVE_DONE_WORKING_REALLOC_CHECK 1
#define HAVE_ALLOCA 1
#define HAVE_ALLOCA_H 1
#define __PTRDIFF_TYPE__ __int64
#define __PTR_TO_INT(pointer) ((__int64) (pointer))
#define __INT_TO_PTR(integer) ((char *) (__int64) (integer))
#include <stdio.h>
#undef optarg
#undef optind
#undef opterr
#undef optopt
#undef getopt
#define optarg bison_optarg
#define optind bison_optind
#define opterr bison_opterr
#define optopt bison_optopt
#define getopt bison_getopt
#include "obstack.h"
#undef obstack_free
typedef char bison_native_pointer_is_64[(sizeof(void *) == 8) ? 1 : -1];
typedef char bison_native_long_is_32[(sizeof(long) == 4) ? 1 : -1];
typedef char bison_native_pointer_integer_is_64[(sizeof(__int64) == 8) ? 1 : -1];
#endif
