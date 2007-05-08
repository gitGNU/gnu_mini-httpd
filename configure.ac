dnl
dnl configure.ac for mini-httpd
dnl
AC_PREREQ(2.59)
AC_INIT([mini-httpd], [2007-05-07], [simons@cryp.to])
AM_INIT_AUTOMAKE([foreign 1.10 std-options])
AC_CONFIG_SRCDIR([http-daemon.hpp])
dnl
dnl This software is best installed in a hierarchy of its own.
dnl
AC_PREFIX_DEFAULT(/usr/local/mini-httpd)
dnl
dnl Get rid of the lousy '-g -O2' defaults.
dnl
CFLAGS=$CFLAGS
CXXFLAGS=$CXXFLAGS
dnl
dnl Figure out which development environment we're using.
dnl
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_LANG(C++)
dnl
dnl Include debug code?
dnl
AC_MSG_CHECKING([whether to include debugging capabilities])
AC_ARG_ENABLE([debugging], [  --enable-debugging      Support debugging? (default: yes)],
[
  if test "x${enableval}" = "xno"; then
    CPPFLAGS="$CPPFLAGS -DNDEBUG"
    AC_MSG_RESULT(no)
  else
    CPPFLAGS="$CPPFLAGS -DDEBUG"
    AC_MSG_RESULT(yes)
  fi
],
[
  CPPFLAGS="$CPPFLAGS -DDEBUG"
  AC_MSG_RESULT(yes)
])
dnl
dnl Write results.
dnl
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
