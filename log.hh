/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __LOG_HH__
#define __LOG_HH__

#include <string>

enum log_level_t
    {
    TRACE,
    DEBUG,
    INFO,
    ERROR
    };
void log(log_level_t level, const char* fmt, ...);

class Tracer
    {
  public:
    Tracer(const char* funcname) : name(funcname)
	{
	log(TRACE, "%sEntering %s() ...", indent.c_str(), name);
	indent.append(" ");
	}
    ~Tracer()
	{
	indent.erase(indent.size()-1);
	log(TRACE, "%sLeaving %s() ...", indent.c_str(), name);
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
