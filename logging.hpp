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

#include <boost/log/log.hpp>

extern void init_logging(char const * package_name);

BOOST_DECLARE_LOG(httpd_logger_info)
BOOST_DECLARE_LOG_DEBUG(httpd_logger_debug)

#define INFO()  BOOST_LOGL(httpd_logger_info,  info)
#define TRACE() BOOST_LOGL(httpd_logger_debug, dbg)

#endif // MINI_HTTPD_LOGGING_HPP_INCLUDED
