/*
 * Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <syslog.h>
#include <stdarg.h>
#include "log.hh"
#include "config.hh"

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

#ifdef DEBUG
void _trace(const char* fmt, ...) throw()
    {
    if (!config->debugging)
        return;
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    va_end(ap);
    }

void _debug(const char* fmt, ...) throw()
    {
    if (!config->debugging)
        return;
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_DEBUG, fmt, ap);
    va_end(ap);
    }
#endif

void info(const char* fmt, ...) throw()
    {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_INFO, fmt, ap);
    va_end(ap);
    }

void error(const char* fmt, ...) throw()
    {
    va_list ap;
    va_start(ap, fmt);
    vsyslog(LOG_ERR, fmt, ap);
    va_end(ap);
    }
