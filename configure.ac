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
dnl AC_CHECK_HEADER(getopt.h, have_getopt_h=yes, have_getopt_h=no, [#include <unistd.h>])
dnl AC_CHECK_FUNC(getopt_long, have_getopt=yes, have_getopt=no)
dnl if test "x$have_getopt_h" != "xyes" -o "x$have_getopt" != "xyes"; then
dnl     CPPFLAGS="$CPPFLAGS -Ilibgetopt"
dnl     AC_LIBOBJ(libgetopt/getopt)
dnl     AC_LIBOBJ(libgetopt/getopt1)
dnl fi

dnl Enable/disable debugging.
dnl
AC_MSG_CHECKING(whether to include debugging capabilities)
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

dnl Enable/disable tracing.
dnl
AC_MSG_CHECKING(whether to enable funcion tracing)
AC_ARG_WITH(trace, [  --with-trace            Enable function tracing? (default: no)],
    [
    if test "$withval" = "no"; then
       AC_MSG_RESULT(no)
    elif test "$withval" = "yes"; then
       CPPFLAGS="$CPPFLAGS -DENABLE_TRACE"
       AC_MSG_RESULT(yes)
    fi
    ],
    [
    CPPFLAGS="$CPPFLAGS -DENABLE_TRACE"
    AC_MSG_RESULT(yes)
    ])

dnl Write results.
dnl
AC_CONFIG_AUX_DIR(spirit)
AC_CONFIG_SUBDIRS(spirit)
AC_CONFIG_FILES(autoconf.mk)
AC_OUTPUT
