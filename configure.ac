dnl
dnl configure.ac -- Process this file with autoconf to produce a configure script.
dnl

dnl Get our current version

AC_INIT(mini-httpd, 1.0, simons@cryp.to)
AM_INIT_AUTOMAKE([foreign 1.7])
AC_CONFIG_SRCDIR([rh-construction.cc])

dnl This software is best installed in a hierarchy of its own.
dnl
AC_PREFIX_DEFAULT(/usr/local/mini-httpd)

dnl Get rid of the lousy '-g -O2' defaults.
dnl
CFLAGS=$CFLAGS
CXXFLAGS=$CXXFLAGS

dnl Figure out which development environment we're using.
dnl
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_LANG(C++)

dnl We require Boost.Spirit.
AC_CHECK_HEADER(boost/spirit.hpp, :,
    AC_MSG_ERROR([Cannot find the Boost library headers! See the README for details.]))

dnl If the system misses the getopt.h include or the getopt_long()
dnl function, we use the version included in the distribution.
dnl
AC_CHECKING([whether getopt library supports long-style parameters])
AC_CHECK_HEADER(getopt.h, have_getopt_h=yes, have_getopt_h=no, [#include <unistd.h>])
AC_CHECK_FUNC(getopt_long, have_getopt=yes, have_getopt=no)
if test "x$have_getopt_h" != "xyes" -o "x$have_getopt" != "xyes"; then
    AC_MSG_RESULT([Using our own version instead.])
    CPPFLAGS="$CPPFLAGS -Ilibgetopt"
    AC_LIBOBJ(libgetopt/getopt)
    AC_LIBOBJ(libgetopt/getopt1)
fi

dnl Include debug code?
dnl
AC_MSG_CHECKING([whether to include debugging capabilities])
AC_ARG_WITH(debug, [  --with-debug            Support debugging? (default: yes)],
    [
    if test "$withval" = "no"; then
       AC_MSG_RESULT(no)
    elif test "$withval" = "yes"; then
       CPPFLAGS="$CPPFLAGS -DDEBUG"
       AC_MSG_RESULT(yes)
    fi
    ],
    [
    CPPFLAGS="$CPPFLAGS -DDEBUG"
    AC_MSG_RESULT(yes)
    ])

AC_CHECK_PROG(docbook2man, docbook2man, docbook2man, touch)

dnl Write results.
dnl
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
