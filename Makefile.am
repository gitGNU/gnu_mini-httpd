#
# Build mini-httpd
#

noinst_HEADERS		 = http-daemon.hpp				\
			   http.hpp					\
			   io-driver.hpp				\
			   io-input-buffer.hpp				\
			   io-output-buffer.hpp				\
			   io.hpp					\
			   logging.hpp

BOOST_LOG_SOURCES	= ../boost/log/functions.cpp			\
			  ../boost/log/log.cpp				\
			  ../boost/log/log_manager.cpp

HTTPD_SOURCES		= main.cpp					\
			  logging.cpp					\
			  http.cpp					\
			  http-daemon.cpp				\
			  $(BOOST_LOG_SOURCES)

HTTPD_CPPFLAGS		= -DBOOST_TEST_NO_AUTO_LINK=1			\
			  -DBOOST_THREAD_BUILD_LIB=1
			  -DDATE_TIME_INLINE				\
			  -DSANITY_BUILDING_DLL				\
			  -UPACKAGE_NAME				\
			  -UPACKAGE_VERSION

sbin_PROGRAMS		= mini-httpd micro-httpd

mini_httpd_SOURCES	= $(HTTPD_SOURCES)
mini_httpd_CXXFLAGS	= -pthread
mini_httpd_CPPFLAGS	= $(HTTPD_CPPFLAGS)				\
			  -DBOOST_SPIRIT_THREADSAFE
mini_httpd_LDADD	= -lboost_thread-gcc34-mt-1_35			\
			  -lboost_system-gcc34-mt-1_35			\
			  -lboost_program_options-gcc34-mt-1_35		\
			  -lboost_prg_exec_monitor-gcc34-mt-1_35

micro_httpd_SOURCES	= $(HTTPD_SOURCES)
micro_httpd_CPPFLAGS	= $(HTTPD_CPPFLAGS)				\
			  -DBOOST_DISABLE_THREADS			\
			  -DBOOST_SPIRIT_SINGLE_GRAMMAR_INSTANCE
micro_httpd_LDADD	= -lboost_system-gcc34-1_35			\
			  -lboost_program_options-gcc34-1_35		\
			  -lboost_prg_exec_monitor-gcc34-1_35
