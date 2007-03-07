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

#ifndef MINI_HTTPD_LOGGING_HPP_INCLUDED
#define MINI_HTTPD_LOGGING_HPP_INCLUDED

/**
 *  \file  logging.hpp
 *  \brief Interface to the mini-httpd logging back-ends.
 *
 *  mini-httpd requires two types of logging back-ends: one for writing HTTP
 *  access logs that, these days, have to look like the ones Apache writes, and
 *  one for administrative messages about the daemon itself. John Torjo has
 *  proposed his <a href="http://www.torjo.com/">logging library</a> to
 *  boost.org, but unfortunately couldn't reach consensus to have it accepted.
 *  For the time being, mini-httpd relies on his code for both tasks, because
 *  the library offers a lot of ready-to-go functionality, including support
 *  for rotating log file, message timestamping, etc.
 *
 *  On systems that appear to conform to POSIX.1-2001, the administrative log
 *  stream is written to \c syslog(3). All other systems log these messages
 *  into a file. The administrative log stream is also written to the console
 *  if the daemon hasn't detached.
 *
 *  \todo Move access logging code into this module.
 */

#include <boost/log/log.hpp>
BOOST_DECLARE_LOG(httpd_logger_info)
BOOST_DECLARE_LOG_DEBUG(httpd_logger_debug)

/**
 *  \brief Initialize all configured log streams.
 *  \param my_name Process id string to use for \c openlog(3) on POSIX systems.
 */
extern void init_logging(char const * my_name);

/**
 *  \brief  Access to the INFO message logger.
 *  \return A std::ostream-like object that represents the log message.
 *  \note   When the object falls out of scope, \c std::endl is appended.
 */
#define INFO() BOOST_LOGL(httpd_logger_info,  info)

/**
 *  \brief  Access to the DEBUG message logger.
 *  \return A std::ostream-like object that represents the log message.
 *  \note   When the object falls out of scope, \c std::endl is appended.
 *
 *  Defining the pre-processor symbol \c NDEBUG disables this log stream
 *  altogether.
 */
#define TRACE() BOOST_LOGL(httpd_logger_debug, dbg) << __func__ << ": "

/*
 *  Trace a list of variables conveniently.
 */
#define TRACE_VAR(v)            #v " = " << v
#define TRACE_VARL(v)           "; " TRACE_VAR(v)
#define TRACE_VAR1(v)           TRACE() << TRACE_VAR(v)
#define TRACE_VAR2(v1,v2)       TRACE_VAR1(v1) << TRACE_VARL(v2)
#define TRACE_VAR3(v1,v2,v3)    TRACE_VAR2(v1,v2) << TRACE_VARL(v3)
#define TRACE_VAR4(v1,v2,v3,v4) TRACE_VAR3(v1,v2,v3) << TRACE_VARL(v4)

#endif // MINI_HTTPD_LOGGING_HPP_INCLUDED
