#
# Build mini-httpd
#

#BOOST_STAGE		= /usr/local/src/boost-current/stage/lib/libboost_
#BOOST_COMPILER_ID	= gcc34
#BOOST_VERSION_ID	= -1_35

BOOST_COMPILER_ID	= gcc
BOOST_STAGE		= /usr/lib/libboost_
BOOST_VERSION_ID	=

BOOST_LIB_ID		= -$(BOOST_COMPILER_ID)$(BOOST_VERSION_ID).a
BOOST_MTLIB_ID		= -$(BOOST_COMPILER_ID)-mt$(BOOST_VERSION_ID).a

noinst_HEADERS		= http-daemon.hpp		\
			  http.hpp			\
			  io-driver.hpp			\
			  io-input-buffer.hpp		\
			  io-output-buffer.hpp		\
			  io.hpp			\
			  logging.hpp			\
			  rfc2616.hpp

HTTPD_SOURCES		= main.cpp			\
			  logging.cpp			\
			  http.cpp			\
			  http-daemon.cpp		\
			  ../boost/log/functions.cpp	\
			  ../boost/log/log.cpp		\
			  ../boost/log/log_manager.cpp	\
			  ../boost/system/error_code.cpp

HTTPD_CPPFLAGS		= -DBOOST_TEST_NO_AUTO_LINK=1	\
			  -DBOOST_SYSTEM_STATIC_LINK=1	\
			  -DDATE_TIME_INLINE		\
			  -UPACKAGE_NAME		\
			  -UPACKAGE_VERSION

sbin_PROGRAMS		= mini-httpd micro-httpd

mini_httpd_SOURCES	= $(HTTPD_SOURCES)
mini_httpd_CXXFLAGS	= -pthread
mini_httpd_CPPFLAGS	= $(HTTPD_CPPFLAGS)				 \
			  -DBOOST_SPIRIT_THREADSAFE
mini_httpd_LDADD	= $(BOOST_STAGE)thread$(BOOST_MTLIB_ID)		 \
			  $(BOOST_STAGE)program_options$(BOOST_MTLIB_ID) \
			  $(BOOST_STAGE)prg_exec_monitor$(BOOST_MTLIB_ID)

micro_httpd_SOURCES	= $(HTTPD_SOURCES)
micro_httpd_CPPFLAGS	= $(HTTPD_CPPFLAGS)				\
			  -DBOOST_THREAD_BUILD_LIB=1			\
			  -DBOOST_DISABLE_THREADS			\
			  -DBOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
micro_httpd_LDADD	= $(BOOST_STAGE)program_options$(BOOST_LIB_ID)	\
			  $(BOOST_STAGE)prg_exec_monitor$(BOOST_LIB_ID)
