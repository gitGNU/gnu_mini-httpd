/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __LOG_HH__
#define __LOG_HH__

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
void trace(const char* fmt, ...) throw();
void debug(const char* fmt, ...) throw();
#else
#  define trace(fmt,...) if (false)
#  define debug(fmt,...) if (false)
#endif

void info(const char* fmt, ...)  throw();
void error(const char* fmt, ...) throw();

inline void log_access(bool success, const char* host, const char* url, const char* peer,
		       int sock, const timeval& runtime, size_t received, size_t sent,
		       size_t read_calls, size_t write_calls)
    {
    info("%s: host = '%s'; url = '%s'; peer = '%s'; socket = %u; runtime = %u.%u; " \
	 "received = %u; sent = %u; read calls = %u; write calls = %u",
	 ((success) ? "success" : "failure"),
	 host, url, peer, sock, runtime.tv_sec, runtime.tv_usec, sent, received);
    }

class Tracer
    {
  public:
    Tracer(const char* funcname) : name(funcname)
	{
	trace("%sEntering %s ...", indent.c_str(), name);
	indent.append("    ");
	}
    ~Tracer()
	{
	indent.erase(indent.size()-4, std::string::npos);
	trace("%sLeaving %s ...", indent.c_str(), name);
	}

  private:
    const char*        name;
    static std::string indent;
    };

#endif
