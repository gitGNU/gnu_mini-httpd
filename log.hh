/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
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
