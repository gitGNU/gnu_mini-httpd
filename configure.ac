dnl configure.ac -- Process this file with autoconf to produce a configure script.

AC_INIT([mini-httpd], m4_esyscmd([build-aux/git-version-gen .tarball-version]), [simons@cryp.to], [mini-httpd], [http://www.nongnu.org/mini-httpd/])
AC_CONFIG_SRCDIR([HTTPRequest.hh])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([1.11 -Wall gnu])
AC_CONFIG_MACRO_DIR([build-aux])
AC_MSG_NOTICE([$PACKAGE_STRING])

AC_PROG_CC
gl_EARLY
AC_PROG_CXX
AC_PROG_RANLIB
AC_LANG([C++])
AC_CHECK_HEADER(boost/spirit.hpp, :,
    AC_MSG_ERROR([Cannot find the Boost library headers! See the README for details.]))
gl_INIT
AC_SYS_LARGEFILE

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

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile libgnu/Makefile])
AC_OUTPUT
