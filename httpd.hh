/*
 * Copyright (c) 2001 by Peter Simons <simons@ieee.org>.
 * All rights reserved.
 */

#ifndef __HTTPD_HH__
#define __HTTPD_HH__

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "libscheduler/scheduler.hh"
#include "RegExp/RegExp.hh"
#include "log.hh"

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

    void read_request();
    void read_file();
    void file_not_found(const string& file);

    enum state_t
	{
	READ_REQUEST,
	WRITE_ANSWER,
	TERMINATE
	};
    state_t state;

    scheduler& mysched;
    int sockfd;
    int filefd;
    char peer_addr_str[32];
    scheduler::handler_properties prop;
    string buffer;
    string host, uri;

    static const RegExp full_get_port_regex;
    static const RegExp full_get_regex;
    static const RegExp get_regex;
    static const RegExp host_regex;
    static const RegExp host_port_regex;

    static char* tmp;
    static unsigned instances;
    };

#endif
