/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef HTTPD_HH
#define HTTPD_HH

#include <string>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "libscheduler/scheduler.hh"
#include "RegExp/RegExp.hh"
#include "log.hh"
#include "file-cache.hh"

class RequestHandler : public scheduler::event_handler
    {
  public:
    explicit RequestHandler(scheduler& sched, int fd, const sockaddr_in& sin);
    virtual ~RequestHandler();

  private:
    virtual void fd_is_readable(int);
    virtual void fd_is_writable(int);
    virtual void read_timeout(int);
    virtual void write_timeout(int);
    virtual void error_condition(int);
    virtual void pollhup(int);

    static size_t myread(int, void*, size_t);
    static size_t mywrite(int, const void*, size_t);

    void file_not_found(const char* url);
    void moved_permanently(const char* url);
    void protocol_error(const char* message);
    bool process_input(char* begin, char* end);

    enum state_t
	{
	READ_REQUEST,
	COPY_FILE,
	WRITE_CACHED_FILE,
	WRITE_REMAINING_DATA,
	TERMINATE
	};
    state_t state;

    scheduler& mysched;
    int sockfd;
    int filefd;
    char peer_addr_str[32];
    std::string host, url;
    char* buffer;
    char* buffer_end;
    char* data;
    char* data_end;
    FileCache::file_object cached_file;

    size_t bytes_sent, bytes_received;
    size_t read_calls, write_calls;
    timeval connection_start;

    static unsigned int instances;

    static const RegExp full_get_port_regex;
    static const RegExp full_get_regex;
    static const RegExp get_regex;
    static const RegExp host_regex;
    static const RegExp host_port_regex;
    };

#endif
