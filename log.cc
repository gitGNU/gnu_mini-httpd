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

void log(log_level_t level, const char* fmt, ...)
    {
    va_list ap;
    int priority;
    va_start(ap, fmt);

    switch(level)
	{
	case TRACE:
	case DEBUG:
	    priority = LOG_DEBUG;
	    break;
	case INFO:
	    priority = LOG_INFO;
	    break;
	case ERROR:
	    priority = LOG_ERR;
	    break;
	};
    vsyslog(priority, fmt, ap);

    va_end(ap);
    }
