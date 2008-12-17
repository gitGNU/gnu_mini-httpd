dnl configure.ac for mini-httpd

AC_PREREQ([2.59])
AC_INIT([mini-httpd], [2.0-beta-1], [simons@cryp.to])
AM_INIT_AUTOMAKE([foreign 1.10])
AC_CONFIG_SRCDIR([http-daemon.hpp])
AC_PREFIX_DEFAULT([/usr/local/mini-httpd])
AC_CONFIG_MACRO_DIR(m4)

CFLAGS="${CFLAGS}"              # No lousy '-g -O2' defaults.
CXXFLAGS="${CXXFLAGS}"

AC_PROG_CXX
AC_LANG([C++])
AC_SYS_LARGEFILE
AX_BOOST_BASE([1.36])
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_SYSTEM
tmp_boost_cppflags="${BOOST_CPPFLAGS}"
tmp_boost_ldflags="${BOOST_LDFLAGS}"
AX_BOOST_THREAD
BOOST_MT_CPPFLAGS="${BOOST_CPPFLAGS}"
BOOST_MT_LDFLAGS="${BOOST_LDFLAGS}"
BOOST_CPPFLAGS="${tmp_boost_cppflags}"
BOOST_LDFLAGS="${tmp_boost_ldflags}"
ACX_PTHREAD
BOOST_MT_CPPFLAGS="${PTHREAD_CFLAGS} ${BOOST_MT_CPPFLAGS}"
BOOST_MT_LDFLAGS="${PTHREAD_LIBS} ${BOOST_MT_LDFLAGS}"
AC_SUBST([BOOST_MT_CPPFLAGS])
AC_SUBST([BOOST_MT_LDFLAGS])

AC_MSG_CHECKING([whether to include debugging capabilities])
AC_ARG_ENABLE([debugging], [  --enable-debugging      Support debugging? (default: yes)],
[
  if test "x${enableval}" = "xno"; then
    CPPFLAGS="$CPPFLAGS -DNDEBUG"
    AC_MSG_RESULT([no])
  else
    CPPFLAGS="$CPPFLAGS -DDEBUG"
    AC_MSG_RESULT([yes])
  fi
],
[
  CPPFLAGS="$CPPFLAGS -DDEBUG"
  AC_MSG_RESULT([yes])
])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
