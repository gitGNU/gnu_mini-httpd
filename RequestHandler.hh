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

    enum state_t
	{
	READ_REQUEST_LINE,
	READ_REQUEST_HEADER,
	READ_REQUEST_BODY,
        SETUP_REPLY,
	COPY_FILE,
	WRITE_REMAINING_DATA,
	TERMINATE
	};
    state_t state;

    bool get_request_line();
    bool get_request_header();
    bool get_request_body();
    bool setup_reply();
    bool copy_file();
    bool write_remaining_data();
    bool terminate();

    typedef bool (RequestHandler::*state_fun_t)();
    static const state_fun_t state_handlers[];

    void protocol_error(const char*);
    void file_not_found(const char*);
    void moved_permanently(const char*);

    scheduler& mysched;
    int sockfd;
    int filefd;

    std::string read_buffer, write_buffer;

    char peer_addr_str[32];
    std::string host, path;

    size_t bytes_sent, bytes_received;
    timeval connection_start;

    static unsigned int instances;
    };

#endif
