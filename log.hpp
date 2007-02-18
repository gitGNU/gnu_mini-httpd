/*
 * Copyright (c) 2001-2007 Peter Simons <simons@cryp.to>
 *
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held liable
 * for any damages arising from the use of this software.
 *
 * Copying and distribution of this file, with or without
 * modification, are permitted in any medium without royalty
 * provided the copyright notice and this notice are preserved.
 */

#ifndef IOXX_HTTPD_LOG_HPP_INCLUDED
#define IOXX_HTTPD_LOG_HPP_INCLUDED

#include <boost/log/log.hpp>

namespace logging
{
  BOOST_DECLARE_LOG_DEBUG(httpd_debug)
  BOOST_DECLARE_LOG(httpd_misc)
  BOOST_DECLARE_LOG(httpd_access)
}

// Abstract access to our logger.
#define TRACE()         BOOST_LOGL(::logging::httpd_debug, dbg)
#define INFO()          BOOST_LOGL(::logging::httpd_misc,  info)
#define ERROR()         BOOST_LOGL(::logging::httpd_misc,  err)

// Continue with literal string or operator<<().
#define SOCKET_TRACE(s) TRACE() << "socket " << s << ": "
#define SOCKET_INFO(s)  INFO()  << "socket " << s << ": "
#define SOCKET_ERROR(s) ERROR() << "socket " << s << ": "

#endif // IOXX_HTTPD_LOG_HPP_INCLUDED
