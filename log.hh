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

inline void log_access(bool success, const char* host, const char* url, const char* peer,
                       const char* referer, const char* user_agent,
		       int sock, const timeval& runtime, size_t received, size_t sent,
		       size_t read_calls, size_t write_calls) throw()
    {
    info("%s: host = '%s'; url = '%s'; peer = '%s'; referer = '%s'; user-agent = '%s' " \
         "socket = %d; runtime = %u.%u; " \
	 "received = %u; sent = %u; read calls = %u; write calls = %u",
	 ((success) ? "success" : "failure"),
	 host, url, peer, referer, user_agent, sock, runtime.tv_sec, runtime.tv_usec,
         received, sent, read_calls, write_calls);
    }

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
