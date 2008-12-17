#
# Makefile.am for mini-httpd
#

ACLOCAL_AMFLAGS		= -I m4

sbin_PROGRAMS		= micro-httpd mini-httpd

noinst_HEADERS		= http-daemon.hpp		\
			  http.hpp			\
			  input-buffer.hpp		\
			  io.hpp			\
			  logging.hpp			\
			  output-buffer.hpp		\
			  rfc2616.hpp			\
			  system-error.hpp

HTTPD_SOURCES		= main.cpp			\
			  io-driver.cpp			\
			  logging.cpp			\
			  http.cpp			\
			  http-daemon.cpp		\
			  boost/log/functions.cpp	\
			  boost/log/log.cpp		\
			  boost/log/log_manager.cpp

HTTPD_CPPFLAGS		= -DBOOST_TEST_NO_AUTO_LINK=1	\
			  -DBOOST_SYSTEM_STATIC_LINK=1	\
			  -DDATE_TIME_INLINE		\
			  -UPACKAGE_NAME

micro_httpd_SOURCES	= $(HTTPD_SOURCES)
micro_httpd_CPPFLAGS	= $(HTTPD_CPPFLAGS) @BOOST_CPPFLAGS@		\
			  -DBOOST_DISABLE_THREADS			\
			  -DBOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
micro_httpd_LDFLAGS	= @BOOST_LDFLAGS@
micro_httpd_LDADD	= @BOOST_PROGRAM_OPTIONS_LIB@ @BOOST_SYSTEM_LIB@

mini_httpd_SOURCES	= $(HTTPD_SOURCES)
mini_httpd_CPPFLAGS	= $(HTTPD_CPPFLAGS) @BOOST_MT_CPPFLAGS@ -DBOOST_SPIRIT_THREADSAFE
mini_httpd_LDLAGS	= @BOOST_MT_LDFLAGS@
mini_httpd_LDADD	= @BOOST_THREAD_LIB@ @BOOST_PROGRAM_OPTIONS_LIB@ @BOOST_SYSTEM_LIB@

CLEANFILES		= $(DIST_ARCHIVES)
MAINTAINERCLEANFILES	= missing install-sh configure Makefile.in	\
			  aclocal.m4 depcomp config.guess config.sub

EXTRA_DISTFILES		= README

noinst_HEADERS += \
  boost/log/detail/defs.hpp \
  boost/log/detail/log_fwd.hpp \
  boost/log/detail/log_manager.hpp \
  boost/log/detail/manipulate_logs.hpp \
  boost/log/detail/ts.hpp \
  boost/log/detail/ts_boost.hpp \
  boost/log/detail/ts_none.hpp \
  boost/log/detail/ts_posix.hpp \
  boost/log/detail/ts_win32.hpp \
  boost/log/extra/appenders.hpp \
  boost/log/extra/file_appender.hpp \
  boost/log/extra/filename_factory.hpp \
  boost/log/extra/functions_ts.hpp \
  boost/log/extra/periodic_file_appender.hpp \
  boost/log/extra/rolling_file_appender.hpp \
  boost/log/extra/shared_memory.hpp \
  boost/log/functions.cpp \
  boost/log/functions.hpp \
  boost/log/log.cpp \
  boost/log/log.hpp \
  boost/log/log_impl.hpp \
  boost/log/log_manager.cpp
