/*
 * Copyright (c) 2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#include "logging.hpp"

#ifndef BOOST_DISABLE_THREADS
#  include <boost/thread/thread.hpp>
#endif
#include <boost/log/log_impl.hpp>
#include <boost/log/functions.hpp>
#include <boost/log/extra/functions_ts.hpp>
#if defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200100)
#  include <syslog.h>
#  ifndef LOG_PERROR
#    define LOG_PERROR 0
#  endif
#else
#  include <iostream>
#endif

BOOST_DEFINE_LOG(httpd_logger_info,  "info")
BOOST_DEFINE_LOG(httpd_logger_debug, "debug")

static void line_writer(std::string const & /* prefix */, std::string const & msg)
{
#if defined(_POSIX_VERSION) && (_POSIX_VERSION >= 200100)
  syslog(LOG_INFO, "%s", msg.c_str());
#else
  std::cout << msg << std::endl;
#endif
}

#ifdef BOOST_HAS_PTHREADS
static void add_thread_id(std::string const & prefix, std::string & msg)
{
  using namespace std;
  char tid[64];
  int len( snprintf(tid, sizeof(tid), "[tid %lx] ", pthread_self()) );
  BOOST_ASSERT(static_cast<size_t>(len) < sizeof(tid) );
  msg.insert(msg.begin(), tid, tid + len);
}
#endif

void init_logging(char const * package_name)
{
  using namespace boost::logging;

  add_appender("*", &line_writer);

#if defined(_POSIX_VERSION) || (_POSIX_VERSION >= 200100)
  openlog(package_name, LOG_CONS | LOG_PID | LOG_PERROR, LOG_DAEMON);
#else
  add_modifier("*", prepend_time("$yyyy-$MM-$ddT$hh:$mm:$ss "));
#endif

#ifdef BOOST_HAS_PTHREADS
  add_modifier("debug", &add_thread_id);
#endif

  flush_log_cache();
}
