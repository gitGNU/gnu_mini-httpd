/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#include <syslog.h>
#include <stdarg.h>
#include "log.hh"

std::string Tracer::indent;

namespace
    {
    struct init_logging
	{
	init_logging()
	    {
	    openlog("httpd", LOG_CONS | LOG_PERROR | LOG_PID, LOG_DAEMON);
	    }
	~init_logging()
	    {
	    closelog();
	    }
	};
    static init_logging sentry;
    }

void trace(const char* fmt, ...)
    {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    va_end(ap);
    }

void debug(const char* fmt, ...)
    {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    va_end(ap);
    }

void info(const char* fmt, ...)
    {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_INFO, fmt, ap);
    va_end(ap);
    }

void error(const char* fmt, ...)
    {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_ERR, fmt, ap);
    va_end(ap);
    }
