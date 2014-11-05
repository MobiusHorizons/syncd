#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([syncd], [0.2], [bec8@students.calvin.edu])
# remove b/c no config.h?
# AM_INIT_AUTOMAKE([1.14 foreign no-define])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

#libtool init
LT_INIT([dlopen])
# Checks for libraries
# FIXME: Replace `main' with a function in `-lc':
#AC_CHECK_LIB([c], [main])
# function in `-lcurl':
AC_CHECK_LIB([curl], [curl_global_init])
# function in `-ldl': #FIXME this probs isn't supposed to be a hard req
AC_CHECK_LIB([ltdl], [lt_dlsym])
# function in `-lpthread':
AC_CHECK_LIB([pthread], [pthread_create])
# function in `-lrest':
AC_CHECK_LIB([rest], [rest_build_param])
# json-c:
AC_CHECK_LIB([json-c], [json_object_new_object])

# libtdl
LT_CONFIG_LTDL_DIR([libltdl])
AC_CONFIG_MACRO_DIR([m4])
LTDL_INIT([recursive])
AC_CONFIG_FILES([libltdl/Makefile])

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

AC_OUTPUT(Makefile libdropbox/Makefile librest/Makefile libexec/Makefile src/Makefile )

