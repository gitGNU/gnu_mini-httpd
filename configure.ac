dnl
dnl configure.ac -- Process this file with autoconf to produce a configure script.
dnl
AC_PREREQ(2.52)
AC_INIT(rh-construction.cc)

dnl Get rid of the lousy '-g -O2' defaults.
dnl
CFLAGS=$CFLAGS
CXXFLAGS=$CXXFLAGS

dnl Figure out which development environment we're using.
dnl
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

dnl What kind of std::hash_map do we have and where does the
dnl system header for it live.
dnl
AC_LANG_PUSH(C++)
AC_CHECK_HEADERS(ext/hash_map hash_map)
AC_LANG_POP(C++)

dnl If the system misses the getopt.h include or the getopt_long()
dnl function, we use the version included in the distribution.
dnl
AC_CHECK_HEADER(getopt.h, have_getopt_h=yes, have_getopt_h=no, [#include <unistd.h>])
AC_CHECK_FUNC(getopt_long, have_getopt=yes, have_getopt=no)
if test "x$have_getopt_h" != "xyes" -o "x$have_getopt" != "xyes"; then
    CPPFLAGS="$CPPFLAGS -Ilibgetopt"
    AC_LIBOBJ(libgetopt/getopt)
    AC_LIBOBJ(libgetopt/getopt1)
fi

dnl Write results.
dnl
AC_CONFIG_FILES(autoconf.mk)
AC_OUTPUT
