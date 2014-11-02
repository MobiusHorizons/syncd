#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([syncd], [0.2], [bec8@students.calvin.edu])
AC_CONFIG_SRCDIR([cache.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lc':
#AC_CHECK_LIB([c], [main])
# function in `-lcurl':
AC_CHECK_LIB([curl], [curl_global_init])
# function in `-ldl':
AC_CHECK_LIB([dl], [dlsym])
# function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])
# function in `-lrest':
AC_CHECK_LIB([rest], [rest_build_param])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_MODE_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ftruncate mkdir strchr strdup strerror])

AC_CONFIG_FILES([Makefile
                 plugins/Makefile])
AC_OUTPUT
# remove b/c no config.h?
# AC_INIT_AUTOMAKE([syncd], [0.2])

