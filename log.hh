/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __LOG_HH__
#define __LOG_HH__

#include <string>

void trace(const char* fmt, ...) throw();
void debug(const char* fmt, ...) throw();
void info(const char* fmt, ...)  throw();
void error(const char* fmt, ...) throw();

class Tracer
    {
  public:
    Tracer(const char* funcname) : name(funcname)
	{
	trace("%sEntering %s() ...", indent.c_str(), name);
	indent.append("    ");
	}
    ~Tracer()
	{
	indent.erase(indent.size()-4, std::string::npos);
	trace("%sLeaving %s() ...", indent.c_str(), name);
	}

  private:
    const char*        name;
    static std::string indent;
    };

#ifdef ENABLE_TRACER
#  define TRACE() Tracer T(__PRETTY_FUNCTION__)
#else
#  define TRACE() if (false)
#endif

#endif
