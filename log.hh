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

#ifndef LOG_HH
#define LOG_HH

#include <string>
#include <sys/time.h>
#include <unistd.h>

#ifdef ENABLE_TRACER
#  define TRACE() Tracer T(__PRETTY_FUNCTION__)
#  ifndef DEBUG
#    define DEBUG
#  endif
#else
#  define TRACE() if (false)
#endif

#ifdef DEBUG
void _trace(const char* fmt, ...) throw();
void _debug(const char* fmt, ...) throw();
#  define trace(x) _trace x
#  define debug(x) _debug x
#else
#  define trace(x) if (false)
#  define debug(x) if (false)
#endif

void info(const char* fmt, ...)  throw();
void error(const char* fmt, ...) throw();

class Tracer
    {
  public:
    Tracer(const char* funcname) : name(funcname)
	{
	trace(("%sEntering %s ...", indent.c_str(), name));
	indent.append("    ");
	}
    ~Tracer()
	{
	indent.erase(indent.size()-4, std::string::npos);
	trace(("%sLeaving %s ...", indent.c_str(), name));
	}

  private:
    const char*        name;
    static std::string indent;
    };

#endif
