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
    virtual void error_condition(int);
    virtual void pollhup(int);

    void file_not_found(const string& file);
    void protocol_error(const string& message);

    bool process_input(const char* begin, const char* end);

    void register_network_read_handler();
    void remove_network_read_handler();
    void register_network_write_handler();
    void remove_network_write_handler();
    void register_file_read_handler();
    void remove_file_read_handler();

    scheduler::handler_properties network_properties;
    scheduler::handler_properties file_properties;

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
